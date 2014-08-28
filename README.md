RReserving: A package for Actuarial Pricing
==========================================

# Introduction

Welcome to the **RReserving** R package for carrying out actuarial reserving. This version of the package contains an algorithm for the Chain Ladder method based on Thomas Mack's 1999 paper <http://casualtyactuarialsociety.net/library/astin/vol29no2/361.pdf>. Note the name change from the initial announcement here <http://www.active-analytics.com/blog/productlaunch-actuarial-reserving/>.

## ChainLadder vs RReserving

Please note that this package is NOT intended to be a replacement for the `ChainLadder` package. This package does a tiny fraction of what the `ChainLadder` package does. The ChainLadder package has methods for Chain Ladder, bootstrapping, GLM, Cape Cod, Clark LDF, Multivariate Chain Ladder, etc. At the moment, we only have one method equivalent to the `MackChainLadder` function in the `ChainLadder` package but we get there very quickly indeed as the benchmark shows.

## Installation

The package can be installed directly from GitHub using the devtools <https://github.com/hadley/devtools> package:

```
require(devtools)
install_github("ActiveAnalytics/RReserving")
require(RReserving)
```

# Emphasis

The emphasis of the packages is on performance and content. All the components in the papers are calculated and the calculations in the R package are written in `C++` using the `Rcpp` & `RcppArmadillo` <http://www.rcpp.org/> packages.

## Performance

At Active Analytics the performance of our releases are important to us and the speed of the our chain ladder function rivals commercial or open implementations. Here is a benchmark of our R `RReserving` `chainladder` function against the `MackChainLadder` function from the `ChainLadder` package:

```
# We only need the MackChainLadder function and a dataset from the ChainLadder package
# Loading the function like this does not affect the benchmark
MackChainLadder <- ChainLadder::MackChainLadder
dat <- ChainLadder::Mortgage

# We use the microbenchmark function
mb = microbenchmark::microbenchmark

# We load our reserving package
require(RReserving)

# The benchmark for MackChainLadder() and our chainladder() function:
mb(chainladder(dat), MackChainLadder(dat), unit = "us", times = 100)

Unit: microseconds
                 expr       min         lq     median         uq       max  neval
     chainladder(dat)    37.346    41.1995    48.8075    69.3235   118.883   100
 MackChainLadder(dat) 24165.159 25623.6585 26337.6920 27088.3740 34485.472   100
```

## Content

Those already used to the ChainLadder package will find the form of our function familiar. The methods of execution of our chainladder function is given below. As with the MackChainLadder function the tail can be fully or partially specified.

```
# Fully specify the tail factor
output = chainladder(dat, tail = 1.05, tail.se = 0.02, tail.sigma = 71)
# Specify tail factor f only
output2 = chainladder(dat, tail = 1.05)
# Specify that the default tail factor should be calculated using interpolation
output2 = chainladder(dat, tail = 1)
# No tail factor calculated
output2 = chainladder(dat)
```

There is a `wghts` parameter that allows a weights matrix to be specified; the function also allows the `alpha` parameter to be specified as in the paper.

We can take a closer look at the object returned, we used the same data set as in Mack's 1999 <http://casualtyactuarialsociety.net/library/astin/vol29no2/361.pdf> paper so feel free to check the outputs.


```
names(output)
[1] "f"        "f.se"     "sigma"    "Mack.S.E" "C"        "C.se"     "R"
[8] "F"        "F.se"
```

Firstly we have the development factors

```
output$f
[1] 11.104259  4.092273  1.707913  1.275920  1.138912  1.068697  1.026335
[8]  1.022683  1.050000
```
Then we have the associated standard error

```
output$f.se
[1] 2.24376318 0.51681801 0.12200144 0.05117008 0.04207692 0.02303354 0.01465199
[8] 0.01222874 0.02000000
```
Then alpha as defined in the paper

```
output$sigma
[1] 1336.96847  988.47643  440.13971  206.98511  164.19978   74.60176   35.49316
[8]   16.88652   71.00000
```

The standard error of the total reserve

```
output$Mack.S.E
[1] 4053668
```
The completed claims triangle, if requested the last column is the tail factor derived ultimate ...

