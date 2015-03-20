object QuickSortConventional {

  def sort(xs: Array[Int]) {
    def swap(i: Int, j: Int) {
      val t = xs(i); xs(i) = xs(j); xs(j) = t
    }

    def sort1(l: Int, r: Int) {
      val pivot = xs((l + r) / 2)
      var i = l; var j = r
      while (i <= j) {
	while (xs(i) < pivot) i += 1
	while (xs(j) > pivot) j -= 1
	if (i <= j) {
	  swap(i, j)
	  i += 1
	  j -= 1
	}
      }
      if (l < j) sort1(l, j)
      if (j < r) sort1(i, r)
    }

    sort1(0, xs.length - 1)
  }

}


object QuickSortFunctional {

  def sort(xs: Array[Int]): Array[Int] = {
    if (xs.length <= 1) xs
    else {
      val pivot = xs(xs.length / 2)
      Array.concat(
	sort(xs filter (pivot >)),
	xs filter (pivot ==),
	sort(xs filter (pivot <)))
    }
  }

}


object SortingTest {

  def conventional() {
    println("*** Conventional ***")
    val a = Array(2, 1, 3)
    println("original: " + a.deep)
    QuickSortConventional.sort(a)
    println("sorted: " + a.deep)
  }

  def functional() {
    println("*** Functional ***")
    val a = Array(2, 1, 3)
    println("original: " + a.deep)
    val sorted = QuickSortFunctional.sort(a)
    println("sorted: " + sorted.deep)
  }

  def concatTest() {
    // val a = Array.concat(Array(1, 2), Array(3, 4, 5))
    // val a = Array.concat(Array("a", "bb"), Array("c", "dd", "e"))
    val a = Array.concat(Array('a', 'b'), Array('c', 'd', 'e'))
    println(a.deep)
  }

  def filterTest() {
    val a = Array(1, 2, 3, 4, 5)
    val b = a filter (3 <) 
    println(b deep)
  }

  def filterTest2() {
    val a = Array(1, 2, 3, 4, 5)

    def even(x: Int): Boolean = {
      (x % 2) == 0
    }

    def odd(x: Int): Boolean = {
      (x % 2) != 0
    }

    val b = a filter (odd)
    println(b deep)
  }

  def main(args: Array[String]) {
    //conventional()
    //functional()

    //concatTest()
    //filterTest()
    filterTest2()
  }
}
