object Math {

  def add(x: Long, y: Long) : Long = {
    x + y
  }

  def mul(x: Long, y: Long) : Long = {
    x * y
  }

  def addAndMul(x: Long, y: Long) : Long = {
    var z : Long = 0
    for (i <- 1 until 10000)
      z += (add(x, y) + mul(x, y) - i)
    z
  }
}

object Test {
  def getLong(s: String) : Long = {
    try {
      s.toLong
    } catch {
      case _ => println("failed to extract long integer from string >" + s + "<")
      sys.exit(1)
    }
  }

  def getMathFunction(s: String) : (Long, Long) => Long = {
    if (s == "add")
      return Math.add(_, _)
    else if (s == "mul")
      return Math.mul(_, _)
    else if (s == "both")
      return Math.addAndMul(_, _)

    println("failed to select math function from string >" + s + "<")
    sys.exit(1)
  }

  def main(args: Array[String]) = {

    if (args.length != 4) {
      println("usage: Test x y n add|mul|both")
      sys.exit(1)
    }

    val x = getLong(args(0))
    val y = getLong(args(1))
    val n = getLong(args(2))
    val f = getMathFunction(args(3))

    var z : Long = 0
    var i = 0L
    while (i < n) {
      val result = f(x, y)
      z += result
      i += 1
    }

    println("z: " + z)

    sys.exit(0)
  }
}
