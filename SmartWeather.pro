QT += widgets printsupport sql

CONFIG += c++11
TEMPLATE = app
TARGET = SmartWeather

SOURCES += \
    gbureaux.cpp \
    gutilisateur.cpp \
    main.cpp \
    smartweatherwindow.cpp \
    stationwindow.cpp \
    technicienne.cpp \
    weatherwisetest.cpp \
    connection.cpp

HEADERS += \
    gbureaux.h \
    gutilisateur.h \
    smartweatherwindow.h \
    stationwindow.h \
    technicienne.h \
    weatherwisetest.h \
    connection.h
