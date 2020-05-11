QT += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Metrology
TEMPLATE = app
DEFINES += QT_DEPRECATED_WARNINGS
CONFIG += qwt
QMAKE_CXXFLAGS += -std=gnu++11
SOURCES += \
        algorithms.cpp \
        delegate.cpp \
        emrmodel.cpp \
        graphic.cpp \
        main.cpp \
        mainwindow.cpp \
        tablemodel.cpp

HEADERS += \
        algorithms.h \
        delegate.h \
        emrmodel.h \
        graphic.h \
        mainwindow.h \
        tablemodel.h

RESOURCES     = mainwindow.qrc
unix {
#LIBS += -ltbb
} else {
#LIBS += tbb.lib
}

include(3rdparty/qtxlsx/src/xlsx/qtxlsx.pri)

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
