TEMPLATE = app
TARGET = mgpview 
QT += opengl 

INCLUDEPATH += . ../../lib
DEPENDPATH += . ../../lib
PRE_TARGETDEPS += ../../lib/libmgp.a

HEADERS += mainwindow.h common.h coast_data.h enor_fir.h enob_fir.h glwidget.h gfxutils.h controlpanel.h textedit.h
SOURCES += main.cpp mainwindow.cpp glwidget.cpp gfxutils.cpp controlpanel.cpp textedit.cpp

LIBS += -L ../../lib -lmgp -lglut -lGLU

BINDIR = $$PREFIX/bin
DATADIR = $$PREFIX/share

target.path = $$BINDIR

desktop.path = $$DATADIR/applications
desktop.files += ../../desktop/metno-$${TARGET}.desktop

icon64.path = $$DATADIR/pixmaps
icon64.files += ../../icons/$${TARGET}.png

INSTALLS += target desktop icon64
