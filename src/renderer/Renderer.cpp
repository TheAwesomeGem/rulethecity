#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <stb_image.h>
#include <glm/ext/matrix_transform.hpp>
#include "Renderer.h"
#include "Screen.h"


void Renderer::init(void* (* proc)(const char*)) {
    init_gl(proc);
    init_shaders();
    init_gpu_buffer();
}

void Renderer::batch_begin() {
    this->is_batch_rendering = true;
}

void Renderer::draw(const Shape& shape, const ShaderInfo& shader_info, glm::vec2 position, float rotation, glm::vec2 scale, glm::vec4 tint_color, std::optional<Texture> texture) {
    if (!this->is_batch_rendering) {
        printf("Only batch rendering is supported atm.");

        return;
    }

    if (draw_buffers.find(shader_info.type) == draw_buffers.end()) {
        draw_buffers.emplace(shader_info.type, shader_info);
    }

    DrawBuffer& draw_buffer = draw_buffers.at(shader_info.type);
    Transformation transformation{position, rotation, scale};

    // No cpu buffer exist, create a new cpu buffer and push the drawable
    if (draw_buffer.cpu_buffers.empty()) {
        // FUTURE TODO: Optimize these buffers with pre-allocation
        CpuBuffer& cpu_buffer = draw_buffer.cpu_buffers.emplace_back();
        add_to_cpu_buffer(shape, transformation, tint_color, texture, cpu_buffer);

        return;
    }

    // Cpu buffers is not empty, then get the last cpu buffer
    CpuBuffer& last_cpu_buffer = draw_buffer.cpu_buffers.back();
    size_t new_vertices_count = last_cpu_buffer.vertices_count + shape.vertices.size();
    size_t new_indices_count = last_cpu_buffer.indices_count + shape.indices.size();

    // Check for conditions to either use the last cpu buffer or create a new cpu buffer
    if (new_vertices_count >= MAX_VERTICES ||
        new_indices_count >= MAX_INDICES ||
        last_cpu_buffer.textures.size() >= MAX_TEXTURES) {
        CpuBuffer& new_cpu_buffer = draw_buffer.cpu_buffers.emplace_back();
        add_to_cpu_buffer(shape, transformation, tint_color, texture, new_cpu_buffer);
    } else {
        add_to_cpu_buffer(shape, transformation, tint_color, texture, last_cpu_buffer);
    }
}

void Renderer::batch_end() {
    size_t draw_calls = 0;

    glBindVertexArray(gpu.vao_id);
    glBindBuffer(GL_ARRAY_BUFFER, gpu.vertex_buffer_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpu.index_buffer_id);

    for (const auto& [shader_type, draw_buffer]: draw_buffers) {
        const Shader& shader = shaders.at(shader_type);
        shader.bind();

        // Each cpu buffer is subject to a draw call
        for (const CpuBuffer& cpu_buffer: draw_buffer.cpu_buffers) {
            // Draw call
            BatchedBuffer batched_buffer = generate_batched_buffer(cpu_buffer);

            const std::vector<Vertex>& gpu_vertex_buffer = batched_buffer.vertices;
            glBufferSubData(GL_ARRAY_BUFFER, 0, gpu_vertex_buffer.size() * sizeof(Vertex), gpu_vertex_buffer.data());

            const std::vector<int>& gpu_index_buffer = batched_buffer.indices;
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, gpu_index_buffer.size() * sizeof(int), gpu_index_buffer.data());

            set_shader_projection(shader);
            set_shader_textures(cpu_buffer);

            glDrawElements(GL_TRIANGLES, cpu_buffer.indices_count, GL_UNSIGNED_INT, 0);

            ++draw_calls;
            // ---
        }
    }

    // Clear the draw buffers after drawing is done
    draw_buffers.clear();

    this->is_batch_rendering = false;

    printf("Draw Count: %zu\n", draw_calls);
}

Renderer::BatchedBuffer Renderer::generate_batched_buffer(const CpuBuffer& cpu_buffer) const {
    // Reserve enough gpu vertices buffer for all the cpu vertices
    std::vector<Vertex> gpu_vertex_buffer;
    gpu_vertex_buffer.reserve(cpu_buffer.vertices_count);

    // Reserve enough gpu indices buffer for all the cpu indices
    std::vector<int> gpu_index_buffer;
    gpu_index_buffer.reserve(cpu_buffer.indices_count);

    int vertex_offset = 0;
    // Each shape needs to be added to the gpu vertex buffer
    for (const Drawable& drawable: cpu_buffer.drawables) {
        std::vector<Vertex> vertex_buffer = generate_vertex_buffer(drawable);
        gpu_vertex_buffer.insert(gpu_vertex_buffer.end(), vertex_buffer.begin(), vertex_buffer.end());

        for (int index: drawable.shape->indices) {
            gpu_index_buffer.push_back(index + vertex_offset);
        }

        vertex_offset += drawable.shape->vertices.size();
    }

    return BatchedBuffer{gpu_vertex_buffer, gpu_index_buffer};
}

void Renderer::set_shader_projection(const Shader& shader) {
    glm::mat4 projection = glm::ortho(0.0F, (float) Screen::WIDTH, 0.0F, (float) Screen::HEIGHT);
    shader.setMatrix("u_projection", projection);
}

void Renderer::set_shader_textures(const CpuBuffer& cpu_buffer) {
    glBindTextureUnit(0, Texture::empty_texture.id);

    size_t tex_index = 1;
    for (GLuint texture_id: cpu_buffer.textures) {
        glBindTextureUnit(tex_index, texture_id);

        printf("Loading Texture(%d): %d\n", tex_index, texture_id);

        ++tex_index;
    }
}

