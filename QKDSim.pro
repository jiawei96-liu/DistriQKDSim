QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Alg/Demand.cpp \
    Alg/KeyManager.cpp \
    Alg/Link.cpp \
    Alg/NetEvent.cpp \
    Alg/Network.cpp \
    Alg/Node.cpp \
    Alg/RelayPath.cpp \
    Alg/Route/BidBfsStrategy.cpp \
    Alg/Route/KeyRateFibStrategy.cpp \
    Alg/Route/KeyRateStrategy.cpp \
    Alg/Route/BfsStrategy.cpp \
    Alg/Route/RouteFactory.cpp \
    Alg/stdafx.cpp \
    GUI/progressbar.cpp \
    GUI/qkdsim.cpp \
    GUI/showqkdsim.cpp \
    main.cpp

HEADERS += \
    Alg/Demand.h \
    Alg/KeyManager.h \
    Alg/Link.h \
    Alg/NetEvent.h \
    Alg/Network.h \
    Alg/Node.h \
    Alg/RelayPath.h \
    Alg/Route/BidBfsStrategy.h \
    Alg/Route/FibonacciHeap.h \
    Alg/Route/KeyRateFibStrategy.h \
    Alg/Route/KeyRateStrategy.h \
    Alg/stdafx.h \
    Alg/targetver.h \
    Alg/Route/BfsStrategy.h \
    Alg/Route/RouteFactory.h \
    GUI/progressbar.h \
    GUI/qkdsim.h

FORMS += \
    GUI/qkdsim.ui

TRANSLATIONS += \
    QKDSim_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
