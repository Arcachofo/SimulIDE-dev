
VERSION = "1.2.0"
RELEASE = ""

TEMPLATE = app
TARGET = simulide

QT += svg
QT += xml
QT += widgets
QT += concurrent
QT += serialport
QT += multimedia widgets

SOURCES      = $$files( $$PWD/src/*.cpp, true )
HEADERS      = $$files( $$PWD/src/*.h, true )
TRANSLATIONS = $$files( $$PWD/resources/translations/*.ts )
FORMS       += $$files( $$PWD/src/*.ui, true )
RESOURCES    = $$PWD/src/application.qrc

INCLUDEPATH += $$PWD/src \
    $$PWD/src/components \
    $$PWD/src/components/active \
    $$PWD/src/components/connectors \
    $$PWD/src/components/graphical \
    $$PWD/src/components/logic \
    $$PWD/src/components/meters \
    $$PWD/src/components/micro \
    $$PWD/src/components/other \
    $$PWD/src/components/other/truthtable \
    $$PWD/src/components/outputs \
    $$PWD/src/components/outputs/displays \
    $$PWD/src/components/outputs/leds \
    $$PWD/src/components/outputs/motors \
    $$PWD/src/components/passive \
    $$PWD/src/components/passive/reactive \
    $$PWD/src/components/passive/resistors \
    $$PWD/src/components/passive/resist_sensors \
    $$PWD/src/components/sources \
    $$PWD/src/components/subcircuits \
    $$PWD/src/components/switches \
    $$PWD/src/gui \
    $$PWD/src/gui/appdialogs \
    $$PWD/src/gui/circuitwidget \
    $$PWD/src/gui/componentlist \
    $$PWD/src/gui/dataplotwidget \
    $$PWD/src/gui/editorwidget \
    $$PWD/src/gui/editorwidget/debuggers \
    $$PWD/src/gui/editorwidget/dialogs \
    $$PWD/src/gui/filebrowser \
    $$PWD/src/gui/memory \
    $$PWD/src/gui/properties \
    $$PWD/src/gui/serial \
    $$PWD/src/simulator \
    $$PWD/src/simulator/elements \
    $$PWD/src/simulator/elements/active \
    $$PWD/src/simulator/elements/outputs \
    $$PWD/src/simulator/elements/passive \
    $$PWD/src/microsim \
    $$PWD/src/microsim/cores \
    $$PWD/src/microsim/cores/avr \
    $$PWD/src/microsim/cores/i51 \
    $$PWD/src/microsim/cores/pic \
    $$PWD/src/microsim/cores/mcs65 \
    $$PWD/src/microsim/cores/z80 \
    $$PWD/src/microsim/cores/scripted \
    $$PWD/src/microsim/cores/intmem \
    $$PWD/src/microsim/modules \
    $$PWD/src/microsim/modules/memory \
    $$PWD/src/microsim/modules/usart \
    $$PWD/src/microsim/modules/twi \
    $$PWD/src/microsim/modules/tcp\
    $$PWD/src/microsim/modules/spi\
    $$PWD/src/microsim/modules/script\
    $$PWD/src/angel/include \
    $$PWD/src/angel/JIT \
    $$PWD/src/angel/src

QMAKE_CXXFLAGS += -Wno-unused-parameter
QMAKE_CXXFLAGS += -Wno-implicit-fallthrough
QMAKE_CXXFLAGS += -fno-strict-aliasing      #AngelScript
QMAKE_CXXFLAGS += -Wno-cast-function-type   #AngelScript
QMAKE_CXXFLAGS += -Wno-deprecated-copy      #AngelScript
QMAKE_CXXFLAGS += -Wno-invalid-offsetof     #AngelScript
QMAKE_CXXFLAGS -= -fPIC
QMAKE_CXXFLAGS += -fno-pic
QMAKE_CXXFLAGS += -Ofast
QMAKE_CXXFLAGS_DEBUG -= -O
QMAKE_CXXFLAGS_DEBUG -= -O1
QMAKE_CXXFLAGS_DEBUG -= -O2
QMAKE_CXXFLAGS_DEBUG -= -O3
QMAKE_CXXFLAGS_DEBUG += -O0

win32 {
    OS = Windows
    QMAKE_LIBS += -lwsock32
    RC_ICONS += $$PWD/resources/icons/simulide.ico
}
linux {
    OS = Linux
    QMAKE_LFLAGS += -no-pie
}
macx {
    OS = MacOs
    QMAKE_LFLAGS += -no-pie
    ICON = $$PWD/resources/icons/simulide.icns
}

CONFIG += qt 
CONFIG += warn_on
CONFIG += no_qml_debug
CONFIG *= c++11

REV_NO = $$system($(which date) +\"\\\"%m%d\\\"\")
#$$system( git rev-parse --short HEAD )
DEFINES += REVNO=\\\"$$REV_NO\\\"
DEFINES += APP_VERSION=\\\"$$VERSION$$RELEASE\\\"

BUILD_DATE = $$system($(which date) +\"\\\"%d-%m-%y\\\"\")
DEFINES += BUILDDATE=\\\"$$BUILD_DATE\\\"

TARGET_NAME   = SimulIDE_$$VERSION$$RELEASE
TARGET_PREFIX = $$BUILD_DIR/executables/$$TARGET_NAME

OBJECTS_DIR *= $$OUT_PWD/build/objects
MOC_DIR     *= $$OUT_PWD/build/moc
INCLUDEPATH += $$MOC_DIR

DESTDIR = $$TARGET_PREFIX

win32 | linux {
    mkpath( $$TARGET_PREFIX/data )
    mkpath( $$TARGET_PREFIX/examples )

    copy2dest.commands = \
        $(COPY_DIR) $$PWD/resources/data     $$TARGET_PREFIX; \
        $(COPY_DIR) $$PWD/resources/examples $$TARGET_PREFIX; \
}

macx {
QMAKE_CC   = /usr/local/Cellar/gcc@7/7.5.0_4/bin/gcc-7
QMAKE_CXX  = /usr/local/Cellar/gcc@7/7.5.0_4/bin/g++-7
QMAKE_LINK = /usr/local/Cellar/gcc@7/7.5.0_4/bin/g++-7

    QMAKE_CXXFLAGS -= -stdlib=libc++
    QMAKE_LFLAGS   -= -stdlib=libc++

    mkpath( $$TARGET_PREFIX/simulide.app )
    mkpath( $$TARGET_PREFIX/simulide.app/Contents/MacOs/data )
    mkpath( $$TARGET_PREFIX/simulide.app/Contents/MacOs/examples )

    copy2dest.commands = \
        $(COPY_DIR) $$PWD/resources/data     $$TARGET_PREFIX/simulide.app/Contents/MacOs; \
        $(COPY_DIR) $$PWD/resources/examples $$TARGET_PREFIX/simulide.app/Contents/MacOs;
}

runLrelease.commands = \
    lrelease $$PWD/resources/translations/*.ts; \
    lrelease $$PWD/resources/translations/qt/*.ts; \
    $(MOVE) $$PWD/resources/translations/*.qm $$PWD/resources/qm; \
    $(MOVE) $$PWD/resources/translations/qt/*.qm $$PWD/resources/qm;

QMAKE_EXTRA_TARGETS += runLrelease
QMAKE_EXTRA_TARGETS += copy2dest
PRE_TARGETDEPS      += runLrelease
POST_TARGETDEPS     += copy2dest

message( "-----------------------------------")
message( "    "                               )
message( "    "$$TARGET_NAME for $$OS         )
message( "    "                               )
message( "    Date:      "$$BUILD_DATE        )
message( "    Qt version: "$$QT_VERSION       )
message( "    "                               )
message( "    Destination Folder:"            )
message( $$TARGET_PREFIX                      )
message( "-----------------------------------")

