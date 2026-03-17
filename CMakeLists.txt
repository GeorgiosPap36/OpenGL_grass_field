cmake_minimum_required(VERSION 3.26)
project(OpenGL_grass_field VERSION 4.3)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0072 NEW)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

set(SOURCES
    src/main.cpp
    src/model/mesh.cpp
    src/model/model.cpp
    src/model/material.cpp
    src/shaders/shader.cpp
    src/shaders/compute_shader.cpp
    src/camera/camera.cpp
    src/scene/scene.cpp

    src/camera/basic_camera_controller.cpp
    src/scene/grass_field_scene.cpp

    dependencies/glad/glad.c)

add_executable(OpenGL_grass_field ${SOURCES})

if(WIN32)
    set(ASSIMP_ROOT "${CMAKE_SOURCE_DIR}/dependencies/assimp")
    target_include_directories(OpenGL_grass_field PRIVATE
        ${ASSIMP_ROOT}/include
    )
    target_link_libraries(OpenGL_grass_field PRIVATE
        ${ASSIMP_ROOT}/lib/libassimp.dll.a
    )
    # Copy dll to build dir
    add_custom_command(TARGET OpenGL_grass_field POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${ASSIMP_ROOT}/bin/libassimp-6.dll
        $<TARGET_FILE_DIR:OpenGL_grass_field>
    )
else()
    find_package(assimp REQUIRED)
    target_link_libraries(OpenGL_grass_field PRIVATE 
        assimp::assimp)
endif()

target_include_directories(OpenGL_grass_field PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glad)

target_link_libraries(OpenGL_grass_field PRIVATE
    glfw
    glm::glm
    OpenGL::GL)

target_compile_definitions(OpenGL_grass_field PRIVATE _CRT_SECURE_NO_WARNINGS)