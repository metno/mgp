TEMPLATE = lib
CONFIG += shared
TARGET = qc
SOURCES += qc.cpp qcchat.cpp qcglobal.cpp
HEADERS += qc.h qcchat.h qcglobal.h
QT += network

target.path = /usr/lib
INSTALLS += target
