object TimerAnonymous {
  def oncePerSecond(callback: () => Unit, n : Int) {
    for (i <- 1 to n) {
      print("[" + i + ":" + n + "] "); callback(); Thread sleep 1000
    }
  }

  def main(args: Array[String]) {
    oncePerSecond(() =>
      println("time flies like an arrow ... (first version, called 3 times)"), 3)
    oncePerSecond(() =>
      println("time flies like an arrow ... (second version, called 4 times)"), 4)
  }
}
