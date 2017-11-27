TEMPLATE = lib
CONFIG += staticlib debug
QT += xml xmlpatterns widgets
TARGET = mgp 
SOURCES += mgpmath.cpp mgp.cpp xmetareaedit.cpp xmetareaeditdialog.cpp polygonintersector.cpp kml.cpp
HEADERS += mgpmath.h mgp.h xmetareaedit.h xmetareaeditdialog.h data/enor_fir.h data/enob_fir.h data/norway_municipalities.kml polygoninterse

RESOURCES = mgp.qrc

target.path = /usr/lib
INSTALLS += target

headersDataFiles.path = /usr/include/mgp
headersDataFiles.files = $$HEADERS
INSTALLS += headersDataFiles
