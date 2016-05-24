TEMPLATE = lib
CONFIG += staticlib debug
TARGET = mgp 
SOURCES += mgpmath.cpp xmetareaedit.cpp mgp.cpp polygonintersector.cpp
HEADERS += mgpmath.h mgp.h enor_fir.h enob_fir.h polygonintersector.h

#target.path = /usr/lib
#INSTALLS += target
