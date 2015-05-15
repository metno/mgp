// fzero allows a function (f) of type A => Unit to be called with a fixed argument (x)
def fzero[A](x: A)(f: A => Unit): A = { f(x); x }

// option 1: (predefining both x and f)
val g = fzero(3)(println)
g // this call now is effectively this: { println(3); 3 }

// Option 2: (predefing x only, i.e. _currying_ (the first part of the chained) fzero (with 3))
val g = fzero(3) _
g(println) // this call is still effectively this: { println(3); 3 }, only f needed to be passed in the call to g

// other examples
g({ y => println("inline ... arg = " + y) })

def h : Int => Unit = { y => println("h() ... arg = " + y) }
g(h)
