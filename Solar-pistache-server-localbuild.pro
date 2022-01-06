QT -= gui
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += libpistache

INCLUDEPATH += \
    Solar-Wrapper
    Solar-Wrapper/api
    Solar-Wrapper/model
    Solar-Wrapper/impl

HEADERS += \
    Solar-Wrapper/api/TrackablesApi.h \
    Solar-Wrapper/impl/TrackablesApiImpl.h \
    Solar-Wrapper/model/Error.h \
    Solar-Wrapper/model/Helpers.h \
    Solar-Wrapper/model/Trackable.h \
    Solar-Wrapper/model/TrackableEncodingInformationStructure.h \
    Solar-Wrapper/model/Transform3d.h \
    Solar-Wrapper/model/nlohmann/json.hpp

SOURCES += \
    Solar-Wrapper/api/TrackablesApi.cpp \
    Solar-Wrapper/impl/TrackablesApiImpl.cpp \
    Solar-Wrapper/main.cpp \
    Solar-Wrapper/model/Error.cpp \
    Solar-Wrapper/model/Helpers.cpp \
    Solar-Wrapper/model/Trackable.cpp \
    Solar-Wrapper/model/TrackableEncodingInformationStructure.cpp \
    Solar-Wrapper/model/Transform3d.cpp
