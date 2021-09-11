
cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(QT_CMAKE_DIR "" CACHE PATH "QT cmake dir")

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT_CMAKE_DIR})

# find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt6 COMPONENTS Widgets REQUIRED)

add_executable(ZedUI
    MainWindow.ui
    MainWindow.cpp
    Main.cpp
    # resources.qrc
)

# target_link_libraries(ZedUI Qt5::Widgets Zed)
target_link_libraries(ZedUI Qt6::Widgets Zed)

# TEMP
set(QT_BASE "D:\\Frameworks\\Qt3\\6.1.3\\msvc2019_64\\bin")

set(MY_PATH "PATH=${QT_BASE}\\bin;${QT_BASE}\\plugins\\platforms;%PATH%")
set_target_properties(ZedUI PROPERTIES VS_DEBUGGER_ENVIRONMENT "${MY_PATH}")
