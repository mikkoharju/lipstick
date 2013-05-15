INCLUDEPATH += $$PWD

PUBLICHEADERS += \
    $$PWD/lipstickcompositor.h \

HEADERS += \
    $$PWD/switchermodel.h \
    $$PWD/switcherpixmapitem.h \
    $$PWD/windowproperty.h \

SOURCES += \
    $$PWD/lipstickcompositor.cpp \
    $$PWD/switchermodel.cpp \
    $$PWD/switcherpixmapitem.cpp \
    $$PWD/windowproperty.cpp \

DEFINES += QT_COMPOSITOR_QUICK

QT += compositor
