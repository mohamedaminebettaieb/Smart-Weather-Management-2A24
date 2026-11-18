QT       += core gui widgets printsupport sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG   += c++17
TARGET    = SmartWeather
TEMPLATE  = app

# Qt 6 recommandé (QLocale::Territory, QTimeZone::territory)

SOURCES += \
    main.cpp \
    connection.cpp \
    bureau.cpp \
    gbureaux.cpp \
    equipement.cpp \
    weatherwisetest.cpp \
    technicien.cpp \
    technicienne.cpp \
    gutilisateur.cpp \
    stationwindow.cpp

HEADERS += \
    connection.h \
    bureau.h \
    gbureaux.h \
    equipement.h \
    weatherwisetest.h \
    technicien.h \
    technicienne.h \
    gutilisateur.h \
    stationwuindow.h

FORMS += \
    gbureaux.ui \
    weatherwisetest.ui \
    gutilisateur.ui

# Déploiement par défaut
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
