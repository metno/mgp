TEMPLATE = app
TARGET = mgpview 
QT += opengl 
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += /disk1/Downloads/boost_1_60_0

HEADERS += mgp.h mgpmath.h mainwindow.h common.h coast_data.h enor_fir.h glwidget.h gfxutils.h controlpanel.h textedit.h

SOURCES += mgp.cpp mgpmath.cpp main.cpp mainwindow.cpp glwidget.cpp gfxutils.cpp controlpanel.cpp textedit.cpp

LIBS += -lglut -lGLU
