@annotation.tailrec
def power(x: Int, n: Int, t: Int = 1): Int = {
  if (n < 1) t
  else power(x, n-1, x*t)
}

// Execution:
//
// power(2, 4)
// x=2, n=4, t=1
// else->
//   power(2, 3, 2*1=2)
//   x=2, n=3, t=2
//   else->
//     power(2, 2, 2*2=4)
//     x=2, n=2, t=4
//     else->
//       power(2, 1, 2*4=8)
//       x=2, n=1, t=8
//       else->
//         power(2, 0, 2*8=16)
//         x=2, n=0, t=16
//         if->16
