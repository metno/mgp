// 1
val max = (a: Int, b: Int) => math.max(a, b)
def maxOf3(t: (Int, Int, Int), maxOf2: (Int, Int) => Int) = {
  maxOf2(t._1, maxOf2(t._2, t._3))
}
maxOf3((5, 3, 6), max) // -> 6
