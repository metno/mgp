SOURCES += main.cpp
QCLIB_DIR = ../qclib
include($$QCLIB_DIR/qclib.pri)
TARGET = qccserver
QT += sql
