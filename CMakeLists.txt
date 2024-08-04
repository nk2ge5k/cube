cmake_minimum_required(VERSION 3.29)
project(cube LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(raylib REQUIRED)

set(SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/src")

set(SOURCES
  "${SOURCE_DIR}/debug.c"
  "${SOURCE_DIR}/main.c"
  "${SOURCE_DIR}/types.c"
)

add_executable(cube ${SOURCES})

target_include_directories(cube
  PRIVATE
    ${SOURCE_DIR}
)

target_link_libraries(cube
  PRIVATE
    m
    raylib
)
