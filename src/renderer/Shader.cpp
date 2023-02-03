#include <glad/glad.h>
#include <cstdio>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include "Shader.h"


void Shader::init() {
    GLuint vertex_shader = load_shader("shader/filled_quad.vert", GL_VERTEX_SHADER);

    if (vertex_shader == 0) {
        return;
    }

    GLuint fragment_shader = load_shader("shader/filled_quad.frag", GL_FRAGMENT_SHADER);

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
}

void Shader::bind() const {
    glUseProgram(program_id);
}

void Shader::unbind() const {
    glUseProgram(0);
}

void Shader::setMatrix(const char* uniform_name, glm::mat<4, 4, float> matrix) const {
    glUniformMatrix4fv(glGetUniformLocation(this->program_id, uniform_name), 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::setIntArray(const char* uniform_name, const std::vector<int>& array) const {
    glUniform1iv(glGetUniformLocation(this->program_id, uniform_name), array.size(), array.data());
}

GLuint Shader::load_shader(const char* file_name, GLenum gl_shader_type) {
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
