#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <vector>
#include <array>
#include <unordered_set>
#include <optional>
#include "Shader.h"
#include "Texture.h"


static constexpr const size_t MAX_QUADS = 1000;
static constexpr const size_t QUAD_VERTICES = 4;
static constexpr const size_t QUAD_INDICES = 6;
static constexpr const size_t MAX_VERTICES = MAX_QUADS * QUAD_VERTICES;
static constexpr const size_t MAX_INDICES = MAX_QUADS * QUAD_INDICES;
static constexpr const size_t MAX_TEXTURES = 8;

struct DrawableQuad {
    glm::vec2 position;
    glm::vec2 size;
    glm::vec4 color;
    size_t texture_index = 999999;
};

class Renderer {
private:
    struct Gpu {
        GLuint vao_id;
        GLuint vertex_buffer_id;
        GLuint index_buffer_id;
    };

    struct Vertex {
        glm::vec3 point;
        glm::vec4 color;
        glm::vec2 uv;
        float texture_index;
    };

    struct CpuBuffer {
        std::vector<DrawableQuad> drawables;
        std::vector<GLuint> textures;

        [[nodiscard]] size_t bytes_used() const {
            return quad_bytes(drawables.size());
        }

        [[nodiscard]] size_t vertices_count() const {
            return drawables.size() * QUAD_VERTICES;
        }

        [[nodiscard]] size_t indices_count() const {
            return drawables.size() * QUAD_INDICES;
        }

        [[nodiscard]] size_t texture_count() const {
            return textures.size();
        }
    };

public:
    void init(void* (* proc)(const char*));

    void batch_begin(); // Initializes the batching

    void draw(DrawableQuad drawable, std::optional<Texture> texture_opt = std::nullopt); // Adds the drawable for rendering

    void batch_end(); // Executes the actual draw command

private:
    void init_gl(void* (* proc)(const char*));

    void init_gpu_buffer();

    void init_batch_vbo();

    void init_batch_ibo();

    [[nodiscard]] std::vector<Vertex> generate_batched_buffer(const CpuBuffer& cpu_buffer, const std::array<glm::vec3, 4>& vertices, const std::array<glm::vec2, 4>& uvs) const;

    void set_shader_projection();

    void set_shader_textures(const CpuBuffer& cpu_buffer);

    static void add_texture_to_batch(Renderer::CpuBuffer& cpu_buffer, DrawableQuad& drawable, const Texture& texture);

    [[nodiscard]] static constexpr size_t quad_bytes(size_t quads) {
        return (sizeof(Vertex) * QUAD_VERTICES) * quads;
    }

    [[nodiscard]] static constexpr std::array<glm::vec3, 4> quad_vertex() {
        return std::array<glm::vec3, 4>{
                glm::vec3{-1.0F, 1.0F, 0.0F},       // BOTTOM LEFT
                glm::vec3{1.0F, 1.0F, 0.0F},        // BOTTOM RIGHT
                glm::vec3{-1.0F, -1.0F, 0.0F},      // TOP LEFT
                glm::vec3{1.0, -1.0F, 0.0F}         // TOP RIGHT
        };
    }

    [[nodiscard]] static constexpr std::array<glm::vec2, 4> quad_uv() {
        return std::array<glm::vec2, 4>{
                glm::vec2{0.0F, 0.0F},                  // BOTTOM LEFT
                glm::vec2{1.0F, 0.0F},                  // BOTTOM RIGHT
                glm::vec2{0.0F, 1.0F},                  // TOP LEFT
                glm::vec2{1.0, 1.0F}                    // TOP RIGHT
        };
    }

private:
    Shader shader;
    Gpu gpu;
    std::vector<CpuBuffer> cpu_buffers;
    bool is_batch_rendering = false;
    Texture no_texture;
public:
    Texture round_texture;
};