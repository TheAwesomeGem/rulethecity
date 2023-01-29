#pragma once


#include <glad/glad.h>


struct Texture {

    static Texture load(const char* file_name);

    static Texture create_empty();

public:
    GLuint id;
};
