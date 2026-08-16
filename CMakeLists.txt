cmake_minimum_required(VERSION 3.20)

project(VoronaCropper)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "C:/Qt/6.11.1/mingw_64")

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_resources(VORONA_RESOURCES
        resources.qrc
)

qt_add_executable(VoronaCropper
        app.rc
        ${VORONA_RESOURCES}

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

target_include_directories(VoronaCropper PRIVATE
        "C:/libs/libheif/libheif/api"
        "C:/libs/libheif-build/include"
)

target_link_directories(VoronaCropper PRIVATE
        "C:/libs/libheif-build/libheif"
)

target_link_libraries(VoronaCropper PRIVATE
        Qt6::Widgets
        heif
)

add_custom_command(TARGET VoronaCropper POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "C:/libs/libheif-build/libheif/libheif.dll"
        "$<TARGET_FILE_DIR:VoronaCropper>"
)

file(COPY
        "${CMAKE_SOURCE_DIR}/light"
        DESTINATION "${CMAKE_BINARY_DIR}"
)

file(COPY
        "${CMAKE_SOURCE_DIR}/logo.png"
        DESTINATION "${CMAKE_BINARY_DIR}"
)