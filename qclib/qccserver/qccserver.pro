SOURCES += main.cpp
QCLIB_DIR = ../qclib
QCLIB_STATIC_DIR = ../qclib_static
include($$QCLIB_STATIC_DIR/qclib_static.pri)
TARGET = qccserver
QT += sql

target.path = /usr/bin
INSTALLS += target
