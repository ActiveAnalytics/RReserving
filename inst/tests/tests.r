library(testthat)
library(RReserving)

context("Simple tests for the Reserving package")

output <- chainladder(mortgage, tail = 1.05, tail.se = 0.02, tail.sigma = 71)

test_that("We get the output length we expect", {
	expect_that(length(output), equals(9))
})

test_that("We get the object type we expect", {
	expect_that(class(output), equals("list"))
})


objNames <- c("f", "f.se", "sigma", "Mack.S.E", "C", "C.se", "R", "F", "F.se")
test_that("The names vector of our object is what we expect",{
	expect_that(names(output), equals(objNames))
})

test_that("Test that we get the results we expect", {
  expect_that(round(output$f, 3), equals(testdat$f))
  expect_that(round(output$f.se, 3), equals(testdat$f.se))
  expect_that(round(output$sigma), equals(testdat$sigma))
  expect_that(round(output$Mack.S.E), equals(testdat$Mack.S.E))
  expect_that(round(output$C), equals(testdat$C))
  expect_that(round(output$C.se), equals(testdat$C.se))
  expect_that(round(output$R), equals(testdat$R))
  expect_that(round(output$F, 4), equals(testdat$F))
  expect_that(round(output$F.se, 4), equals(testdat$F.se))
})

