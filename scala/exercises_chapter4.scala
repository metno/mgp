// 1
def areaOfCircle(r: Double) : Double = {
  math.pow(r, 2) * math.Pi
}

// 2
def areaOfCircle2(r: String) : Double = {
  math.pow(r.toDouble, 2) * math.Pi
}

// 3
@annotation.tailrec
def printTo50by5(v: Int = 5) : Unit = {
  if (v <= 50) {
    println(v)
    printTo50by5(v + 5)
  }
}

// 4
def formatMS(ms: Long) : String = {
  val msInSec : Long = 1000
  val msInMin : Long = msInSec * 60
  val msInHour : Long = msInMin * 60
  val msInDay : Long = msInHour * 24

  val msDays : Long = ms
  val days : Long = msDays / msInDay

  val msHours : Long = msDays % msInDay
  val hours : Long = msHours / msInHour

  val msMins : Long = msHours % msInHour
  val mins : Long = msMins / msInMin

  val msSecs : Long = msMins % msInMin
  val secs : Long = msSecs / msInSec

  s"days: $days; hours: $hours; mins: $mins; secs: $secs"
}

// 5
def pow1(x: Double, n: Int) : Double = {
  math.pow(x, n)
}

def pow2(x: Double, n: Int) : Double = {
  var a = x
  for (i <- 1 until n) a *= x
  a
}

@annotation.tailrec
def pow3(x: Double, n: Int) : Double = {
  if (n < 2) {
    x
  } else {
    x * pow3(x, n - 1)
  }
}
