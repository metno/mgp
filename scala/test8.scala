class A(_s: String, _x: Int) {
  var __s = _s
  var __x = _x
  def s = __s
  def x() = __x
  def setX(_x: Int) { __x = _x }
}

object Test8 {
  def main(args: Array[String]) {
    val a = new A("bravo", 4711)
    println("a.s(): " + a.s + ", a.x(): " + a.x())
    a.setX(17)
    println("a.s(): " + a.s + ", a.x(): " + a.x())
  }
}
