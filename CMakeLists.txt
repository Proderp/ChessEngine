cmake_minimum_required(VERSION 3.16)
project(ChessEngine LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compile the fathom files into library
add_library(fathom STATIC 
    vendor/fathom/tbprobe.c
    vendor/fathom/tbconfig.h
    vendor/fathom/tbcore.h
    vendor/fathom/tbprobe.h
)
target_include_directories(fathom PUBLIC vendor/fathom)

find_package(SFML 3 COMPONENTS Graphics Window System Main REQUIRED)

set(
    SOURCE_FILES
    src/app.cpp
    src/board.cpp
    src/game-elements.cpp
    src/game.cpp
    src/render.cpp
)

add_library(chess-library ${SOURCE_FILES})
target_include_directories(chess-library PUBLIC includes/)
target_link_libraries(chess-library PUBLIC SFML::Graphics SFML::Window SFML::System fathom)

if(WIN32)
    set(WINDOWS_RESOURCES assets/window/Resource.rc)
endif()

add_executable(ChessEngine WIN32 src/main.cpp ${WINDOWS_RESOURCES})
target_link_libraries(ChessEngine PRIVATE chess-library SFML::Main)

add_custom_command(TARGET ChessEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:ChessEngine>/assets"
    
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/data"
    "$<TARGET_FILE_DIR:ChessEngine>/data"
)