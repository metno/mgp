// Examples of partial fuunctions, i.e. functions that accept only a subset
// of the possible values of the input type). NOTE: if we use the 'case _ ...' and 'case x ...'
// to catch all possible input, the term "partial function" wouldn't really be applicable
// anymore!

val f: Int => String = {
  case 1 => "one"
  case 6 => "six"
  //case _ => "unsupported!"
}
f(1) // one
f(6) // six
f(2) // runtime error!

val g: Int => String = {  
  case 1 => "one"
  case 6 => "six"
  //case x => s"unsupported: $x" // same as above, but now using value binding to show actual input
}
g(1) // one
g(6) // six
g(2) // runtime error!
