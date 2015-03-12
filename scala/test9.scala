class Complex(real: Double, imaginary: Double) {
  def re = real
  def im = imaginary
  override def toString() =
    "" + re + (if (im < 0) "" else "+") + im + "i"
}

object Test9 {
  def main(args: Array[String]) {
    val a = new Complex(1.2, -3.4)
    println("a.toString(): >" + a + "<")
  }
}
