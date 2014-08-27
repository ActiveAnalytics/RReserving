#' @name chainladder
#' @title Function to Carry out Chain Ladder Analysis
#'
#' @description The chainladder function executes the Chain Ladder algorithm that calculates claims reserves,
#' the completed claims triangles and all associated standard errors as described by Thomas Mack's 1999 paper. 
#' Its use is modelled after the MackChainLadder function in the ChainLadder package.
#' 
#' @usage chainladder(tri, wghts = 1, alpha = 1, tail = 0, tail.se = 0, tail.sigma = 0)
#' 
#' @param tri (matrix) cumulative claims triangle with development periods across the columns and
#'  accounting periods down the rows. See \code{?mortgage} for an example of the expected matrix format.
#' @param wghts (matrix) same size as \code{tri} holding the weights matrix 
#'  \eqn{w_{ik} \epsilon [0, 1]} as in the paper. Defaults to a matrix of 1s.
#' @param alpha (discrete numeric) \eqn{\alpha \epsilon {0, 1, 2}} as in the paper defaults to 1 to 
#'  give the standard chain ladder algorithm.
#' @param tail (numeric) denotes the tail factor of \eqn{f}. Defaults to \code{tail = 0} where no tail 
#'    factor is calculated. \code{tail = 1} to calculate a tail factor by log-linear extrapolation of 
#'    log(f - 1) otherwise the numeric value entered is used as the ultimate for the development factor
#'    \eqn{f_ult}.
#' @param tail.se (numeric) dentoes the tail factor of \eqn{f_se}. If \code{tail = 0}, no \code{tail.se}
#'    is calculated, if \code{tail = 1}, and \code{tail.se = 0} a tail value is obtained by 
#'    log-linear regression or else the value entered is used.
#' @param tail.sigma (numeric) denotes the tail factor for \eqn{sigma}. The usage mirrors \code{tail.se}.
#' 
#' @return The chainladder function returns a list with all the calculated components as below:
#' 
#' \itemize{
#' \item{f} {the development factors.}
#' \item{f.se} {the standard error of the development factors.}
#' \item{sigma} {the \eqn{\sigma} value as described by Mack's 1999 paper.}
#' \item{Mack.S.E} {the standard error for the whole triangle.}
#' \item{C} {the completed claims triangle. If the tail factor was requested, the ultimates are the last column.}
#' \item{C.se} {the associated standard errors for the claims.}
#' \item{R} {the reserves (calculated from the \code{C} matrix for convenience).}
#' \item{F} {the individual development factors calculated from the triangle. Only the triangle is calculated.}
#' \item{F.se} {the full standard error matrix for the individual development factors.}
#' }
#' 
#' @references
#' Mack T., The Standard Error of Chain Ladder Reserve Estimates Recursive Calculation 
#'    and Inclusion of a tail factor, ASTIN BULLETIN, vol 29, No. 2, 1999, pp 361 - 366.
#'
#' Mack T., Distribution-Free Calculation of the Standard Error of Chain Ladder Reserve Estimates,
#' ASTIN BULLETIN, vol 23, No. 2, 1993, pp 213 - 225.
#' 
#' Gesmann M., Murphy D., Zhang W., ChainLadder: Statistical methods and models for the calculation 
#'  of outstanding claims reserves in general insurance.
#'
#' @seealso The \code{MackChainLadder} function in the \code{ChainLadder} package.
#'
#' @export
#' 
#' @examples
#' 
#' data(mortgage)
#' # Execution of the chainladder with no tail factor
#' chainladder(mortgage)
#' # Executiuon of chainladder with just development tail factor
#' chainladder(mortgage, tail = 1.05)
#' # Full specification of the tail factor
#' chainladder(mortgage, tail = 1.05, tail.se = 0.02, tail.sigma = 71)
#' 
chainladder <- function(tri, wghts = 1, alpha = 1, tail = 0, tail.se = 0, tail.sigma = 0)
{
  p = ncol(tri)
  if(wghts == 1 & length(wghts) == 1){
    wghts = matrix(1, ncol = p, nrow = p)
  }
  
  cl <- chainladder_(tri, wghts, alpha, tail, tail.se, tail.sigma)
  cl$C.se <- sqrt(cl$C.se)
  cl$f.se <- sqrt(cl$f.se)
  cl$sigma <- sqrt(cl$sigma)
  cl$F.se <- sqrt(cl$F.se)
  cl$Mack.S.E <- sqrt(cl$Mack.S.E)
  return(cl)
}
