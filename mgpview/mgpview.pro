TEMPLATE = app
TARGET = mgpview 
QT += opengl 
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += /disk1/Downloads/boost_1_60_0

HEADERS += mainwindow.h common.h coast_data.h enor_fir.h glwidget.h gfxutils.h util3d.h cartesiankeyframe.h controlpanel.h textedit.h

SOURCES += main.cpp mainwindow.cpp glwidget.cpp gfxutils.cpp util3d.cpp cartesiankeyframe.cpp controlpanel.cpp textedit.cpp

LIBS += -lglut -lGLU
