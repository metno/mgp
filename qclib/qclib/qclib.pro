TEMPLATE = lib
CONFIG += shared
TARGET = qc
SOURCES += qc.cpp qcchat.cpp
HEADERS += qc.h qcchat.h
QT += network

target.path = /usr/lib
INSTALLS += target
