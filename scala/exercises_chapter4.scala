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

//@annotation.tailrec ### not tail-recursive, since it is the multiplication, not the recursive
// call, that is in tail position!
def pow3(x: Double, n: Int) : Double = {
  if (n < 2) {
    x
  } else {
    x * pow3(x, n - 1)
  }
}

@annotation.tailrec
def pow4(x: Double, n: Int, r: Double = 1.0) : Double = {
  if (n < 1) {
    r
  } else {
    pow4(x, n - 1, r * x)
  }
}

// 6
def dist(p1: (Double, Double), p2: (Double, Double)) : Double = {
  math.sqrt(math.pow(p2._1 - p1._1, 2) + math.pow(p2._2 - p1._2, 2))
}

// 7
def intInPos1(t: (Any, Any)) : (Any, Any) = {
  if (t._1.isInstanceOf[Int]) {
    t
  } else if (t._2.isInstanceOf[Int]) {
    (t._2, t._1)
  } else {
    throw new Exception("none of the components are of type Int")
  }    
}

// 8
def addStringRepr[T1, T2, T3](t: (T1, T2, T3)) : (T1, String, T2, String, T3, String) = {
  (t._1, t._1.toString, t._2, t._2.toString, t._3, t._3.toString)
}
