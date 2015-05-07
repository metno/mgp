for (i <- 1 to 10) print(i + " ")
println

var seq = for (i <- 1 to 10) yield i

println("\nseq:" + seq)

println("\nnested for-loop:")
for { i <- 0 until 4; j <- 0 until 3 } println(s"i: $i, j: $j")

println("\nfor-loop with iterator guard and value binding:")
for { i <- 0 until 10 if i % 2  == 0; j = i * 10 } println(j)
