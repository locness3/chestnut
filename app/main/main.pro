include(../../project-settings.pri)

QT += core gui multimedia opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = chestnut
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += ../

SOURCES += main.cpp


FORMS +=

CONFIG += link_pkgconfig
PKGCONFIG += libavutil libavformat libavcodec libavfilter libswscale libswresample


LIBS += -L../$${DESTDIR}/ -lchestnut
CONFIG(coverage) {
    LIBS += -lgcov
}

PRE_TARGETDEPS += ../$${DESTDIR}/libchestnut.a


RESOURCES += \
    ../icons/icons.qrc

isEmpty(PREFIX) {
    PREFIX = /usr/local
}

target.path = $$PREFIX/bin

effects.files = $$PWD/effects/*.frag $$PWD/effects/*.xml $$PWD/effects/*.vert
effects.path = $$PREFIX/share/olive-editor/effects

metainfo.files = $$PWD/packaging/linux/org.olivevideoeditor.Olive.appdata.xml
metainfo.path = $$PREFIX/share/metainfo
desktop.files = $$PWD/packaging/linux/org.olivevideoeditor.Olive.desktop
desktop.path = $$PREFIX/share/applications
mime.files = $$PWD/packaging/linux/org.olivevideoeditor.Olive.xml
mime.path = $$PREFIX/share/mime/packages
icon16.files = $$PWD/packaging/linux/icons/16x16/org.olivevideoeditor.Olive.png
icon16.path = $$PREFIX/share/icons/hicolor/16x16/apps
icon32.files = $$PWD/packaging/linux/icons/32x32/org.olivevideoeditor.Olive.png
icon32.path = $$PREFIX/share/icons/hicolor/32x32/apps
icon48.files = $$PWD/packaging/linux/icons/48x48/org.olivevideoeditor.Olive.png
icon48.path = $$PREFIX/share/icons/hicolor/48x48/apps
icon64.files = $$PWD/packaging/linux/icons/64x64/org.olivevideoeditor.Olive.png
icon64.path = $$PREFIX/share/icons/hicolor/64x64/apps
icon128.files = $$PWD/packaging/linux/icons/128x128/org.olivevideoeditor.Olive.png
icon128.path = $$PREFIX/share/icons/hicolor/128x128/apps
icon256.files = $$PWD/packaging/linux/icons/256x256/org.olivevideoeditor.Olive.png
icon256.path = $$PREFIX/share/icons/hicolor/256x256/apps
icon512.files = $$PWD/packaging/linux/icons/512x512/org.olivevideoeditor.Olive.png
icon512.path = $$PREFIX/share/icons/hicolor/512x512/apps
icon1024.files = $$PWD/packaging/linux/icons/1024x1024/org.olivevideoeditor.Olive.png
icon1024.path = $$PREFIX/share/icons/hicolor/1024x1024/apps
INSTALLS += target effects metainfo desktop mime icon16 icon32 icon48 icon64 icon128 icon256 icon512 icon1024
