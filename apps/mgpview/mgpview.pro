TEMPLATE = app
TARGET = mgpview 
QT += opengl 

CONFIG += debug


INCLUDEPATH += . ../../lib
DEPENDPATH += . ../../lib
PRE_TARGETDEPS += ../../lib/libmgp.a

HEADERS += mainwindow.h common.h coast_data.h glwidget.h gfxutils.h controlpanel.h
SOURCES += main.cpp mainwindow.cpp glwidget.cpp gfxutils.cpp controlpanel.cpp

LIBS += -L ../../lib -lmgp -lglut -lGLU

BINDIR = $$PREFIX/bin
DATADIR = $$PREFIX/share

target.path = $$BINDIR

desktop.path = $$DATADIR/applications
desktop.files += ../../desktop/metno-$${TARGET}.desktop

icon64.path = $$DATADIR/pixmaps
icon64.files += ../../icons/$${TARGET}.png

INSTALLS += target desktop icon64
