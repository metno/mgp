/*

    The class CallgrindTester is a simple API to facilite callgrind-profiling.

    Typical usage:

    STEP 1: Write the test code, essentially something like this:

        #include "callgrindtester.h"
        ...
        TestObject testObj;
        CallcrindTester cgTester;
        cgTester.start(10, 500, &testObj, "action"); // this starts the test
        ...
        void TestObject::action(int step, int nSteps) // this public slot is called at each test step
        {
          qDebug() << "action:" << step << ":" << nSteps;
          // do something interesting, like posting a MouseMove event etc.
        }

    STEP 2: Build the test code and execute it through valgrind/callgrind:

        $ valgrind --tool=callgrind --instr-atstart=no <executable>

    STEP 3: Analyze the result:

        $ kcachegrind callgrind.out.<pid>

*/

#ifndef CALLGRINDTESTER_H
#define CALLGRINDTESTER_H

#include <QtCore>
#include "/usr/include/valgrind/callgrind.h"

class CallgrindTester : public QObject
{
  Q_OBJECT
public:
  CallgrindTester()
    : active_(false)
    , step_(0)
    , nSteps_(0)
    , stepDelay_(0)
    , testObject_(0)
    , stepAction_(0)
  {
  }

public slots:

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
  bool start(int nSteps, int stepDelay, QObject *testObject, const char *stepAction)
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

private slots:

  void advanceTest()
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

private:
  bool active_;
  int step_;
  int nSteps_;
  int stepDelay_;
  QObject *testObject_;
  const char *stepAction_;
};

#endif // CALLGRINDTESTER_H
