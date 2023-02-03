#pragma once

#include <SDL_opengl.h>
#include <glm/detail/type_mat4x4.hpp>
#include <vector>
#include <unordered_map>


enum class ShaderType {
    FILLED_QUAD
};

struct ShaderInfo {
    ShaderType type;
    std::unordered_map<const char*, float> floats_params;
};

class Shader {
public:
    Shader() : program_id { 0 } {

    }

    void init();

    void bind() const;

    void unbind() const;

    void setMatrix(const char* uniform_name, glm::mat<4, 4, float> matrix) const;

    void setIntArray(const char* uniform_name, const std::vector<int>& array) const;

private:
    static GLuint load_shader(const char* file_name, GLenum gl_shader_type);

private:
    GLuint program_id;
};
