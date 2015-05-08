// Returns the sum of the numbers 1..n if n >= 1, otherwise -1.
def sum(n : Int) : Int = {
  n match {
    case x if x > 1 => x + sum(x - 1)
    case x if x == 1 => 1
    case x => -1
  }
}
