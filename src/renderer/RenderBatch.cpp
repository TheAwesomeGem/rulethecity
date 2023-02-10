#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "RenderBatch.h"
#include "Screen.h"


void RenderBatch::init() {
    assert(shape != nullptr);

    init_gpu_buffer();
}

void RenderBatch::queue(glm::vec2 position, glm::vec2 scale, glm::vec4 tint_color, std::optional<Texture> texture) {
    Transform transform{position, 0.0F, scale};

    // No render buffer exist, create a new render buffer and push the drawable
    if (render_buffers.empty()) {
        // FUTURE TODO: Optimize these buffers with pre-allocation
        RenderBuffer& render_buffer = render_buffers.emplace_back();
        add_to_render_buffer(transform, tint_color, texture, render_buffer);

        printf("Queing batch: %d.\n", render_buffers.size());

        return;
    }

    // Render buffers is not empty, then get the last render buffer
    RenderBuffer& last_render_buffer = render_buffers.back();
    size_t new_vertices_count = last_render_buffer.vertices_count + shape->vertices.size();
    size_t new_indices_count = last_render_buffer.indices_count + shape->indices.size();

    // Check for conditions to either use the last render buffer or create a new render buffer
    if (new_vertices_count >= MAX_VERTICES ||
        new_indices_count >= MAX_INDICES ||
        last_render_buffer.textures.size() >= MAX_TEXTURES) {
        RenderBuffer& new_render_buffer = render_buffers.emplace_back();
        add_to_render_buffer(transform, tint_color, texture, new_render_buffer);
    } else {
        add_to_render_buffer(transform, tint_color, texture, last_render_buffer);
    }

    printf("Queing batch: %d.\n", render_buffers.size());
}

void RenderBatch::flush() {
    printf("Flushing batch: %d.\n", render_buffers.size());

    shape->shader_program.bind();

    glBindVertexArray(gpu.gl_vao_id);
    glBindBuffer(GL_ARRAY_BUFFER, gpu.gl_vbo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpu.gl_ibo_id);

    // Each render buffer is subject to a draw call
    for (const RenderBuffer& render_buffer: render_buffers) {
        printf("Drawing buffer.\n");
        // Draw call
        BatchedBuffer batched_buffer = generate_batched_buffer(render_buffer);
        batched_buffer.vertices[7] = 0.0F;
        batched_buffer.vertices[8] = 0.0F;
        batched_buffer.vertices[17] = 1.0F;
        batched_buffer.vertices[18] = 0.0F;
        batched_buffer.vertices[27] = 0.0F;
        batched_buffer.vertices[28] = 1.0F;
        batched_buffer.vertices[37] = 1.0F;
        batched_buffer.vertices[38] = 1.0F;
        glBufferSubData(GL_ARRAY_BUFFER, 0, batched_buffer.vertices_size, batched_buffer.vertices.data());

        const std::vector<int>& gpu_index_buffer = batched_buffer.indices;
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, gpu_index_buffer.size() * sizeof(int), gpu_index_buffer.data());

        set_shader_projection(shape->shader_program);
        set_shader_textures(render_buffer);

        glDrawElements(shape->gl_render_mode, render_buffer.indices_count, GL_UNSIGNED_INT, 0);
        // ---
    }

    render_buffers.clear();

    printf("Clearing batch.\n");
}

RenderBatch::BatchedBuffer RenderBatch::generate_batched_buffer(const RenderBatch::RenderBuffer& render_buffer) const {
    // Reserve enough gpu vertices buffer for all the cpu vertices
    std::vector<float> gpu_vertex_buffer;
    gpu_vertex_buffer.reserve(render_buffer.vertices_count * shape->vertex_layout.vertex_components);

    // Reserve enough gpu indices buffer for all the cpu indices
    std::vector<int> gpu_index_buffer;
    gpu_index_buffer.reserve(render_buffer.indices_count);

    int vertex_offset = 0;
    // Each shape needs to be added to the gpu vertex buffer
    for (const Drawable& drawable: render_buffer.draw_buffer) {
        std::vector<float> vertex_buffer = generate_vertex_buffer(drawable);
        gpu_vertex_buffer.insert(gpu_vertex_buffer.end(), vertex_buffer.begin(), vertex_buffer.end());

        for (int index: shape->indices) {
            gpu_index_buffer.push_back(index + vertex_offset);
        }

        vertex_offset += shape->vertices.size();
    }

    return BatchedBuffer{gpu_vertex_buffer, gpu_index_buffer,
            gpu_vertex_buffer.size() * sizeof(float)};
}

