def conditional[A](x: A, p: A => Boolean, f: A => A): A = {
  if (p(x)) f(x) else x
}

for (i <- 1 to 17)
  conditional(
    i,
    (x: Int) => if (((x % 3) == 0) || ((x % 5) == 0)) true else {
      println(x)
      false
    },
    (x: Int) => {
      if ((x % 3) == 0) print("type")
      if ((x % 5) == 0) print("safe")
      println
      -1
    }
  )


// alternative 1:

def conditional1[A](x: A, p: A => Boolean, f: A => A): Boolean = {
  if (p(x)) { f(x); true } else false
}

for (i <- 1 to 17)
  if (conditional1(
    i,
    (x: Int) => ((x % 3) == 0) || ((x % 5) == 0),
    (x: Int) => {
      if ((x % 3) == 0) print("type")
      if ((x % 5) == 0) print("safe")
      -1
    }))	{
    println
  } else {
    println(i)
  }
