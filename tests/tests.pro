CONFIG += qtestlib
QT += xml xmlpatterns
TEMPLATE = app
TARGET = testmgp

INCLUDEPATH += . ../lib
DEPENDPATH += . ../lib
PRE_TARGETDEPS += ../lib/libmgp.a

SOURCES += testmgp.cpp
HEADERS += testmgp.h

LIBS += -L ../lib -lmgp
