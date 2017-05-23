project(screensaver.asterwave)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

enable_language(CXX)

find_package(Kodi REQUIRED)
find_package(SOIL REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(${KODI_INCLUDE_DIR}/.. # Hack way with "/..", need bigger Kodi cmake rework to match right include ways
                    ${SOIL_INCLUDE_DIRS}
                    ${OPENGL_INCLUDE_DIRS})

set(ASTERWAVE_SOURCES src/Effect.cpp
                      src/Util.cpp
                      src/Water.cpp
                      src/waterfield.cpp)

set(ASTERWAVE_HEADERS src/Effect.h
                      src/timer.h
                      src/types.h
                      src/Util.h
                      src/waterfield.h
                      src/Water.h)

set(DEPLIBS ${SOIL_LIBRARIES})
list(APPEND DEPLIBS ${OPENGL_LIBRARIES})

build_addon(screensaver.asterwave ASTERWAVE DEPLIBS)

include(CPack)
