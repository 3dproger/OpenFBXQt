QT += opengl

INCLUDEPATH += $$PWD

SOURCES += \
        $$PWD/OpenFBX/src/miniz.c \
        $$PWD/OpenFBX/src/ofbx.cpp \
        $$PWD/basescenewidget.cpp \
        $$PWD/joint.cpp \
        $$PWD/loader.cpp \
        $$PWD/material.cpp \
        $$PWD/model.cpp \
        $$PWD/scene.cpp \
        $$PWD/skeleton.cpp

HEADERS += \
        $$PWD/OpenFBX/src/miniz.h \
        $$PWD/OpenFBX/src/ofbx.h \
        $$PWD/basescenewidget.h \
        $$PWD/joint.h \
        $$PWD/loader.h \
        $$PWD/material.h \
        $$PWD/model.h \
        $$PWD/modeldatastorage.h \
        $$PWD/openfbxqt.h \
        $$PWD/scene.h \
        $$PWD/skeleton.h

RESOURCES += \
    $$PWD/OpenFBXQt-resources.qrc
