TEMPLATE = lib
CONFIG += staticlib
TARGET = qc
SOURCES += ../qclib/qc.cpp ../qclib/qcchat.cpp ../qclib/qcglobal.cpp
HEADERS += ../qclib/qc.h ../qclib/qcchat.h ../qclib/qcglobal.h
QT += network

target.path = /usr/lib
INSTALLS += target
