
VERSION = "0.5.16"
RELEASE = "-RC7"

TEMPLATE = app

TARGET = simulide

QT += svg
QT += xml
QT += script
QT += widgets
QT += concurrent
QT += serialport
QT += multimedia widgets

SOURCES += ../src/*.cpp \
    ../src/gui/*.cpp \
    ../src/gui/circuitwidget/*.cpp \
    ../src/gui/circuitwidget/components/*.cpp \
    ../src/gui/circuitwidget/components/active/*.cpp \
    ../src/gui/circuitwidget/components/logic/*.cpp \
    ../src/gui/circuitwidget/components/mcu/*.cpp \
    ../src/gui/circuitwidget/components/meters/*.cpp \
    ../src/gui/circuitwidget/components/other/*.cpp \
    ../src/gui/circuitwidget/components/outputs/*.cpp \
    ../src/gui/circuitwidget/components/outputs/displays/*.cpp \
    ../src/gui/circuitwidget/components/passive/*.cpp \
    ../src/gui/circuitwidget/components/sources/*.cpp \
    ../src/gui/circuitwidget/components/switches/*.cpp \
    ../src/gui/dataplotwidget/*.cpp \
    ../src/gui/serial/*.cpp \
    ../src/gui/componentselector/*.cpp \
    ../src/gui/filebrowser/*.cpp \
    ../src/gui/editorwidget/*.cpp \
    ../src/gui/editorwidget/findreplacedialog/*.cpp \
    ../src/gui/dialogs/*.cpp \
    ../src/gui/dialogs/properties/*.cpp \
    ../src/gui/memory/*.cpp \
    ../src/simulator/*.cpp \
    ../src/simulator/elements/*.cpp \
    ../src/simulator/elements/active/*.cpp \
    ../src/simulator/elements/outputs/*.cpp \
    ../src/simulator/elements/passive/*.cpp \
    ../src/simulator/elements/processors/*.cpp \
    ../src/gpsim/*.cc \
    ../src/gpsim/devices/*.cc \
    ../src/gpsim/modules/*.cc \
    ../src/gpsim/registers/*.cc \
    ../src/mcusim/*.cpp \
    ../src/mcusim/cores/*.cpp \
    ../src/mcusim/cores/avr/*.cpp \
    ../src/mcusim/cores/i51/*.cpp \
    ../src/mcusim/cores/pic/*.cpp \
    ../src/mcusim/modules/usart/*.cpp \
    ../src/mcusim/modules/twi/*.cpp \
    ../src/mcusim/modules/spi/*.cpp

HEADERS += ../src/*.h \
    ../src/gui/*.h \
    ../src/gui/circuitwidget/*.h \
    ../src/gui/circuitwidget/components/*.h \
    ../src/gui/circuitwidget/components/active/*.h \
    ../src/gui/circuitwidget/components/logic/*.h \
    ../src/gui/circuitwidget/components/mcu/*.h \
    ../src/gui/circuitwidget/components/meters/*.h \
    ../src/gui/circuitwidget/components/other/*.h \
    ../src/gui/circuitwidget/components/outputs/*.h \
    ../src/gui/circuitwidget/components/outputs/displays/*.h \
    ../src/gui/circuitwidget/components/passive/*.h \
    ../src/gui/circuitwidget/components/sources/*.h \
    ../src/gui/circuitwidget/components/switches/*.h \
    ../src/gui/dataplotwidget/*.h \
    ../src/gui/serial/*.h \
    ../src/gui/componentselector/*.h \
    ../src/gui/filebrowser/*.h \
    ../src/gui/editorwidget/*.h \
    ../src/gui/editorwidget/findreplacedialog/*.h \
    ../src/gui/dialogs/*.h \
    ../src/gui/dialogs/properties/*.h \
    ../src/gui/memory/*.h \
    ../src/simulator/*.h \
    ../src/simulator/elements/*.h \
    ../src/simulator/elements/active/*.h \
    ../src/simulator/elements/outputs/*.h \
    ../src/simulator/elements/passive/*.h \
    ../src/simulator/elements/processors/*.h \
    ../resources/data/*.xml \
    ../src/gpsim/*.h \
    ../src/gpsim/devices/*.h \
    ../src/gpsim/modules/*.h \
    ../src/gpsim/registers/*.h \
    ../src/mcusim/*.h \
    ../src/mcusim/cores/*.h \
    ../src/mcusim/cores/avr/*.h \
    ../src/mcusim/cores/i51/*.h \
    ../src/mcusim/cores/pic/*.h \
    ../src/mcusim/modules/usart/*.h \
    ../src/mcusim/modules/twi/*.h \
    ../src/mcusim/modules/spi/*.h

INCLUDEPATH += ../src \
    ../src/gui \
    ../src/gui/circuitwidget \
    ../src/gui/circuitwidget/components \
    ../src/gui/circuitwidget/components/active \
    ../src/gui/circuitwidget/components/logic \
    ../src/gui/circuitwidget/components/mcu \
    ../src/gui/circuitwidget/components/meters \
    ../src/gui/circuitwidget/components/other \
    ../src/gui/circuitwidget/components/outputs \
    ../src/gui/circuitwidget/components/outputs/displays \
    ../src/gui/circuitwidget/components/passive \
    ../src/gui/circuitwidget/components/sources \
    ../src/gui/circuitwidget/components/switches \
    ../src/gui/dataplotwidget \
    ../src/gui/serial \
    ../src/gui/componentselector \
    ../src/gui/filebrowser \
    ../src/gui/editorwidget \
    ../src/gui/editorwidget/findreplacedialog \
    ../src/gui/dialogs \
    ../src/gui/dialogs/properties \
    ../src/gui/memory \
    ../src/simulator \
    ../src/simulator/elements \
    ../src/simulator/elements/active \
    ../src/simulator/elements/outputs \
    ../src/simulator/elements/passive \
    ../src/simulator/elements/processors \
    ../src/gpsim \
    ../src/gpsim/devices \
    ../src/gpsim/modules \
    ../src/gpsim/registers \
    ../src/mcusim \
    ../src/mcusim/cores \
    ../src/mcusim/cores/avr \
    ../src/mcusim/cores/i51 \
    ../src/mcusim/cores/pic \
    ../src/mcusim/modules \
    ../src/mcusim/modules/usart \
    ../src/mcusim/modules/twi \
    ../src/mcusim/modules/spi

TRANSLATIONS +=  \
    ../resources/translations/simulide.ts \
    ../resources/translations/simulide_cz.ts \
    ../resources/translations/simulide_de.ts \
    ../resources/translations/simulide_en.ts \
    ../resources/translations/simulide_es.ts \
    ../resources/translations/simulide_fr.ts \
    ../resources/translations/simulide_pt_BR.ts \
    ../resources/translations/simulide_ru.ts \
    ../resources/translations/simulide_nl.ts \
    ../resources/translations/simulide_it.ts \
    ../resources/translations/simulide_tr.ts

FORMS +=   \
    ../src/gui/dialogs/*.ui \
    ../src/gui/dialogs/properties/*.ui \
    ../src/gui/memory/*.ui \
    ../src/gui/dataplotwidget/*.ui \
    ../src/gui/serial/serialmon.ui

RESOURCES = ../src/application.qrc

QMAKE_CXXFLAGS += -Wno-unused-parameter
QMAKE_CXXFLAGS += -Wno-missing-field-initializers
QMAKE_CXXFLAGS += -Wno-implicit-fallthrough
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
    RC_ICONS += ../src/icons/simulide.ico
}
linux {
    OS = Linux
    QMAKE_LFLAGS += -no-pie
}
macx {
    OS = MacOs
    QMAKE_LFLAGS += -no-pie
    ICON = ../src/icons/simulide.icns
}

CONFIG += qt 
CONFIG += warn_on
CONFIG += no_qml_debug
CONFIG *= c++11

BUILD_DATE = $$system(date +\"\\\"%d-%m-%y\\\"\")

DEFINES += MAINMODULE_EXPORT=
DEFINES += APP_VERSION=\\\"$$VERSION$$RELEASE\\\"
DEFINES += BUILDDATE=\\\"$$BUILD_DATE\\\"

TARGET_NAME   = SimulIDE_$$VERSION$$RELEASE$$
TARGET_PREFIX = $$BUILD_DIR/executables/$$TARGET_NAME

OBJECTS_DIR *= $$OUT_PWD/build/objects
MOC_DIR     *= $$OUT_PWD/build/moc
INCLUDEPATH += $$MOC_DIR

runLrelease.commands = lrelease ../resources/translations/*.ts; 
QMAKE_EXTRA_TARGETS += runLrelease
POST_TARGETDEPS     += runLrelease

win32 | linux {
    DESTDIR = $$TARGET_PREFIX/bin 
    mkpath( $$TARGET_PREFIX/bin )
    copy2dest.commands = \
        $(MKDIR)    $$TARGET_PREFIX/share/simulide/data ; \
        $(MKDIR)    $$TARGET_PREFIX/share/simulide/examples ; \
        $(MKDIR)    $$TARGET_PREFIX/share/simulide/translations ; \
        $(COPY_DIR) ../resources/data              $$TARGET_PREFIX/share/simulide ; \
        $(COPY_DIR) ../resources/examples          $$TARGET_PREFIX/share/simulide ; \
        $(COPY_DIR) ../resources/icons             $$TARGET_PREFIX/share ; \
        $(MOVE)     ../resources/translations/*.qm $$TARGET_PREFIX/share/simulide/translations ;
}
macx {
#    QMAKE_CC = gcc-10
#    QMAKE_CXX = g++-10
#    QMAKE_LINK = g++-10
    QMAKE_CXXFLAGS -= -stdlib=libc++
    QMAKE_LFLAGS -= -stdlib=libc++
    DESTDIR = $$TARGET_PREFIX 
    mkpath( $$TARGET_PREFIX/simulide.app )
    copy2dest.commands = \
        $(MKDIR)    $$TARGET_PREFIX/simulide.app/Contents/share/simulide/data ; \
        $(MKDIR)    $$TARGET_PREFIX/simulide.app/Contents/share/simulide/examples ; \
        $(MKDIR)    $$TARGET_PREFIX/simulide.app/Contents/share/simulide/translations ; \
        $(COPY_DIR) ../resources/data              $$TARGET_PREFIX/simulide.app/Contents/share/simulide ; \
        $(COPY_DIR) ../resources/examples          $$TARGET_PREFIX/simulide.app/Contents/share/simulide ; \
        $(COPY_DIR) ../resources/icons             $$TARGET_PREFIX/simulide.app/Contents/share ; \
        $(MOVE)     ../resources/translations/*.qm $$TARGET_PREFIX/simulide.app/Contents/share/simulide/translations ;
}

QMAKE_EXTRA_TARGETS += copy2dest
POST_TARGETDEPS     += copy2dest


message( "-----------------------------------")
message( "    "                               )
message( "    "$$TARGET_NAME for $$OS         )
message( "    "                               )
message( "          Qt version: "$$QT_VERSION )
message( "    "                               )
message( "    Destination Folder:"            )
message( $$TARGET_PREFIX                      )
message( "-----------------------------------")

