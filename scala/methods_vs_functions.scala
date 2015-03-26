object MethodsVsFunctions {

  def main(args: Array[String]) = {

    // NOTE: { and } can be dropped in all examples since there is only one statement.

    // *** method, implicit return type
    def addOneM1(x: Int) = { x + 1 }
    val m1 = addOneM1(1)
    println(m1)

    // *** method, explicit return type
    def addOneM2(x: Int): Double = { x + 1 }
    val m2 = addOneM2(2)
    println(m2)

    // *** function, implicit return type
    val addOneF1 = (x: Int) => { x + 1 }
    val f1 = addOneF1(11)
    println(f1)

    // *** function, explicit return type
    val addOneF2: (Int) => Double = (x: Int) => { x + 1 }
    val f2 = addOneF2(12)
    println(f2)
  }
}
