cmake_minimum_required(VERSION 3.16)

project(FinanceTracker VERSION 2.1.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS include/*.h)

add_executable(${PROJECT_NAME} WIN32
    ${SOURCES}
    ${HEADERS}
    resources/app.qrc
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Widgets
    Qt6::Sql
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/style.qss
    ${CMAKE_CURRENT_BINARY_DIR}/style.qss
    COPYONLY
)

if (WIN32)
    target_sources(${PROJECT_NAME} PRIVATE app.rc)
endif()
