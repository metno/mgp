TEMPLATE = app
TARGET = mgpview 
QT += opengl 

INCLUDEPATH += . ../../lib
DEPENDPATH += . ../../lib
PRE_TARGETDEPS += ../../lib/libmgp.a

HEADERS += mainwindow.h common.h coast_data.h enor_fir.h glwidget.h gfxutils.h controlpanel.h textedit.h
SOURCES += main.cpp mainwindow.cpp glwidget.cpp gfxutils.cpp controlpanel.cpp textedit.cpp

LIBS += -L ../../lib -lmgp -lglut -lGLU

target.path = $$PREFIX/bin
INSTALLS += target
