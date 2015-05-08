@annotation.tailrec
def sum(n : Int, r : Int = 1) : Int = {
  if (n <= 1) r else sum(n - 1, n + r)
}

// Execution:
//
// sum(4)
// n=4, r=1
// else->
//   sum(3, 4+1=5)
//   n=3, r=5
//   else->
//     sum(2, 3+5=8)
//     n=2, r=8
//     else->
//       sum(1, 2+8=10)
//       n=1, r=10
//       if->10
