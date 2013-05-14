INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/lipstickcompositor.h \
    $$PWD/switchermodel.h \
    $$PWD/switcherpixmapitem.h \

SOURCES += \
    $$PWD/lipstickcompositor.cpp \
    $$PWD/switchermodel.cpp \
    $$PWD/switcherpixmapitem.cpp \

DEFINES += QT_COMPOSITOR_QUICK

QT += compositor
