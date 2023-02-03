#include <stb_image.h>
#include "Texture.h"


Texture Texture::empty_texture{};

void Texture::init() {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    empty_texture = Texture::create_empty();
}

Texture Texture::load(const char* file_name) {
    int width, height, channels;
    unsigned char* data = stbi_load(file_name, &width, &height, &channels, 0);
    GLuint format = GL_RGB;

    if (channels == 4) {
        format = GL_RGBA;
    }

    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    return Texture{
            texture_id
    };
}

Texture Texture::create_empty() {
    unsigned char data[] = {255, 255, 255};

    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    return Texture{
            texture_id
    };
}
