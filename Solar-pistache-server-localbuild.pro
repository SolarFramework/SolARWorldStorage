## global defintions : target lib name, version
TARGET = SolARService_WorldStorage
VERSION = 0.11.0

QMAKE_PROJECT_DEPTH = 0

## remove Qt dependencies
QT     -= core gui
CONFIG -= qt
CONFIG += c++1z
CONFIG += console
CONFIG += verbose

DEFINES += MYVERSION=$${VERSION}

include(findremakenrules.pri)

LIBS += -L"/usr/local" -l"PistacheGen"

# Default rules for deployment.
qnx: target.path = $${PWD}/bin/Debug# /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += libpistache

DEPENDENCIESCONFIG = sharedlib install_recurse

#NOTE : CONFIG as staticlib or sharedlib, DEPENDENCIESCONFIG as staticlib or sharedlib, QMAKE_TARGET.arch and PROJECTDEPLOYDIR MUST BE DEFINED BEFORE templatelibconfig.pri inclusion
include ($$shell_quote($$shell_path($${QMAKE_REMAKEN_RULES_ROOT}/templateappconfig.pri)))  # Shell_quote & shell_path required for visual on windows

INCLUDEPATH += \
    Solar-Wrapper/ \
    Solar-Wrapper/interfaces/

HEADERS += \
    Solar-Wrapper/interfaces/DefaultSolARImpl.h \
    Solar-Wrapper/interfaces/TrackablesSolARImpl.h \
    Solar-Wrapper/interfaces/UnitSysConversion.h \
    Solar-Wrapper/interfaces/WorldAnchorsSolARImpl.h \
    Solar-Wrapper/interfaces/WorldLinksSolARImpl.h

SOURCES += \
    Solar-Wrapper/src/DefaultSolARImpl.cpp \
    Solar-Wrapper/src/TrackablesSolARImpl.cpp \
    Solar-Wrapper/src/WorldAnchorsSolARImpl.cpp \
    Solar-Wrapper/src/WorldLinksSolARImpl.cpp \
    Solar-Wrapper/src/main.cpp

DISTFILES += \
    build/SolARSample_World_Storage_conf.xml \
    packagedependencies.txt

config_files.path = target.path
config_files.files= $$files($${PWD}/SolARSample_World_Storage_conf.xml)
INSTALLS += config_files


#NOTE : Must be placed at the end of the .pro
include ($$shell_quote($$shell_path($${QMAKE_REMAKEN_RULES_ROOT}/remaken_install_target.pri)))) # Shell_quote & shell_path required for visual on windows
