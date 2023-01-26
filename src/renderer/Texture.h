#pragma once


#include <glad/glad.h>


struct Texture {

    static Texture load(const char* file_name);

public:
    GLuint id;
};
