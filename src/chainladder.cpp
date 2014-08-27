// [[Rcpp::depends(RcppArmadillo)]]
// [[Rcpp::plugins(cpp11)]]
#include <RcppArmadillo.h>
#include <iostream>
#include <vector>
#include <iterator>
#include <algorithm>

using namespace std;
using namespace Rcpp;

/* Calculating betas for calculating tail factors */
// [[Rcpp::export]]
SEXP calc_betas(SEXP y)
{
  int i, j, k;
  //double a, b;
  NumericVector _y(y);
  LogicalVector l = !is_na(_y);
  j = sum(l);
  NumericMatrix x(j, 2);
  NumericVector __y(j);
  // Creating matrices for the calculation
  k = 0;
  for(i = 0; i < _y.size(); ++i)
  {
    if(l[i]){
      x(k, 0) = 1.;
      x(k, 1) = i + 1;
      __y[k] = _y[i];
      ++k;
    }
  }
  // Calculation of beta
  arma::mat X(x.begin(), j, 2, FALSE);
  arma::vec Y(__y.begin(), j, FALSE);
  arma::vec output = arma::inv(X.st()*X)*X.st()*Y;
  return wrap(output);
}

// Overload of the above function
arma::vec calc_betas(NumericVector y)
{
    int i, j, k;
  LogicalVector l = !is_na(y);
  j = sum(l);
  NumericMatrix x(j, 2);
  NumericVector _y(j);
  // Creating matrices for the calculation
  k = 0;
  for(i = 0; i < y.size(); ++i)
  {
    if(l[i]){
      x(k, 0) = 1.;
      x(k, 1) = i + 1;
      _y[k] = y[i];
      ++k;
    }
  }
  // Calculation of beta
  arma::mat X(x.begin(), j, 2, FALSE);
  arma::vec Y(_y.begin(), j, FALSE);
  arma::vec output = arma::inv(X.st()*X)*X.st()*Y;
  return output;
}


