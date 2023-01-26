#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <stb_image.h>
#include "Renderer.h"
#include "Screen.h"


void Renderer::init(void* (* proc)(const char*)) {
    init_gl(proc);
    shader.init();
    init_gpu_buffer();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    no_texture = Texture::load("texture/no_texture.png");
    round_texture = Texture::load("texture/round_texture.png");
}

void Renderer::batch_begin() {
    this->is_batch_rendering = true;
}

void Renderer::draw(DrawableQuad drawable, std::optional<Texture> texture_opt) {
    if (!this->is_batch_rendering) {
        printf("Only batch rendering is supported atm.");

        return;
    }

    Texture texture;
    if (texture_opt.has_value()) {
        texture = *texture_opt;
    } else {
        texture = no_texture;
    }

    // No cpu buffer exist, create a new cpu buffer and push the drawable
    if (this->cpu_buffers.empty()) {
        // Future TODO: Optimize these buffers with pre-allocation
        CpuBuffer& cpu_buffer = this->cpu_buffers.emplace_back();
        Renderer::add_texture_to_batch(cpu_buffer, drawable, texture);
        cpu_buffer.drawables.push_back(drawable);

        return;
    }

    // Cpu buffers is not empty, then get the last cpu buffer
    CpuBuffer& last_cpu_buffer = this->cpu_buffers.back();
    size_t quad_byte = quad_bytes(1);
    size_t bytes_left = quad_bytes(MAX_QUADS) - last_cpu_buffer.bytes_used();

    // Check for conditions to either use the last cpu buffer or create a new cpu buffer
    if (bytes_left <= quad_byte || last_cpu_buffer.texture_count() > MAX_TEXTURES) {
        CpuBuffer& new_cpu_buffer = this->cpu_buffers.emplace_back();
        Renderer::add_texture_to_batch(new_cpu_buffer, drawable, texture);
        new_cpu_buffer.drawables.push_back(drawable);
    } else {
        auto it = std::find(last_cpu_buffer.textures.begin(), last_cpu_buffer.textures.end(), texture.id);

        if (it == last_cpu_buffer.textures.end()) {
            Renderer::add_texture_to_batch(last_cpu_buffer, drawable, texture);
        } else {
            drawable.texture_index = std::distance(last_cpu_buffer.textures.begin(), it);
            printf("Texture already exists at: %zu\n", drawable.texture_index);
        }

        last_cpu_buffer.drawables.push_back(drawable);
    }
}

void Renderer::batch_end() {
    size_t draw_calls = 0;

    auto vertices = quad_vertex();
    auto uvs = quad_uv();

    glBindVertexArray(gpu.vao_id);
    glBindBuffer(GL_ARRAY_BUFFER, gpu.vertex_buffer_id);

    // Each cpu buffer is subject to a draw call
    for (const CpuBuffer& cpu_buffer: cpu_buffers) {
        // Draw call
        std::vector<Vertex> gpu_vertex_buffer = generate_batched_buffer(cpu_buffer, vertices, uvs);
        glBufferSubData(GL_ARRAY_BUFFER, 0, gpu_vertex_buffer.size() * sizeof(Vertex), gpu_vertex_buffer.data());

        shader.bind();

        set_shader_projection();
        set_shader_textures(cpu_buffer);

        glDrawElements(GL_TRIANGLES, cpu_buffer.indices_count(), GL_UNSIGNED_INT, 0);

        ++draw_calls;
        // ---
    }

    // Clear the cpu buffer after drawing is done
    cpu_buffers.clear();

    this->is_batch_rendering = false;

    printf("Draw Count: %zu\n", draw_calls);
}

std::vector<Renderer::Vertex> Renderer::generate_batched_buffer(const CpuBuffer& cpu_buffer, const std::array<glm::vec3, 4>& vertices, const std::array<glm::vec2, 4>& uvs) const {
    // Reserve enough gpu vertices buffer for all the cpu vertices
    std::vector<Vertex> gpu_vertex_buffer;
    gpu_vertex_buffer.reserve(cpu_buffer.vertices_count());

    // Each quad needs to be transformed to world position and added to the gpu vertex buffer
    for (const DrawableQuad& drawable: cpu_buffer.drawables) {
        glm::vec3 scale = glm::vec3{drawable.size.x * 0.5F, drawable.size.y * 0.5F, 1.0F};
        glm::mat4 transform = glm::mat4{1.0F};
        transform = glm::translate(transform, glm::vec3(drawable.position.x + scale.x, drawable.position.y + scale.y, 0.0F));
        // NOTE: Rotate here.
        transform = glm::scale(transform, scale);

        for (size_t i = 0; i < QUAD_VERTICES; ++i) {
            glm::vec3 vertex_point = vertices[i];
            glm::vec2 uv_point = uvs[i];

            glm::vec4 world_point = transform * glm::vec4(vertex_point.x, vertex_point.y, vertex_point.z, 1.0F);
            gpu_vertex_buffer.push_back(Vertex{world_point, drawable.color, uv_point, static_cast<float>(drawable.texture_index)});
        }

        printf("Drawing Texture Index: %zu\n", drawable.texture_index);
    }

    return gpu_vertex_buffer;
}

void Renderer::set_shader_projection() {
    glm::mat4 projection = glm::ortho(0.0F, (float) Screen::WIDTH, 0.0F, (float) Screen::HEIGHT);
    shader.setMatrix("u_projection", projection);
}

void Renderer::set_shader_textures(const CpuBuffer& cpu_buffer) {
    std::vector<int> textures;
    textures.reserve(cpu_buffer.texture_count());
    size_t tex_index = 0;
    for (GLuint texture_id: cpu_buffer.textures) {
        glBindTextureUnit(tex_index, texture_id);
        textures.push_back(tex_index);

        ++tex_index;

        printf("Loading Texture(%d): %d\n", tex_index, texture_id);
    }
    shader.setIntArray("u_textures", textures);
}

void Renderer::add_texture_to_batch(Renderer::CpuBuffer& cpu_buffer, DrawableQuad& drawable, const Texture& texture) {
    size_t texture_index = cpu_buffer.texture_count();
    cpu_buffer.textures.push_back(texture.id);
    drawable.texture_index = texture_index;
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

    uint32_t indices[MAX_INDICES];
    uint32_t offset = 0;

    for (int i = 0; i < MAX_INDICES; i += 6) {
        indices[i] = 0 + offset;
        indices[i + 1] = 1 + offset;
        indices[i + 2] = 3 + offset;
        indices[i + 3] = 0 + offset;
        indices[i + 4] = 3 + offset;
        indices[i + 5] = 2 + offset;

        offset += 4;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpu.index_buffer_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices, GL_STATIC_DRAW);
}
