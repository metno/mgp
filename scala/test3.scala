
import java.util.{Date, Locale}
import java.text.DateFormat
import java.text.DateFormat._
object FrenchDate {
  def main(args: Array[String]) {
    val now = new Date(1428674211L * 1000L)
	val df = getDateInstance(LONG, Locale.FRANCE)
	println(df format now)
  }
}
