QT       += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include(../OpenFBXQt/OpenFBXQt.pri)

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    scenewidget.cpp

HEADERS += \
    mainwindow.h \
    scenewidget.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32: {
    CONFIG(debug, debug|release) {
        #debug
    } else {
        #release
        contains(QT_ARCH, i386) {
            DESTDIR = $$_PRO_FILE_PWD_/../release_win32
        } else {
            DESTDIR = $$_PRO_FILE_PWD_/../release_win64
        }

        #QMAKE_POST_LINK += $$(QTDIR)/bin/windeployqt --release --qmldir $$(QTDIR)/qml $$DESTDIR $$escape_expand(\\n\\t) # In Qt 5.15 with --release not working
        QMAKE_POST_LINK += $$(QTDIR)/bin/windeployqt --qmldir $$(QTDIR)/qml $$DESTDIR $$escape_expand(\\n\\t)
    }
}

RESOURCES += \
    resources.qrc
