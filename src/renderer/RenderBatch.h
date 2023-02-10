#pragma once

#include <optional>
#include "Shape.h"
#include "Texture.h"


static constexpr const size_t MAX_VERTICES = 4000;
static constexpr const size_t MAX_INDICES = 6000;

struct RenderBatch {
private:
    struct Gpu {
        GLuint gl_vao_id;
        GLuint gl_vbo_id;
        GLuint gl_ibo_id;
    };

    struct Transform {
        // FUTURE TODO: This should be 3d coordinate to take 'Z' for z-sorting
        glm::vec2 position = glm::vec2{0.0F, 0.0F};
        float rotation = 0.0F;
        glm::vec2 scale = glm::vec2{1.0F, 1.0F};
    };

    struct Drawable {
        explicit Drawable(Transform transform_)
                : transform{transform_}, tint_color{1.0F, 1.0F, 1.0F, 1.0F}, texture_index{-1} {

        }

        Transform transform;
        glm::vec4 tint_color;
        int texture_index;
    };

    struct RenderBuffer {
        std::vector<Drawable> draw_buffer;
        std::vector<GLuint> textures;

        size_t vertices_count;
        size_t indices_count;
    };

    struct BatchedBuffer {
        std::vector<float> vertices;
        std::vector<int> indices;

        size_t vertices_size;
    };

public:

    explicit RenderBatch(const Shape* shape_) : render_buffers { }, gpu {}, shape { shape_ }
    {
    }

    // Initializes the GPU buffers
    void init();

    // Queues to the RenderBuffer
    void queue(glm::vec2 position, glm::vec2 scale, glm::vec4 tint_color, std::optional<Texture> = std::nullopt);

    // Executes a draw call
    void flush();

private:
    [[nodiscard]] BatchedBuffer generate_batched_buffer(const RenderBuffer& render_buffer) const;

    [[nodiscard]] std::vector<float> generate_vertex_buffer(const RenderBatch::Drawable& drawable) const;

    void set_shader_projection(const ShaderProgram& shader);

    void set_shader_textures(const RenderBuffer& render_buffer);

    void init_gpu_buffer();

    void init_batch_vbo();

    void init_batch_ibo();

    void add_to_render_buffer(Transform transform, glm::vec4 tint_color, std::optional<Texture> texture, RenderBuffer& render_buffer);

private:
    // This buffer exists only on CPU
    std::vector<RenderBuffer> render_buffers;

    // Gpu Data
    Gpu gpu;

    // Every different shape has its own RenderBatch
    const Shape* shape;
};
