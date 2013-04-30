TARGET = rovergauge
TEMPLATE = app
DEFINES += "ROVERGAUGE_VER_MAJOR=0"
DEFINES += "ROVERGAUGE_VER_MINOR=4"
DEFINES += "ROVERGAUGE_VER_PATCH=1"
sim-mode {
    DEFINES += "ENABLE_SIM_MODE"
}
LIBS += -lcomm14cux
win32 {
  LIBS += "-L$$(SystemDrive)/comm14cux"
  INCLUDEPATH += "$$(SystemDrive)/comm14cux/include"
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
    fueltrimbar.cpp \
    helpviewer.cpp \
    analogwidgets/manometer.cpp \
    analogwidgets/led.cpp \
    analogwidgets/abstractmeter.cpp \
    analogwidgets/widgetwithbackground.cpp \
    analogwidgets/functions.cpp \
    qledindicator/qledindicator.cpp
sim-mode {
    SOURCES += simulationmodedialog.cpp
}
HEADERS += mainwindow.h \
    optionsdialog.h \
    faultcodedialog.h \
    serialdevenumerator.h \
    cuxinterface.h \
    aboutbox.h \
    logger.h \
    commonunits.h \
    idleaircontroldialog.h \
    fueltrimbar.h \
    helpviewer.h \
    analogwidgets/manometer.h \
    analogwidgets/led.h \
    analogwidgets/abstractmeter.h \
    analogwidgets/widgetwithbackground.h \
    analogwidgets/functions.h \
    qledindicator/qledindicator.h
sim-mode {
    HEADERS += simulationmodedialog.h
}
FORMS += mainwindow.ui

RESOURCES += \
    rovergauge_resources.qrc
