TEMPLATE = app
TARGET = earthsphereviewer 
QT += opengl 
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += coast_data.h externalview.h earthspheregfxutil.h util3d.h cartesiankeyframe.h

SOURCES += main.cpp externalview.cpp earthspheregfxutil.cpp util3d.cpp cartesiankeyframe.cpp

LIBS += -lglut -lGLU