```
output$C
       [,1]   [,2]      [,3]    [,4]    [,5]    [,6]    [,7]    [,8]    [,9]    [,10]
 [1,] 58046 127970  476599.0 1027692 1360489 1647310 1819179 1906852 1950105  2047610
 [2,] 24492 141767  984288.0 2142656 2961978 3683940 4048898 4115760 4209118  4419573
 [3,] 32848 274682 1522637.0 3203427 4445927 5158781 5342585 5483281 5607658  5888041
 [4,] 21439 529828 2900301.0 4999019 6460112 6853904 7324745 7517641 7688163  8072571
 [5,] 40397 763394 2920745.0 4989572 5648563 6433219 6875160 7056216 7216272  7577086
 [6,] 90748 951994 4210640.0 5866482 7485161 8524943 9110579 9350505 9562602 10040732
 [7,] 62096 868480 1954797.0 3338623 4259816 4851558 5184844 5321386 5442091  5714195
 [8,] 24983 284441 1164010.4 1988029 2536565 2888926 3087386 3168691 3240567  3402595
 [9,] 13121 145699  596240.1 1018326 1299303 1479792 1581449 1623096 1659913  1742908
```

and the associated standard error

```
output$C.se
      [,1]     [,2]     [,3]    [,4]      [,5]      [,6]      [,7]      [,8]       [,9]     [,10]
 [1,]    0      0.0      0.0       0       0.0       0.0       0.0       0.0       0.00  106544.1
 [2,]    0      0.0      0.0       0       0.0       0.0       0.0       0.0   60883.43  179976.6
 [3,]    0      0.0      0.0       0       0.0       0.0       0.0  113393.5  139670.27  249707.6
 [4,]    0      0.0      0.0       0       0.0       0.0  251132.9  295260.4  319019.65  417857.0
 [5,]    0      0.0      0.0       0       0.0  456927.9  544256.1  575178.4  596210.29  670156.0
 [6,]    0      0.0      0.0       0  584337.0  862501.4  967279.5 1007399.6 1037861.76 1127984.1
 [7,]    0      0.0      0.0  659974  938781.2 1135845.3 1230032.2 1267288.3 1298251.31 1377496.2
 [8,]    0      0.0 547296.5 1058013 1384868.1 1602335.3 1718387.5 1765323.0 1806031.70 1901740.3
 [9,]    0 155949.8 745195.1 1319331 1697070.1 1942624.8 2078338.8 2133664.3 2182258.43 2293436.8
```

The reserves

```
output$R
[1]   97505.25  303813.39  545456.06 1218667.38 1928522.57 4174250.14 3759398.29
[8] 3118154.01 1729787.46
```

The individual development factors (calculated from the triangle) ...

```output$F
          [,1]     [,2]     [,3]     [,4]     [,5]     [,6]     [,7]     [,8]
[1,]  2.204631 3.724303 2.156303 1.323830 1.210822 1.104333 1.048194 1.022683
[2,]  5.788298 6.942998 2.176859 1.382386 1.243743 1.099067 1.016514 0.000000
[3,]  8.362214 5.543272 2.103868 1.387866 1.160339 1.035629 0.000000 0.000000
[4,] 24.713280 5.474043 1.723621 1.292276 1.060957 0.000000 0.000000 0.000000
[5,] 18.897294 3.825999 1.708322 1.132074 0.000000 0.000000 0.000000 0.000000
[6,] 10.490523 4.422969 1.393252 0.000000 0.000000 0.000000 0.000000 0.000000
[7,] 13.986086 2.250826 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000
[8,] 11.385382 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000
```

... and the associated standard errors:


```
output$F.se
           [,1]     [,2]      [,3]       [,4]       [,5]       [,6]       [,7]        [,8]       [,9]
 [1,]  5.549259 2.763199 0.6375496 0.20417738 0.14077485 0.05812479 0.02631521 0.012228739 0.05084279
 [2,]  8.542973 2.625299 0.4436387 0.14140439 0.09540731 0.03886806 0.01763909 0.008323676 0.03460691
 [3,]  7.376778 1.886041 0.3566912 0.11564629 0.07787381 0.03284547 0.01535567 0.007211405 0.02998248
 [4,]  9.131012 1.357997 0.2584455 0.09257564 0.06460298 0.02849575 0.01311440 0.006158846 0.02560631
 [5,]  6.651914 1.131337 0.2575394 0.09266324 0.06908815 0.02941270 0.01353640 0.006357029 0.02643029
 [6,]  4.438157 1.013093 0.2144947 0.08545751 0.06001668 0.02555073 0.01175903 0.005522333 0.02295992
 [7,]  5.365242 1.060685 0.3148036 0.11328056 0.07955677 0.03386948 0.01558751 0.007320281 0.03043515
 [8,]  8.458607 1.853405 0.4079549 0.14680059 0.10309784 0.04389155 0.02019990 0.009486373 0.03944100
 [9,] 11.671796 2.589632 0.5700067 0.20511416 0.14405137 0.06132658 0.02822390 0.013254643 0.05510813
```

## Future Updates

We hope that the next or future updates of the R RReserving package will provide tools to carry out the Bornhuetter-Ferguson.


### Contact Details
Chibisi Chima-Okereke: chibisi@activeanalytics.co.uk
