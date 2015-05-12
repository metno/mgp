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

  val ms1 : Long = ms
  val days : Long = ms1 / msInDay

  val ms2 : Long = ms1 % msInDay
  val hours : Long = ms2 / msInHour

  val ms3 : Long = ms2 % msInHour
  val mins : Long = ms3 / msInMin

  val ms4 : Long = ms3 % msInMin
  val secs : Long = ms4 / msInSec

  s"days: $days; hours: $hours; mins: $mins; secs: $secs"
}

// 5
