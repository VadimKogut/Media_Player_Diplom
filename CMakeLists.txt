cmake_minimum_required(VERSION 3.16)

project(Z-Player VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск компонентов Qt
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Multimedia MultimediaWidgets)

# Список всех исходных файлов
set(MY_SOURCES
    main.cpp
    settingswindow.h settingswindow.cpp
    mediaplayer.h mediaplayer.cpp
    theme.h
    thememanager.h thememanager.cpp
)

# Добавляем ресурсы Windows (иконку)
if(WIN32)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources.rc")
        list(APPEND MY_SOURCES resources.rc)
    endif()
endif()

# Создание исполняемого файла (внутреннее имя цели остается прежним для линковки)
qt_add_executable(Media_Player_Diplom
    MANUAL_FINALIZATION
    ${MY_SOURCES}
)

# Линковка библиотек
target_link_libraries(Media_Player_Diplom PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::MultimediaWidgets
)

# Настройки свойств: добавляем OUTPUT_NAME "Z-Player"
set_target_properties(Media_Player_Diplom PROPERTIES
    OUTPUT_NAME "Z-Player"       # <--- ТЕПЕРЬ ФАЙЛ БУДЕТ Z-Player.exe
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(Qt6_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Media_Player_Diplom)
endif()