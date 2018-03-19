#-------------------------------------------------
#
# Project created by QtCreator 2018-03-18T18:44:55
#
#-------------------------------------------------
QT_MODULES = core gui opengl sql xml network
greaterThan(QT_MAJOR_VERSION, 4): QT_MODULES += widgets

QT       = $${QT_MODULES}

TARGET = lib3ds_qt
TEMPLATE = lib

DEFINES += LIB3DS_QT_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    model.cpp \
    lib3ds/atmosphere.c \
    lib3ds/background.c \
    lib3ds/camera.c \
    lib3ds/chunk.c \
    lib3ds/ease.c \
    lib3ds/file.c \
    lib3ds/io.c \
    lib3ds/light.c \
    lib3ds/material.c \
    lib3ds/matrix.c \
    lib3ds/mesh.c \
    lib3ds/node.c \
    lib3ds/quat.c \
    lib3ds/shadow.c \
    lib3ds/tcb.c \
    lib3ds/tracks.c \
    lib3ds/vector.c \
    lib3ds/viewport.c

HEADERS += lib3ds_qt_global.h \
    model.h \
    lib3ds/atmosphere.h \
    lib3ds/background.h \
    lib3ds/camera.h \
    lib3ds/chunk.h \
    lib3ds/chunktable.h \
    lib3ds/chunktable.sed \
    lib3ds/ease.h \
    lib3ds/file.h \
    lib3ds/io.h \
    lib3ds/light.h \
    lib3ds/material.h \
    lib3ds/matrix.h \
    lib3ds/mesh.h \
    lib3ds/node.h \
    lib3ds/quat.h \
    lib3ds/shadow.h \
    lib3ds/tcb.h \
    lib3ds/tracks.h \
    lib3ds/types.h \
    lib3ds/vector.h \
    lib3ds/viewport.h \
    gl_check_macro.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

DISTFILES += \
    lib3ds/types.txt


include(build_config.pri)
