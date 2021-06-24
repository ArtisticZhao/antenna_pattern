QT       += core gui network serialport
QT +=  charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat

CONFIG      += warn_off

#DEFINES *= QWT_DLL  QWT_POLAR_DLL
#CONFIG += qwt qwtpolar
#LIBS += -Ld:/Qt/5.15.2/msvc2019_64/lib -lqwt
#LIBS += -Ld:/Qt/5.15.2/msvc2019_64/lib -lqwtpolar
#INCLUDEPATH += D:/Qt/third-party/qwt-6.1.6/src
#INCLUDEPATH += D:/Qt/third-party/qwtpolar-1.1.1/src

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

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
