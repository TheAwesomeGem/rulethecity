#pragma once

#include <SDL_opengl.h>
#include <glm/detail/type_mat4x4.hpp>
#include <vector>
#include <unordered_map>


static constexpr const size_t MAX_TEXTURES = 8;

class ShaderProgram {
public:
    ShaderProgram() : program_id { 0 } {

    }

    void init(const char* vertex_shader_file, const char* fragment_shader_file);

    void bind() const;

    void unbind() const;

    void setMatrix(const char* uniform_name, glm::mat<4, 4, float> matrix) const;

    void setIntArray(const char* uniform_name, const std::vector<int>& array) const;

private:
    static GLuint load_shader(const char* file_name, GLenum gl_shader_type);

    void init_texture_slots() const;

private:
    GLuint program_id;
};
