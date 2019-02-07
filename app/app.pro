#-------------------------------------------------
#
# Project created by QtCreator 2018-05-11T10:31:59
#
#-------------------------------------------------


include(../project-settings.pri)

TEMPLATE = lib
CONFIG += staticlib
TARGET = oliveeditor


SOURCES += \
    ui/mainwindow.cpp \
    panels/project.cpp \
    panels/effectcontrols.cpp \
    panels/viewer.cpp \
    panels/timeline.cpp \
    ui/sourcetable.cpp \
    dialogs/aboutdialog.cpp \
    ui/timelinewidget.cpp \
    project/media.cpp \
    project/footage.cpp \
    project/sequence.cpp \
    project/clip.cpp \
    playback/playback.cpp \
    playback/audio.cpp \
    io/config.cpp \
    dialogs/newsequencedialog.cpp \
    ui/viewerwidget.cpp \
    ui/viewercontainer.cpp \
    dialogs/exportdialog.cpp \
    ui/collapsiblewidget.cpp \
    panels/panels.cpp \
    io/exportthread.cpp \
    ui/timelineheader.cpp \
    io/previewgenerator.cpp \
    ui/labelslider.cpp \
    dialogs/preferencesdialog.cpp \
    ui/audiomonitor.cpp \
    project/undo.cpp \
    ui/scrollarea.cpp \
    ui/comboboxex.cpp \
    ui/colorbutton.cpp \
    dialogs/replaceclipmediadialog.cpp \
    ui/fontcombobox.cpp \
    ui/checkboxex.cpp \
    ui/keyframeview.cpp \
    ui/texteditex.cpp \
    dialogs/demonotice.cpp \
    project/marker.cpp \
    dialogs/speeddialog.cpp \
    dialogs/mediapropertiesdialog.cpp \
    io/crc32.cpp \
    project/projectmodel.cpp \
    io/loadthread.cpp \
    dialogs/loaddialog.cpp \
    debug.cpp \
    io/path.cpp \
    effects/internal/linearfadetransition.cpp \
    effects/internal/transformeffect.cpp \
    effects/internal/solideffect.cpp \
    effects/internal/texteffect.cpp \
    effects/internal/timecodeeffect.cpp \
    effects/internal/audionoiseeffect.cpp \
    effects/internal/paneffect.cpp \
    effects/internal/toneeffect.cpp \
    effects/internal/volumeeffect.cpp \
    effects/internal/crossdissolvetransition.cpp \
    effects/internal/shakeeffect.cpp \
    effects/internal/exponentialfadetransition.cpp \
    effects/internal/logarithmicfadetransition.cpp \
    effects/internal/cornerpineffect.cpp \
    io/math.cpp \
    io/qpainterwrapper.cpp \
    project/effect.cpp \
    project/transition.cpp \
    project/effectrow.cpp \
    project/effectfield.cpp \
    effects/internal/cubetransition.cpp \
    project/effectgizmo.cpp \
    io/clipboard.cpp \
    dialogs/stabilizerdialog.cpp \
    io/avtogl.cpp \
    ui/resizablescrollbar.cpp \
    ui/sourceiconview.cpp \
    project/sourcescommon.cpp \
    ui/keyframenavigator.cpp \
    panels/grapheditor.cpp \
    ui/graphview.cpp \
    ui/keyframedrawing.cpp \
    ui/clickablelabel.cpp \
    project/keyframe.cpp \
    ui/rectangleselect.cpp \
    dialogs/actionsearch.cpp \
    ui/embeddedfilechooser.cpp \
    effects/internal/fillleftrighteffect.cpp \
    effects/internal/voideffect.cpp \
    dialogs/texteditdialog.cpp \
    dialogs/debugdialog.cpp \
    project/projectitem.cpp \
    project/sequenceitem.cpp \
    ui/renderthread.cpp \
    ui/renderfunctions.cpp \
    ui/viewerwindow.cpp \
    project/projectfilter.cpp

HEADERS += \
    ui/mainwindow.h \
    panels/project.h \
    panels/effectcontrols.h \
    panels/viewer.h \
    panels/timeline.h \
    ui/sourcetable.h \
    dialogs/aboutdialog.h \
    ui/timelinewidget.h \
    project/media.h \
    project/footage.h \
    project/sequence.h \
    project/clip.h \
    playback/playback.h \
    playback/audio.h \
    io/config.h \
    dialogs/newsequencedialog.h \
    ui/viewerwidget.h \
    ui/viewercontainer.h \
    dialogs/exportdialog.h \
    ui/collapsiblewidget.h \
    panels/panels.h \
    io/exportthread.h \
    ui/timelinetools.h \
    ui/timelineheader.h \
    io/previewgenerator.h \
    ui/labelslider.h \
    dialogs/preferencesdialog.h \
    ui/audiomonitor.h \
    project/undo.h \
    ui/scrollarea.h \
    ui/comboboxex.h \
    ui/colorbutton.h \
    dialogs/replaceclipmediadialog.h \
    ui/fontcombobox.h \
    ui/checkboxex.h \
    ui/keyframeview.h \
    ui/texteditex.h \
    dialogs/demonotice.h \
    project/marker.h \
    project/selection.h \
    dialogs/speeddialog.h \
    dialogs/mediapropertiesdialog.h \
    io/crc32.h \
    project/projectmodel.h \
    io/loadthread.h \
    dialogs/loaddialog.h \
    debug.h \
    io/path.h \
    effects/internal/transformeffect.h \
    effects/internal/solideffect.h \
    effects/internal/texteffect.h \
    effects/internal/timecodeeffect.h \
    effects/internal/audionoiseeffect.h \
    effects/internal/paneffect.h \
    effects/internal/toneeffect.h \
    effects/internal/volumeeffect.h \
    effects/internal/shakeeffect.h \
    effects/internal/linearfadetransition.h \
    effects/internal/crossdissolvetransition.h \
    effects/internal/exponentialfadetransition.h \
    effects/internal/logarithmicfadetransition.h \
    effects/internal/cornerpineffect.h \
    io/math.h \
    io/qpainterwrapper.h \
    project/effect.h \
    project/transition.h \
    project/effectrow.h \
    project/effectfield.h \
    effects/internal/cubetransition.h \
    project/effectgizmo.h \
    io/clipboard.h \
    dialogs/stabilizerdialog.h \
    io/avtogl.h \
    ui/resizablescrollbar.h \
    ui/sourceiconview.h \
    project/sourcescommon.h \
    ui/keyframenavigator.h \
    panels/grapheditor.h \
    ui/graphview.h \
    ui/keyframedrawing.h \
    ui/clickablelabel.h \
    project/keyframe.h \
    ui/rectangleselect.h \
    dialogs/actionsearch.h \
    ui/embeddedfilechooser.h \
    effects/internal/fillleftrighteffect.h \
    effects/internal/voideffect.h \
    ui/defaultshortcut.h \
    project/projectitem.h \
    dialogs/texteditdialog.h \
    dialogs/debugdialog.h \
    ui/renderthread.h \
    ui/renderfunctions.h \
    ui/viewerwindow.h \
    project/projectfilter.h

DISTFILES +=


