cmake_minimum_required(VERSION 3.24)
project(rulethecity)

set(CMAKE_CXX_STANDARD 20)


# Use these commands in vcpkg to add these libraries
# vcpkg install sdl2
# vcpkg install sdl2-image
# vcpkg install glad[gl-api-45,extensions]
# vcpkg install glm
find_package(SDL2 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Stb REQUIRED)
# =========

add_executable(rulethecity src/main.cpp src/renderer/ShaderProgram.cpp src/renderer/ShaderProgram.h src/renderer/stb_image.cpp src/renderer/Texture.cpp src/renderer/Texture.h src/renderer/Screen.h src/renderer/Renderer.cpp src/renderer/Renderer.h src/renderer/ShapeGenerator.h src/renderer/Shape.cpp src/renderer/Shape.h src/renderer/RenderBatch.cpp src/renderer/RenderBatch.h)
target_link_libraries(rulethecity
        PRIVATE
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>)
target_include_directories(rulethecity PRIVATE ${Stb_INCLUDE_DIR})
target_link_libraries(rulethecity PRIVATE glad::glad)
target_link_libraries(rulethecity PRIVATE glm::glm)
