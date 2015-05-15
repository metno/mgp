def createMultiplier(x: Int) : Int => Int = _ * x
//def createMultiplier(x: Int) : Int => Int =             { _ * x }   EQUIVALENT
//def createMultiplier(x: Int) : Int => Int = (y: Int) =>   y * x        ''
//def createMultiplier(x: Int) : Int => Int = (y: Int) => { y * x }      ''

val mul2 = createMultiplier(2)
mul2(3) // -> 6

val mul10 = createMultiplier(10)
mul10(3) // -> 30
