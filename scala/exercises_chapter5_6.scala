def conditional[A](x: A, p: A => Boolean, f: A => A): A = {
  if (p(x)) f(x) else x
}

// option 1:
conditional(10, { x : Int => x > 10 }, { x : Int => x + 1 }) // -> 10 (p(x) was false)
conditional(11, { x : Int => x > 10 }, { x : Int => x + 1 }) // -> 12 (p(x) was true)

// option 2:
val p: Int => Boolean = _ > 10
val f: Int => Int = _ + 1
conditional(10, p, f)
conditional(11, p, f)
