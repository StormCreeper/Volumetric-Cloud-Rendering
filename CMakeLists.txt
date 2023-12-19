# ----------------------------------------------------------------------------
# CMakeLists.txt
#
#  Created on: 13 Dec 2020
#      Author: Kiwon Um
#        Mail: kiwon.um@telecom-paris.fr
#
# Description: CMake configuration
#
# Copyright 2021-2023 Kiwon Um
#
# The copyright to the computer program(s) herein is the property of Kiwon Um,
# Telecom Paris, France. The program(s) may be used and/or copied only with
# the written permission of Kiwon Um or in accordance with the terms and
# conditions stipulated in the agreement/contract under which the program(s)
# have been supplied.
# ----------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.5)

SET(CMAKE_EXPORT_COMPILE_COMMANDS 1)
SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED True)
# add_compile_definitions(_MY_OPENGL_IS_33_)

project(Fur_Rendering)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(SOURCES
  main.cpp
  mesh.cpp
  shader.cpp
  object3d.cpp

  camera.hpp
  mesh.hpp
  shader.hpp
  gl_includes.hpp
  object3d.hpp
  framebuffer.hpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_sources(${PROJECT_NAME} PRIVATE dep/glad/src/gl.c)
target_include_directories(${PROJECT_NAME} PRIVATE dep/glad/include/)

add_subdirectory(dep/glfw)
target_link_libraries(${PROJECT_NAME} glfw)

add_subdirectory(dep/glm)
target_link_libraries(${PROJECT_NAME} glm)

add_subdirectory(dep/imgui)
target_link_libraries(${PROJECT_NAME} IMGUI)

target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS})

# Create a custom target to copy resources to the build directory (Added by Telo PHILIPPE)
add_custom_target(CopyResources
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources ${CMAKE_BINARY_DIR}/resources
)