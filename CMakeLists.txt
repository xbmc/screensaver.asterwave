project(screensaver.asterwave)

cmake_minimum_required(VERSION 3.3)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

enable_language(CXX)

find_package(Kodi REQUIRED)
find_package(glm REQUIRED)

# set(APP_RENDER_SYSTEM "gles") # Leaved here for test purpose only

if(APP_RENDER_SYSTEM STREQUAL "gl" OR NOT APP_RENDER_SYSTEM)
  find_package(OpenGl REQUIRED)
  set(DEPLIBS ${OPENGL_LIBRARIES})
  set(includes ${OPENGL_INCLUDE_DIR})
  add_definitions(${OPENGL_DEFINITIONS})
elseif(APP_RENDER_SYSTEM STREQUAL "gles")
  find_package(OpenGLES REQUIRED)
  set(DEPLIBS ${OPENGLES_LIBRARIES})
  set(includes ${OPENGLES_INCLUDE_DIR})
  add_definitions(${OPENGLES_DEFINITIONS})
endif()

include_directories(${KODI_INCLUDE_DIR}/..) # Hack way with "/..", need bigger Kodi cmake rework to match right include ways

if(NOT ${CORE_SYSTEM_NAME} STREQUAL "")
  if(${CORE_SYSTEM_NAME} STREQUAL "osx" OR ${CORE_SYSTEM_NAME} STREQUAL "ios")
    list(APPEND DEPLIBS "-framework CoreVideo")
  endif()
endif()

add_subdirectory(lib/SOIL2)

set(ASTERWAVE_SOURCES src/Effect.cpp
                      src/Util.cpp
                      src/Water.cpp
                      src/waterfield.cpp)

set(ASTERWAVE_HEADERS src/Effect.h
                      src/types.h
                      src/Util.h
                      src/waterfield.h
                      src/Water.h)

list(APPEND DEPLIBS soil2)
list(APPEND DEPENDS glm)

build_addon(screensaver.asterwave ASTERWAVE DEPLIBS)

if(NOT GLM_FOUND)
  add_dependencies(screensaver.asterwave glm)
endif()

target_include_directories(screensaver.asterwave PUBLIC ${GLM_INCLUDE_DIR})

include(CPack)
