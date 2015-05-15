def square(m: Double) = m * m

// WRONG:
val sq = square // would be interpreted as an invocation of square, not assignment of a function value

// RIGHT:
val sq : Double => Double = square // using explicit signature
val sq = square _ // using currying
