class Reference[T] {
  private var contents: T = _
  def set(value: T) { contents = value }
  def get: T = contents
}

object Test12 {

  def example1() {
    val x = new Reference[Int]
    //x.set(4711)
    x.set(Integer.parseInt("a4711", 16))
    println("x.get: " + x.get)
  }

  def example2() {
    val x = new Reference[String]
    x.set("bravo")
    println("x.get: " + x.get)
  }

  def main(args: Array[String]) {
    example1()
    example2()
  }
}
