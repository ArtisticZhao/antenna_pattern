QT       += core gui network serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat

CONFIG      += warn_off

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    QTelnet.cpp \
    main.cpp \
    mainwindow.cpp \
    rotatorprotocol.cpp

HEADERS += \
    QTelnet.h \
    mainwindow.h \
    rotatorprotocol.h

FORMS += \
    mainwindow.ui

INCLUDEPATH += $$PWD/3rd_qextserialport
include ($$PWD/3rd_qextserialport/3rd_qextserialport.pri)

#qwt设置
QWT_ROOT = c:/Qt/thirdparty/qwt-6.1.4
include( $${QWT_ROOT}/qwtfunctions.pri )
INCLUDEPATH += $${QWT_ROOT}/src
DEPENDPATH  += $${QWT_ROOT}/src
contains(QWT_CONFIG, QwtFramework) {
    LIBS      += -F$${QWT_ROOT}/lib
}
else {
    LIBS      += -L$${QWT_ROOT}/lib
}
qwtAddLibrary(qwt)
contains(QWT_CONFIG, QwtOpenGL ) {
    QT += opengl
}
else {
    DEFINES += QWT_NO_OPENGL
}
contains(QWT_CONFIG, QwtSvg) {
    QT += svg
}
else {
    DEFINES += QWT_NO_SVG
}
win32 {
    contains(QWT_CONFIG, QwtDll) {
        DEFINES    += QT_DLL QWT_DLL
    }
}
#end qwt set

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
