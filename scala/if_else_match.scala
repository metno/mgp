// Execute like this:
//
// $ /disk1/Downloads/scala-2.11.6/bin/scala
// ...
// scala> :load if_else_match.scala

var x = 8

val s1 = if (x == 6) {
  "x == 6"
} else if (x == 5) {
  "x == 5"
} else {
  s"x == $x ---> !(x == 5 || x == 6)"
}

val s2 = x match {
  case 6 => {
    println("foo")
    "x == 6.."
  }
  case 5 => "x == 5.."
  case _ => s"x == $x ---> !(x == 5 || x == 6).."
}

val s3 = x match {
  case 6 | 5 => {
    "x == 6 || x == 5 .."
  }
  case _ => s"x == $x ---> !(x == 5 || x == 6).."
}

val s4 = x match {
  case y if y > 7 => s"$y > 7"
  case y => s"$y <= 7"
}

val s5 = x match {
  case 6 => 106
  case 7 => 107
  case y => 200 + y
}
