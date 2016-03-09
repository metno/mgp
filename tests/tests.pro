CONFIG += qtestlib
TEMPLATE = app
TARGET = testmgp

INCLUDEPATH += . ../lib
DEPENDPATH += . ../lib
PRE_TARGETDEPS += ../lib/libmgp.a

SOURCES += testmgp.cpp

LIBS += -L ../lib -lmgp
