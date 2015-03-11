object Math {
  def fact(n: Long) : Long = {
    var f = 1L
    if (n > 1) f = n * fact(n - 1L)
    f
  }
}

object Factorial {
  def main(args: Array[String]) {
    var n : Long = 0
    try {
      n = args(0).toLong
    } catch {
      case _ => {
	println("expected an integer argument")
	System.exit(1)
      }
    }

    var f : Long = 0
    try {
      f = Math.fact(n)
    } catch {
      case _ => {
	println("computation failed")
	System.exit(1)
      }
    }
    println(n + "! = " + f)
  }
}
