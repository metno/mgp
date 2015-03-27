/*

Every function in Scala can be treated as an object and it works the
other way too - every object can be treated as a function, provided it
has the apply method.

*/

class Action(x: Int) {
  def printInfo() = {
    println("this is an Action instance, x = " + x + ", hash = " + {hashCode() toHexString})
  }
}

object Action {
  final def apply(block: => Int) : Action = { println("ok1"); new Action(block) }
}

object TestObject1 {
  final def apply() = println("called apply() on TestObject1 ...")

  final def apply(block: => Int) = println("called apply(block: => Int) on TestObject1 ... block = " + block)
  // this is equivalent and will be prioritized:
  final def apply(block: Int) = println("called apply(block: Int) on TestObject1 ... block = " + block)
}

object TestObject2 {
  final def apply() = println("called apply() on TestObject2 ...")
  final def apply(block: () => Int) = println(
    "called apply(block: () => Int) on TestObject2 ... block = " + block + ", block() = " + block())
}

object ApplyTest {

  def main(args: Array[String]) = {

    def test1() {
      val act = Action {
	17
      }
      println(act)
      act.printInfo()
    }

    def test2() {
      def f(x: Int) = Action {
	x + 1
      }

      val act2 = f(13)
      println(act2)
      act2.printInfo()
    }

    def test3() {
      // *** TestObject1 ******************************************************************

      // calling first version of apply()
      TestObject1()

      // calling second version of apply() in different ways
      TestObject1(   4711   )
      TestObject1( { 4711 } )
      TestObject1  { 4711 }
      TestObject1( ( 4711 ) )
    }

    def test4() {
      // *** TestObject2 ******************************************************************

      // calling first version of apply()
      TestObject2()

      // calling second version of apply() in different ways
      TestObject2(        () =>   4711 )
      TestObject2(        () => ( 4711 ) )
      TestObject2(    (   () =>   4711 ) )
      TestObject2({       () =>   4711 } )
      TestObject2({ { ( { () =>   4711 } ) } } )
    }

    test1()
    test2()
    test3()
    test4()

  }
}
