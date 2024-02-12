QT += opengl

INCLUDEPATH += $$PWD

#CONFIG += c++17

SOURCES += \
        $$PWD/OpenFBX/src/libdeflate.c \
        $$PWD/OpenFBX/src/ofbx.cpp \
        $$PWD/armature.cpp \
        $$PWD/basescenewidget.cpp \
        $$PWD/joint.cpp \
        $$PWD/loader.cpp \
        $$PWD/material.cpp \
        $$PWD/model.cpp \
        $$PWD/scene.cpp

HEADERS += \
        $$PWD/OpenFBX/src/libdeflate.h \
        $$PWD/OpenFBX/src/ofbx.h \
        $$PWD/armature.h \
        $$PWD/basescenewidget.h \
        $$PWD/datastorage.h \
        $$PWD/joint.h \
        $$PWD/loader.h \
        $$PWD/material.h \
        $$PWD/model.h \
        $$PWD/openfbxqt.h \
        $$PWD/scene.h

RESOURCES += \
    $$PWD/OpenFBXQt-resources.qrc
