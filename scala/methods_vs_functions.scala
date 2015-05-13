object MethodsVsFunctions {

  def main(args: Array[String]) = {

    // NOTE: { and } can be dropped in all examples since there is only one statement.

    // *** method, explicit return type
    def addOneM1(x: Int): Double = { x + 1 }
    val m1 = addOneM1(1)
    println(m1)

    // *** method, implicit return type
    def addOneM2(x: Int) = { x + 1 }
    val m2 = addOneM1(2)
    println(m2)

    // *** function, explicit signature
    val addOneF1: (Int) => Double = (x: Int) => { x + 1 }
    val f1 = addOneF1(11)
    println(f1)

    // *** function, explicit signature, implicit parameter type
    val addOneF2: (Int) => Double = x => { x + 1 }
    val f2 = addOneF2(12)
    println(f2)

    // *** function, explicit signature, implicit parameter type, placeholder syntax
    val addOneF3: (Int) => Double = _ + 1
    val f3 = addOneF3(13)
    println(f3)

    // *** function, implicit signature (thus the parameter type must be explicit!)
    val addOneF4 = (x: Int) => { x + 1 }
    val f4 = addOneF4(14)
    println(f4)
  }
}
