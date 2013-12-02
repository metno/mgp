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

class CallgrindTester : public QObject
{
  Q_OBJECT
public:
  CallgrindTester();
public slots:
  bool start(int nSteps, int stepDelay, QObject *testObject, const char *stepAction);
private slots:
  void advanceTest();
private:
  bool active_;
  int step_;
  int nSteps_;
  int stepDelay_;
  QObject *testObject_;
  const char *stepAction_;
};

#endif // CALLGRINDTESTER_H
