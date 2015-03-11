object Timer {
  def f(h: (Int) => Unit, msecs: Int) {
    var i = 0
    while (true) {
      h(i)
      i = i + 1
      /* Thread sleep msecs */ // option 1
      Thread sleep msecs // option 2
    }
  }

  def g(i: Int) {
    println("g(), i: " + i)
  }

  def main(args: Array[String]) {
    val msecs = args(0).toInt
    f(g, msecs)
  }
}
