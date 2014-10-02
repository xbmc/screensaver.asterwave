project(screensaver.asterwave)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

enable_language(CXX)

find_package(xbmc REQUIRED)
find_package(SOIL REQUIRED)

include_directories(${XBMC_INCLUDE_DIR}
                    ${SOIL_INCLUDE_DIRS})

set(ASTERWAVE_SOURCES src/Effect.cpp
                      src/Util.cpp
                      src/Water.cpp
                      src/waterfield.cpp)

set(DEPLIBS ${SOIL_LIBRARIES})

build_addon(screensaver.asterwave ASTERWAVE DEPLIBS)

include(CPack)
