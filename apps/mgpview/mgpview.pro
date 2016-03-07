TEMPLATE = app
TARGET = mgpview 
QT += opengl 
DEPENDPATH += . ../../lib
INCLUDEPATH += . ../../lib

HEADERS += mainwindow.h common.h coast_data.h enor_fir.h glwidget.h gfxutils.h controlpanel.h textedit.h
SOURCES += main.cpp mainwindow.cpp glwidget.cpp gfxutils.cpp controlpanel.cpp textedit.cpp

LIBS += -L ../../lib -lmgp -lglut -lGLU
