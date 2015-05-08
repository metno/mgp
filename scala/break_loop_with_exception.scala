// one way to break out of a loop:

object AllDone extends Exception {}

try {
  for (i <- 1 to 1000000000) {
    if (i > 5)
      throw AllDone
    print(i + " ")
  }
} catch {
  case AllDone => println("caught AllDone")
}

println("done")
