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

target_sources(${PROJECT_NAME} PRIVATE dep/glad/src/glad.c)
target_include_directories(${PROJECT_NAME} PRIVATE dep/glad/include/)

add_subdirectory(dep/glfw)
target_link_libraries(${PROJECT_NAME} glfw)

add_subdirectory(dep/glm)
target_link_libraries(${PROJECT_NAME} glm)

add_subdirectory(dep/imgui)
target_link_libraries(${PROJECT_NAME} IMGUI)

target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS})