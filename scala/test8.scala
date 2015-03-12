class A(_s: String, _x: Int) {
  def s() = _s
  def x() = _x * 2
}

object Test8 {
  def main(args: Array[String]) {
    var a = new A("bravo", 4711)
    println("a.s(): " + a.s() + ", a.x(): " + a.x())
  }
}
