TARGET = rovergauge
TEMPLATE = app
DEFINES += "ROVERGAUGE_VER_MAJOR=0"
DEFINES += "ROVERGAUGE_VER_MINOR=3"
DEFINES += "ROVERGAUGE_VER_PATCH=2"
LIBS += -lcomm14cux
win32 {
  LIBS += -LC:/comm14cux
  INCLUDEPATH += "C:/comm14cux/include"
}
SOURCES += main.cpp \
    mainwindow.cpp \
    optionsdialog.cpp \
    faultcodedialog.cpp \
    serialdevenumerator.cpp \
    cuxinterface.cpp \
    aboutbox.cpp \
    logger.cpp \
    idleaircontroldialog.cpp \
    fueltrimbar.cpp
HEADERS += mainwindow.h \
    optionsdialog.h \
    faultcodedialog.h \
    serialdevenumerator.h \
    cuxinterface.h \
    aboutbox.h \
    logger.h \
    commonunits.h \
    idleaircontroldialog.h \
    fueltrimbar.h
FORMS += mainwindow.ui
include(qledindicator/qledindicator.pri)
include(analogwidgets/analogwidgets.pri)

RESOURCES += \
    rovergauge_resources.qrc