// [[Rcpp::export(chainladder_)]]
SEXP chainladder(SEXP tri, SEXP wghts, SEXP alpha, SEXP tail, SEXP tail_se, SEXP tail_sigma)
{
  int i, p, k;
  NumericMatrix C(tri);
  p = C.ncol();
  NumericMatrix w(wghts);
  NumericVector alph(alpha);
  NumericVector EwCik(p - 1);
  NumericVector EwCikFik(p - 1);
  NumericVector EwCikFikfik(p - 1);
  NumericMatrix F(p - 1, p - 1);
  NumericVector f(p);
  NumericVector f_se(p - 1);
  NumericVector sigma(p - 1);
  NumericVector R(p);
  NumericMatrix F_se(p, p - 1);
  NumericMatrix C_se(p, p);
  NumericVector _tail(tail);
  NumericVector _tail_se(tail_se);
  NumericVector _tail_sigma(tail_sigma);
  _tail_se[0] = pow(_tail_se[0], 2);
  _tail_sigma[0] = pow(_tail_sigma[0], 2);
  double t_se, t_sgm;
  t_se = _tail_se[0];
  t_sgm = _tail_sigma[0];
  double Caij;
  for(k = 0; k < p - 1; ++k)
  {
    for(i = 0; i < p - k - 1; ++i)
    {
      F(i, k) = C(i, k + 1)/C(i, k);
      Caij = pow(C(i, k), alph[0]);
      EwCikFik[k] += w(i, k)*Caij*F(i, k);
      EwCik[k] += w(i, k)*Caij;
    }
    f[k] = EwCikFik[k]/EwCik[k];
    for(i = 0; i < p - k - 1; ++i)
    {
      Caij = pow(C(i, k), alph[0]);
      EwCikFikfik[k] += w(i, k)*Caij*pow(F(i, k) - f[k], 2);
    }
    sigma[k] = EwCikFikfik[k]/(p - k - 2);
    f_se[k] = sigma[k]/EwCik[k];
  }
  
  sigma[p - 2] = min(pow(sigma[p - 3], 2)/sigma[p - 4], min(sigma[p - 4], sigma[p - 3]));
  f_se[p - 2] = sigma[p - 2]/EwCik[p - 2];
  // Calculating the Claims and Reserve
  for(i = p - 1; i > 0; --i)
  {
    for(k = p - i; k < p; ++k)
    {
      C(i, k) = C(i, k - 1)*f[k - 1];
    }
  }
  
  // Calculating F.se
  for(i = 0; i < p; ++i)
  {
    for(k = 0; k < p - 1; ++k)
    {
      F_se(i, k) = sigma[k] / (w(i, k) * pow(C(i, k), alph[0]));
    }
  }
  
  // Calculating C_se
  for(i = p - 1; i > 0; --i)
  {
    for(k = p - i; k < p; ++k)
    {
      C_se(i, k) = pow(C(i, k - 1), 2) * (F_se(i, k - 1) + f_se(k - 1)) + C_se(i, k - 1)*pow(f[k - 1], 2);
    }
  }
  
  f[p - 1] = 1.;
  
  // tail factor
  arma::vec betas;
  NumericVector _f_se(p);
  NumericVector _sigma(p);
  NumericMatrix _C(p, p + 1);
  double tail_x;
  if(_tail[0] != 0)
  {
    // tail factor for f
    NumericVector _f(f.begin(), f.end() - 1);
    _f = Rcpp::log(_f - 1);
    betas = calc_betas(_f);
    arma::vec ttail = arma::linspace(p, p + 99, 100);
    double tail_f = arma::prod(arma::exp(betas[0] + ttail*betas[1]) + 1);
    // use tail factor for f only if a tail factor has been specified
    if(_tail[0] == 1)
    {
      f[p - 1] = tail_f < 2 ? tail_f : 1.;
    }else{
      f[p - 1] = _tail[0];
    }
    tail_x = (log(f[p - 1] - 1) - betas[0]) / betas[1];
    
    // If there is no tail factor for f.se use extrapolation
    if(t_se == 0)
    {
      betas = calc_betas(Rcpp::log(f_se));
      t_se = exp(betas[1]*tail_x + betas[0]);
    }
    for(i = 0; i < p - 1; ++i)
    {
      _f_se[i] = f_se[i];
    }
    _f_se[p - 1] = t_se;
    f_se = _f_se;
    
    if(t_sgm == 0)
    {
      betas = calc_betas(Rcpp::log(sigma));
      t_sgm = exp(betas[1]*tail_x + betas[0]);
    }
    for(i = 0; i < p - 1; ++i)
    {
      _sigma[i] = sigma[i];
    }
    _sigma[p - 1] = t_sgm;
    sigma = _sigma;
    
    // ULT for C
    for(i = 0; i < p; ++i)
      _C(_, i) = C(_, i);
    
    _C(_, p) = C(_, p - 1)*f[p - 1];
    C = _C;
    
    
    NumericMatrix _C(p, p);
    // ULT for F_se
    for(i = 0; i < p - 1; ++i)
      _C(_, i) = F_se(_, i);
    
    for(i = 0; i < p; ++i)
    {
      _C(i, p - 1) = sigma[p - 1] / (w(i, p - 1) * pow(C(i, p - 1), alph[0]));
    }
    F_se = _C;
    
    
    // ULT for C_se
    NumericMatrix _C_se(p, p + 1);
    for(i = 0; i < p; ++i)
      _C_se(_, i) = C_se(_, i);
    
    for(i = 0; i < p; ++i)
    {
     _C_se(i, p) = pow(C(i, p - 1), 2) * (F_se(i, p - 1) + f_se(p - 1)) + C_se(i, p - 1)*pow(f[p - 1], 2);
    }
    
    C_se = _C_se;
  }
  
  // Calculate the standard error for the triangle
  double Mack_SE;
  double term2;
  double term3;
  int ii, n = F_se.ncol();
  for(k = 0; k < n; ++k)
  {
    ii = n - 1 - k;
    for(i = ii; i < n; ++i)
    {
      term2 = term2 + pow(C(i, k), 2)*F_se(i, k);
      term3 = term3 + C(i, k);
    }
    Mack_SE = Mack_SE*pow(f[k], 2) + term2 + pow(term3, 2)*f_se[k];
    term2 = term3 = 0;
  }
  
  n = C.ncol();
  
  // The Reserve
  for(i = 0; i < n; ++i)
  {
    R[i] = C(i, n - 1) - C(i, p - i - 1);
  }
  
  List output = List::create(_["f"] = f, _["f.se"] = f_se, _["sigma"] = sigma,
      _["Mack.S.E"] = Mack_SE, _["C"] = C, _["C.se"] = C_se, _["R"] = R,
      _["F"] = F, _["F.se"] = F_se);
  return wrap(output);
}

