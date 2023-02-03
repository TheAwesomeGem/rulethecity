#pragma once


#include <glad/glad.h>


struct Texture {

    static void init();

    static Texture load(const char* file_name);

    static Texture create_empty();

public:
    GLuint id;

    static Texture empty_texture;
};