void Renderer::add_to_cpu_buffer(const Shape& shape, Transformation transformation, glm::vec4 tint_color, std::optional<Texture> texture, Renderer::CpuBuffer& cpu_buffer) {
    Drawable& drawable = cpu_buffer.drawables.emplace_back(&shape, transformation);
    cpu_buffer.vertices_count += shape.vertices.size();
    cpu_buffer.indices_count += shape.indices.size();
    drawable.tint_color = tint_color;

    if (texture.has_value()) {
        auto it = std::find(cpu_buffer.textures.begin(), cpu_buffer.textures.end(), texture->id);

        if (it == cpu_buffer.textures.end()) {
            size_t texture_index = cpu_buffer.textures.size();
            cpu_buffer.textures.push_back(texture->id);
            drawable.texture_index = texture_index;
        } else {
            drawable.texture_index = std::distance(cpu_buffer.textures.begin(), it);
            printf("Texture already exists at: %zu\n", drawable.texture_index);
        }
    }
}

std::vector<Renderer::Vertex> Renderer::generate_vertex_buffer(const Renderer::Drawable& drawable) const {
    // FUTURE TODO: Profile this! Shapes with tons of vertices might be slow to render
    size_t vertices_count = drawable.shape->vertices.size();
    std::vector<Vertex> vertices;
    vertices.reserve(vertices_count);
    Transformation transformation = drawable.transformation;

    for (size_t i = 0; i < vertices_count; ++i) {
        // FUTURE TODO: Profile if we should do transformation in the graphics card at the cost of more memory
        glm::vec3 shape_vertices = drawable.shape->vertices[i];
        glm::mat4 transform = glm::mat4{1.0F};
        transform = glm::translate(transform, glm::vec3{transformation.position.x, transformation.position.y, 0.0F});
        transform = glm::rotate(transform, glm::degrees(transformation.rotation), glm::vec3{0.0F, 1.0F, 0.0F});
        transform = glm::scale(transform, glm::vec3{transformation.scale.x, transformation.scale.y, 1.0F});
        glm::vec3 transformed_vertices = transform * glm::vec4{shape_vertices.x, shape_vertices.y, shape_vertices.z, 1.0F};
        // FUTURE TODO: How to deal with z?
        vertices.emplace_back(transformed_vertices, drawable.tint_color, drawable.shape->uvs[i], (float) (drawable.texture_index + 1));
    }

    return vertices;
}

static void APIENTRY openglCallbackFunction(
        GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar* message,
        const void* userParam
) {
    (void) source;
    (void) type;
    (void) id;
    (void) severity;
    (void) length;
    (void) userParam;
    fprintf(stderr, "%s\n", message);
    if (severity == GL_DEBUG_SEVERITY_HIGH) {
        fprintf(stderr, "Aborting...\n");
        abort();
    }
}

void Renderer::init_gl(void* (* proc)(const char*)) {
    gladLoadGLLoader(proc);
    printf("OpenGL loaded\n");
    printf("Vendor           : %s\n", glGetString(GL_VENDOR));
    printf("Renderer         : %s\n", glGetString(GL_RENDERER));
    printf("Version          : %s\n", glGetString(GL_VERSION));
    printf("GLSL             : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Enable the debug callback
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(openglCallbackFunction, nullptr);
    glDebugMessageControl(
            GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true
    );

    glViewport(0, 0, Screen::WIDTH, Screen::HEIGHT); // Rendering Viewport
    // 28, 44, 50
    glClearColor(0.11f, 0.172f, 0.196f, 1.0f); // Clear color for the color bit field

}

void Renderer::init_gpu_buffer() {
    this->gpu = Gpu{0, 0};

    glGenVertexArrays(1, &gpu.vao_id);
    glBindVertexArray(gpu.vao_id);
    init_batch_vbo();
    init_batch_ibo();
    glBindVertexArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// TODO: This needs to change to have support different shaders
// TODO: We need to generate a different vbo and vao for different shaders
void Renderer::init_batch_vbo() {
    glGenBuffers(1, &gpu.vertex_buffer_id);

    glBindBuffer(GL_ARRAY_BUFFER, gpu.vertex_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * MAX_VERTICES, nullptr, GL_DYNAMIC_DRAW);

    GLuint point_attrib_index = 0;
    glEnableVertexAttribArray(point_attrib_index);
    glVertexAttribPointer(point_attrib_index, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*) offsetof(Vertex, point));

    GLuint color_attrib_index = 1;
    glEnableVertexAttribArray(color_attrib_index);
    glVertexAttribPointer(color_attrib_index, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*) offsetof(Vertex, color));

    GLuint uv_attrib_index = 2;
    glEnableVertexAttribArray(uv_attrib_index);
    glVertexAttribPointer(uv_attrib_index, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*) offsetof(Vertex, uv));

    GLuint texture_index_attrib_index = 3;
    glEnableVertexAttribArray(texture_index_attrib_index);
    glVertexAttribPointer(texture_index_attrib_index, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*) offsetof(Vertex, texture_index));
}

void Renderer::init_batch_ibo() {
    glGenBuffers(1, &gpu.index_buffer_id);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpu.index_buffer_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * MAX_INDICES, nullptr, GL_DYNAMIC_DRAW);
}

void Renderer::init_shaders() {
    Texture::init();

    std::array<ShaderType, 1> shader_types{ShaderType::FILLED_QUAD};

    for (ShaderType shader_type: shader_types) {
        Shader shader{};
        shader.init();

        shader.bind();
        std::vector<int> textures{};
        textures.reserve(MAX_TEXTURES);

        for (int i = 0; i < MAX_TEXTURES + 1; ++i) {
            textures.push_back(i);
        }

        shader.setIntArray("u_textures", textures);
        shader.unbind();

        shaders.emplace(shader_type, shader);
    }
}
