cmake_minimum_required(VERSION 3.20)

project(VoronaCropper)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "C:/Qt/6.11.1/mingw_64")

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(VoronaCropper
        src/main.cpp
        src/MainWindow.cpp
        src/MainWindow.h
        src/ImageCanvas.cpp
        src/ImageCanvas.h
        src/ImageItem.cpp
        src/ImageItem.h
        src/CropState.cpp
        src/CropState.h
        src/LightState.cpp
        src/LightState.h
        src/CropCanvas.cpp
        src/CropCanvas.h
)

target_link_libraries(VoronaCropper PRIVATE
        Qt6::Widgets
)

file(COPY
        "${CMAKE_SOURCE_DIR}/light"
        DESTINATION "${CMAKE_BINARY_DIR}"
)
