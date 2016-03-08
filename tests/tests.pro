CONFIG += qtestlib
TEMPLATE = app
TARGET = testmgp
DEPENDPATH += . ../lib
INCLUDEPATH += . ../lib

SOURCES += testmgp.cpp

LIBS += -L ../lib -lmgp
