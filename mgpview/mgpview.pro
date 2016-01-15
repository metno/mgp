TEMPLATE = app
TARGET = mgpview 
QT += opengl 
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += mainwindow.h common.h coast_data.h glwidget.h gfxutils.h util3d.h cartesiankeyframe.h

SOURCES += main.cpp mainwindow.cpp glwidget.cpp gfxutils.cpp util3d.cpp cartesiankeyframe.cpp

LIBS += -lglut -lGLU
