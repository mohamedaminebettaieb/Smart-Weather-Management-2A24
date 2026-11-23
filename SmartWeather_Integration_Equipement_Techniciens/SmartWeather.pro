
QT += widgets printsupport sql

CONFIG += c++17
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
    connection.cpp \
    bureau.cpp \
    equipement.cpp \
    technicien.cpp \
    utilisateur.cpp \
    qrcodegen.cpp \
    maintenance.cpp \
    maintenancedialog.cpp \
    recrutementcv.cpp \
    cvcore.cpp \
    cvanalyzer.cpp

HEADERS += \
    gbureaux.h \
    gutilisateur.h \
    smartweatherwindow.h \
    stationwindow.h \
    technicienne.h \
    weatherwisetest.h \
    connection.h \
    bureau.h \
    equipement.h \
    technicien.h \
    utilisateur.h \
    qrcodegen.hpp \
    maintenance.h \
    maintenancedialog.h \
    recrutementcv.h \
    cvcore.h \
    cvanalyzer.h
