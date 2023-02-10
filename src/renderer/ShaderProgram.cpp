#include <glad/glad.h>
#include <cstdio>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include "ShaderProgram.h"


void ShaderProgram::init(const char* vertex_shader_file, const char* fragment_shader_file) {
    GLuint vertex_shader = load_shader(vertex_shader_file, GL_VERTEX_SHADER);

    if (vertex_shader == 0) {
        return;
    }

    GLuint fragment_shader = load_shader(fragment_shader_file, GL_FRAGMENT_SHADER);

    if (fragment_shader == 0) {
        return;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint program_linked;
    glGetProgramiv(program, GL_LINK_STATUS, &program_linked);
    if (program_linked != GL_TRUE) {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetProgramInfoLog(program, 1024, &log_length, message);
        printf("Error: Cannot link shaders to program: %s", message);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        return;
    }

    glValidateProgram(program);

    GLint program_validated;
    glGetProgramiv(program, GL_VALIDATE_STATUS, &program_validated);
    if (program_validated != GL_TRUE) {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetProgramInfoLog(program, 1024, &log_length, message);
        printf("Error: Cannot validate shaders to program: %s", message);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        return;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    printf("Loaded Main Program.\n");

    this->program_id = program;

    // FUTURE TODO: Maybe only certain shader needs this? Should we move this outside of here?
    init_texture_slots();
}

void ShaderProgram::bind() const {
    glUseProgram(program_id);
}

void ShaderProgram::unbind() const {
    glUseProgram(0);
}

void ShaderProgram::setMatrix(const char* uniform_name, glm::mat<4, 4, float> matrix) const {
    glUniformMatrix4fv(glGetUniformLocation(this->program_id, uniform_name), 1, GL_FALSE, glm::value_ptr(matrix));
}

void ShaderProgram::setIntArray(const char* uniform_name, const std::vector<int>& array) const {
    glUniform1iv(glGetUniformLocation(this->program_id, uniform_name), array.size(), array.data());
}

GLuint ShaderProgram::load_shader(const char* file_name, GLenum gl_shader_type) {
    std::ifstream shader_file{file_name};

    if (!shader_file.good()) {
        printf("Failed to open %s\n", file_name);

        return 0;
    }

    std::stringstream shader_buffer;
    shader_buffer << shader_file.rdbuf();
    shader_file.close();

    std::string shader_code = shader_buffer.str();

    GLuint shader_id;
    shader_id = glCreateShader(gl_shader_type);
    const char* shader_src = shader_code.c_str();
    glShaderSource(shader_id, 1, &shader_src, nullptr);
    glCompileShader(shader_id);

    GLint shader_compiled;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &shader_compiled);
    if (shader_compiled != GL_TRUE) {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetShaderInfoLog(shader_id, 1024, &log_length, message);
        printf("Error: Cannot compile %s shader: %s", file_name, message);

        return 0;
    }

    return shader_id;
}

void ShaderProgram::init_texture_slots() const {
    bind();
    std::vector<int> textures{};
    textures.reserve(MAX_TEXTURES);

    for (int i = 0; i < MAX_TEXTURES + 1; ++i) {
        textures.push_back(i);
    }

    setIntArray("u_textures", textures);
    unbind();
}
