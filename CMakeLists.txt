cmake_minimum_required(VERSION 3.16)

project(SmartWeather VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets PrintSupport Sql)

qt_add_executable(SmartWeather
    main.cpp

    connection.cpp
    connection.h

    bureau.cpp
    bureau.h
    gbureaux.cpp
    gbureaux.h
    gbureaux.ui

    equipement.cpp
    equipement.h
    weatherwisetest.cpp
    weatherwisetest.h
    weatherwisetest.ui

    technicien.cpp
    technicien.h
    technicienne.cpp
    technicienne.h

    gutilisateur.cpp
    gutilisateur.h
    gutilisateur.ui

    stationwindow.cpp
    stationwuindow.h
)

target_link_libraries(SmartWeather PRIVATE
    Qt6::Widgets
    Qt6::PrintSupport
    Qt6::Sql
)
