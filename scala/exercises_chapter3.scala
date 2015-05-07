// 1
val x = " "
val res = x match {
  case y if y.trim() == "" => "n/a"
  case y => y
}
println("res: " + res)

// 2
// skipped

// 3
val colName = "cyAn"
val hexString = colName.toLowerCase() match {
  case "cyan" => "00ffff"
  case "magenta" => "ff00ff"
  case "yellow" => "ffff00"
  case _ => "error!"
}
println("hexString: " + hexString)

// 4
for (i <- 1 to 100) {
  print(s"$i, ")
  if (i % 5 == 0)
    println
}

// 5
for (i <- 1 to 100) {
  print(i)
  i match {
    case x if (x % 3 == 0) && (x % 5 == 0) => println(" typesafe")
    case x if (x % 3 == 0) => println(" type")
    case x if (x % 5 == 0) => println(" safe")
    case _ => println
  }
}

// 6
for (i <- 1 to 100) { println(s"$i " + (if (i % 3 == 0) "type" else "") + ((if (i % 5 == 0) "safe" else ""))) }