std::vector<float> RenderBatch::generate_vertex_buffer(const RenderBatch::Drawable& drawable) const {
    // FUTURE TODO: Profile this! Shapes with tons of vertices might be slow to render
    size_t vertices_count = shape->vertices.size();
    std::vector<float> vertices;
    vertices.reserve(vertices_count * shape->vertex_layout.vertex_components);
    Transform transform = drawable.transform;

    for (const Shape::Vertex& vertex: shape->vertices) {
        // FUTURE TODO: Profile if we should do transformation in the graphics card at the cost of more memory
        glm::mat4 transformation_matrix = glm::mat4{1.0F};
        transformation_matrix = glm::translate(transformation_matrix, glm::vec3{transform.position.x, transform.position.y, 0.0F});
        transformation_matrix = glm::rotate(transformation_matrix, glm::degrees(transform.rotation), glm::vec3{0.0F, 1.0F, 0.0F});
        transformation_matrix = glm::scale(transformation_matrix, glm::vec3{transform.scale.x, transform.scale.y, 1.0F});
        glm::vec3 transformed_position = transformation_matrix * glm::vec4{vertex.points.x, vertex.points.y, 0.0F, 1.0F};
        // FUTURE TODO: How to deal with z?

        std::vector<float> generated_vertices = shape->generate_vertex(transformed_position, drawable.tint_color, vertex.uvs, (float) (drawable.texture_index + 1));
        vertices.insert(vertices.end(), generated_vertices.begin(), generated_vertices.end());
    }

    return vertices;
}

void RenderBatch::add_to_render_buffer(RenderBatch::Transform transform, glm::vec4 tint_color, std::optional<Texture> texture, RenderBatch::RenderBuffer& render_buffer) {
    Drawable& drawable = render_buffer.draw_buffer.emplace_back(transform);
    render_buffer.vertices_count += shape->vertices.size();
    render_buffer.indices_count += shape->indices.size();
    drawable.tint_color = tint_color;

    if (texture.has_value()) {
        auto it = std::find(render_buffer.textures.begin(), render_buffer.textures.end(), texture->id);

        if (it == render_buffer.textures.end()) {
            size_t texture_index = render_buffer.textures.size();
            render_buffer.textures.push_back(texture->id);
            drawable.texture_index = texture_index;
        } else {
            drawable.texture_index = std::distance(render_buffer.textures.begin(), it);
            printf("Texture already exists at: %zu\n", drawable.texture_index);
        }
    }
}

void RenderBatch::set_shader_projection(const ShaderProgram& shader) {
    glm::mat4 projection = glm::ortho(0.0F, (float) Screen::WIDTH, 0.0F, (float) Screen::HEIGHT);
    shader.setMatrix("u_projection", projection);
}

void RenderBatch::set_shader_textures(const RenderBuffer& render_buffer) {
    glBindTextureUnit(0, Texture::empty_texture.id);

    size_t tex_index = 1;
    for (GLuint texture_id: render_buffer.textures) {
        glBindTextureUnit(tex_index, texture_id);

        printf("Loading Texture(%d): %d\n", tex_index, texture_id);

        ++tex_index;
    }
}

void RenderBatch::init_gpu_buffer() {
    this->gpu = Gpu{0, 0};

    glGenVertexArrays(1, &gpu.gl_vao_id);
    glBindVertexArray(gpu.gl_vao_id);
    init_batch_vbo();
    init_batch_ibo();
    glBindVertexArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void RenderBatch::init_batch_vbo() {
    glGenBuffers(1, &gpu.gl_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, gpu.gl_vbo_id);

    const Shape::VertexLayout& vertex_layout = shape->vertex_layout;

    glBufferData(GL_ARRAY_BUFFER, (vertex_layout.vertex_components * sizeof(float)) * MAX_VERTICES, nullptr, GL_DYNAMIC_DRAW);
    size_t prev_size_in_bytes = 0;

    for (size_t i = 0; i < vertex_layout.attributes.size(); ++i) {
        const Shape::VertexAttrib& vertex_attrib = vertex_layout.attributes[i];

        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, vertex_attrib.gl_component_count, vertex_attrib.gl_data_type, GL_FALSE, (vertex_layout.vertex_components * sizeof(float)),
                              (const void*) prev_size_in_bytes);

        prev_size_in_bytes += vertex_attrib.bytes();
    }
}

void RenderBatch::init_batch_ibo() {
    glGenBuffers(1, &gpu.gl_ibo_id);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpu.gl_ibo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * MAX_INDICES, nullptr, GL_DYNAMIC_DRAW);
}
