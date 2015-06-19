import scala.concurrent._
import ExecutionContext.Implicits.global
import scala.util.{Success, Failure}

val f: Future[Int] = Future {
  throw new Exception("hm...")
  4711
}

val firstOccurrence: Future[Int] = Future {
  val source = scala.io.Source.fromFile("myText.txt")
  source.toSeq.indexOfSlice("myKeyword")
}

firstOccurrence onComplete {
  case Success(x) => println("success: " + x)
  case Failure(x) => println("failure: " + x.getMessage)
}
