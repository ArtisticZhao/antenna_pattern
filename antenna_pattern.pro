QT       += core gui network serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat

CONFIG      += warn_off
DEFINES *= QWT_DLL  QWT_POLAR_DLL
CONFIG += qwt qwtpolar
LIBS += -Ld:/Qt/5.15.2/msvc2019_64/lib -lqwt
LIBS += -Ld:/Qt/5.15.2/msvc2019_64/lib -lqwtpolar
INCLUDEPATH += D:/Qt/third-party/qwt-6.1.6/src
INCLUDEPATH += D:/Qt/third-party/qwtpolar-1.1.1/src
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    QTelnet.cpp \
    main.cpp \
    mainwindow.cpp \
    plot.cpp \
    rotatorprotocol.cpp

HEADERS += \
    QTelnet.h \
    mainwindow.h \
    plot.h \
    rotatorprotocol.h

FORMS += \
    mainwindow.ui

INCLUDEPATH += $$PWD/3rd_qextserialport
include ($$PWD/3rd_qextserialport/3rd_qextserialport.pri)

include(D:/Qt/third-party/qwt-6.1.6/qwtfunctions.pri)

#INCLUDEPATH += D:/Qt/third-party/qwt-6.1.6/src
#DEPENDPATH += D:/Qt/third-party/qwt-6.1.6/src
#qwtAddLibrary(qwt)
#end qwt
##qwt设置
#QWT_ROOT = d:/Qt/third-party/qwt-6.1.6
#include( $${QWT_ROOT}/qwtfunctions.pri )
#INCLUDEPATH += $${QWT_ROOT}/src
#DEPENDPATH  += $${QWT_ROOT}/src
#contains(QWT_CONFIG, QwtFramework) {
#    LIBS      += -F$${QWT_ROOT}/lib
#}
#else {
#    LIBS      += -L$${QWT_ROOT}/lib
#}
#qwtAddLibrary(qwt)
#contains(QWT_CONFIG, QwtOpenGL ) {
#    QT += opengl
#}
#else {
#    DEFINES += QWT_NO_OPENGL
#}
#contains(QWT_CONFIG, QwtSvg) {
#    QT += svg
#}
#else {
#    DEFINES += QWT_NO_SVG
#}
#win32 {
#    contains(QWT_CONFIG, QwtDll) {
#        DEFINES    += QT_DLL QWT_DLL
#    }
#}
##end qwt set

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
