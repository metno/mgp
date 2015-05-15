// Note difference between:
//
//   def f(a: Int, b: Int, g: (Int, Int) => Int) = { println(s"g($a, $b) ..."); g(a, b) }
//   f(4, 3, { println("max 1 ..."); math.max(_, _) }) // NOTE: here the we could have left out "(_, _)" !
//
// which produces the following output:
//
//   max 1 ...
//   g(4, 3) ...
//   res0: Int = 4
//
// and
//
//   def f(a: Int, b: Int, g: (Int, Int) => Int) = { println(s"g($a, $b) ..."); g(a, b) }
//   val max = (x: Int, y: Int) => { println("max 2 ..."); math.max(x, y) }
//   f(4, 3, max) // NOTE: here we could have passed { println("passing max ..."); max } as the 3rd argument!
//
// which produces the following output:
//
//   g(4, 3) ...
//   max 2 ...
//   res0: Int = 4
//
// In the first case, the expression block passed as the 3rd argument to f is evaluated before f (hence the
// "max 1 ..." msg, and the "(Int, Int) => Int" return value; the latter is then applied as g in the body of f).
// In the second case, the function value (max) passed as the 3rd argument isn't applied before the body of f.
// 


def f(a: Int, b: Int, g: (Int, Int) => Int) = { println(s"g($a, $b) ..."); g(a, b) }
println

val max = (x: Int, y: Int) => { println(s"max($x, $y) ..."); math.max(x, y) }
f(util.Random.nextInt, util.Random.nextInt, max)
println

val min = (x: Int, y: Int) => { println(s"min($x, $y) ..."); math.min(x, y) }
f(util.Random.nextInt, util.Random.nextInt, min)
println

val snd = (x: Int, y: Int) => { println(s"snd($x, $y) ..."); y }
f(util.Random.nextInt, util.Random.nextInt, snd)
