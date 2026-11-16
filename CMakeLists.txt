cmake_minimum_required(VERSION 3.16)

project(SmartWeather VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets PrintSupport Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets PrintSupport Sql)

set(PROJECT_SOURCES
    main.cpp
    connection.cpp
    connection.h
    equipement.cpp
    equipement.h
    gbureaux.cpp
    gbureaux.h
    gbureaux.ui
    gutilisateur.cpp
    gutilisateur.h
    gutilisateur.ui
    stationwindow.cpp
    stationwuindow.h
    technicien.cpp
    technicien.h
    technicienne.cpp
    technicienne.h
    weatherwisetest.cpp
    weatherwisetest.h
    weatherwisetest.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SmartWeather
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(SmartWeather
        ${PROJECT_SOURCES}
    )
endif()

target_link_libraries(SmartWeather PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Sql






)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SmartWeather)
endif()