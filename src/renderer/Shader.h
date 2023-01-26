#pragma once

#include <SDL_opengl.h>
#include <glm/detail/type_mat4x4.hpp>
#include <vector>


class Shader {
public:
    void init();

    void bind();

    void unbind();

    void setMatrix(const char* uniform_name, glm::mat<4, 4, float> matrix);

    void setIntArray(const char* uniform_name, const std::vector<int>& array);

private:
    static GLuint load_shader(const char* file_name, GLenum shader_type);

private:
    GLuint program_id = 0;
};
