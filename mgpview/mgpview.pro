TEMPLATE = app
TARGET = earthsphereviewer 
QT += opengl 
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += coast_data.h glwidget.h gfxutils.h util3d.h cartesiankeyframe.h

SOURCES += main.cpp glwidget.cpp gfxutils.cpp util3d.cpp cartesiankeyframe.cpp

LIBS += -lglut -lGLU
