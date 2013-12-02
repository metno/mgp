#include "callgrindtester.h"
#include "/usr/include/valgrind/callgrind.h"

CallgrindTester::CallgrindTester()
  : active_(false)
  , step_(0)
  , nSteps_(0)
  , stepDelay_(0)
  , testObject_(0)
  , stepAction_("")
{
}

// Starts a test that runs in \a nSteps steps with a \a stepDelay milliseconds delay between each step.
// Upon each step, \a stepAction is called on \a testObject. stepAction must be a public slot with the
// following signature:
//
//     void stepAction(int step, int nSteps)
//
// where the first argument indicates the current step in the interval [1, nSteps].
//
// The function returns true iff the test could be started without problems.
// Problems are reported using qWarning().
bool CallgrindTester::start(int nSteps, int stepDelay, QObject *testObject, const char *stepAction)
{
  if (nSteps < 1) {
    qWarning("nSteps must be a positive integer: %d\n", nSteps);
    return false;
  }
  nSteps_ = nSteps;

  if (stepDelay_ < 0) {
    qWarning("nDelay must be a non-negative integer: %d\n", stepDelay);
    return false;
  }
  stepDelay_ = stepDelay;

  testObject_ = testObject;
  stepAction_ = stepAction;

  if (active_) {
    qWarning("a test is already active\n");
    return false;
  }
  active_ = true;
  step_ = 0;

  QTimer::singleShot(0, this, SLOT(advanceTest()));

  CALLGRIND_START_INSTRUMENTATION;
  CALLGRIND_ZERO_STATS;

  return true;
}

void CallgrindTester::advanceTest()
{
  step_++;

  QMetaObject::invokeMethod(testObject_, stepAction_, Qt::DirectConnection, Q_ARG(int, step_), Q_ARG(int, nSteps_));

  if (step_ < nSteps_) {
    QTimer::singleShot(stepDelay_, this, SLOT(advanceTest()));
  } else {
    CALLGRIND_DUMP_STATS;
    CALLGRIND_STOP_INSTRUMENTATION;
    active_ = false;
  }
}
