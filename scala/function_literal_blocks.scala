// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Basis:

def applyForNonNeg(x: Double, op: Double => Double) = {
  if (x < 0) {
    println("warning: negative argument not supported: " + x)
    0.0
  } else {
    op(x)
  }
}

val mySqrt : Double => Double = math.sqrt(_)

applyForNonNeg(9.0, mySqrt) // 3.0
applyForNonNeg(-2.0, mySqrt) // 0.0


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Introducing funton literal block, various options

applyForNonNeg(9.0, { y => math.sqrt(y) })

applyForNonNeg(9.0, { y => mySqrt(y) })

// splitting into parameter groups to organize syntax in a better way (i.e. now we invoke
// applyForNonNeg with expression block syntax):
def applyForNonNeg(x: Double)(op: Double => Double) = {
  if (x < 0) {
    println("warning: negative argument not supported: " + x)
    0.0
  } else {
    op(x)
  }
}

applyForNonNeg(9.0) { y => math.sqrt(y) }

applyForNonNeg(9.0) { y => // y denotes the argument that applyForNonNeg passes to op !
  println("we can put anything we like in the expression block!")
  math.sqrt(y)
}

// We now have a cleaner invocation of applyForNonNeg, passing it the value parameter in
// parentheses and the function parameter as a free-standing function literal block.
