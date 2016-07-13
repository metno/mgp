TEMPLATE = lib
CONFIG += staticlib debug
QT += xml xmlpatterns
TARGET = mgp 
SOURCES += mgpmath.cpp xmetareaedit.cpp mgp.cpp polygonintersector.cpp kml.cpp
HEADERS += mgpmath.h mgp.h data/enor_fir.h data/enob_fir.h data/norway_municipalities.kml polygonintersector.h kml.h
RESOURCES = mgp.qrc

target.path = /usr/lib
INSTALLS += target

headersDataFiles.path = /usr/include/mgp
headersDataFiles.files = $$HEADERS
INSTALLS += headersDataFiles
