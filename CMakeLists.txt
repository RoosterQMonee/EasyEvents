cmake_minimum_required(VERSION 3.14)
project(EasyEventsDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    main.cpp
)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(EasyEventsDemo ${SOURCES})
