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


static constexpr const size_t MAX_VERTICES = 4000;
static constexpr const size_t MAX_INDICES = 6000;
static constexpr const size_t MAX_TEXTURES = 8;

struct Shape {
    std::vector<glm::vec3> vertices;
    std::vector<int> indices;
    std::vector<glm::vec2> uvs;
};

struct Transformation {
    glm::vec2 position;
    float rotation;
    glm::vec2 scale;
};

class Renderer {
private:
    struct Drawable {
        Drawable(const Shape* shape_, Transformation transformation_)
                : shape{shape_}, transformation{transformation_}, texture_index{-1} {

        }

        const Shape* shape;
        Transformation transformation;
        glm::vec4 tint_color;
        int texture_index;
    };

    struct Gpu {
        GLuint vao_id;
        GLuint vertex_buffer_id;
        GLuint index_buffer_id;
    };

    struct Vertex {
        Vertex(glm::vec3 point_, glm::vec4 color_, glm::vec2 uv_, float texture_index_)
                : point{point_}, color{color_}, uv{uv_}, texture_index{texture_index_} {

        }

        glm::vec3 point;
        glm::vec4 color;
        glm::vec2 uv;
        float texture_index;
    };

    struct CpuBuffer {
        std::vector<Drawable> drawables;
        std::vector<GLuint> textures;
        size_t vertices_count;
        size_t indices_count;

        [[nodiscard]] size_t bytes_used() const {
            return sizeof(Vertex) * vertices_count;
        }
    };

    struct BatchedBuffer {
        std::vector<Vertex> vertices;
        std::vector<int> indices;
    };

public:
    void init(void* (* proc)(const char*));

    void batch_begin(); // Initializes the batching

    void draw(const Shape& shape,
              glm::vec2 position,
              float rotation,
              glm::vec2 scale,
              glm::vec4 tint_color = {1.0F, 1.0F, 1.0F, 1.0F},
              std::optional<Texture> texture = std::nullopt); // Adds the shape for rendering

    void batch_end(); // Executes the actual draw command

private:
    void init_gl(void* (* proc)(const char*));

    void init_gpu_buffer();

    void init_batch_vbo();

    void init_batch_ibo();

    void init_shader();

    [[nodiscard]] BatchedBuffer generate_batched_buffer(const CpuBuffer& cpu_buffer) const;

    void set_shader_projection();

    void set_shader_textures(const CpuBuffer& cpu_buffer);

    static void add_to_cpu_buffer(const Shape& shape, Transformation transformation, glm::vec4 tint_color, std::optional<Texture> texture, CpuBuffer& cpu_buffer);

    [[nodiscard]] std::vector<Vertex> generate_vertex_buffer(const Drawable& drawable) const;

private:
    Shader shader;
    Gpu gpu;
    std::vector<CpuBuffer> cpu_buffers;
    bool is_batch_rendering = false;
    Texture empty_texture;
};