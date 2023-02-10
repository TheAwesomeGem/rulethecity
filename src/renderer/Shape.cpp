#include "Shape.h"


void Shape::init() {
    init_vertex_layout();
}

void Shape::init_vertex_layout() {
    std::vector<VertexAttrib> attributes;

    populate_base_attributes(attributes);
    populate_extra_attributes(attributes);

    size_t vertex_components = 0;

    for (const VertexAttrib& attrib: attributes) {
        vertex_components += attrib.gl_component_count;
    }

    this->vertex_layout = VertexLayout{
            vertex_components,
            attributes
    };
}

void Shape::populate_base_attributes(std::vector<VertexAttrib>& attributes) {
    attributes.emplace_back(
            AttributeType::POINT_POSITION, GL_FLOAT, 3
    );

    attributes.emplace_back(
            AttributeType::TINT_COLOR, GL_FLOAT, 4
    );

    attributes.emplace_back(
            AttributeType::UV, GL_FLOAT, 2
    );

    attributes.emplace_back(
            AttributeType::TEXTURE_INDEX, GL_FLOAT, 1
    );
}

void Shape::populate_extra_attributes(std::vector<VertexAttrib>& attributes) {

}

std::vector<float> Shape::generate_vertex(glm::vec3 transformed_position, glm::vec4 tint_color, glm::vec2 uv, float texture_index) const {
    std::vector<float> new_vertices;

    for (const VertexAttrib& attrib: vertex_layout.attributes) {
        switch (attrib.type) {
            case AttributeType::POINT_POSITION: {
                new_vertices.push_back(transformed_position.x);
                new_vertices.push_back(transformed_position.y);
                new_vertices.push_back(transformed_position.z);
                break;
            }
            case AttributeType::TINT_COLOR: {
                new_vertices.push_back(tint_color.x);
                new_vertices.push_back(tint_color.y);
                new_vertices.push_back(tint_color.z);
                new_vertices.push_back(tint_color.w);
                break;
            }
            case AttributeType::UV: {
                new_vertices.push_back(uv.x);
                new_vertices.push_back(uv.y);
                break;
            }
            case AttributeType::TEXTURE_INDEX: {
                new_vertices.push_back(texture_index);
                break;
            }
        }
    }

    // FUTURE TODO: Fill up extra attributes here for custom shaders

    return new_vertices;
}
