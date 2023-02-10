#pragma once

#include "Renderer.h"


namespace ShapeGenerator {
    static Shape generate_quad(size_t id, ShaderProgram shader_program) {
        return Shape{
                id,
                {
                        Shape::Vertex{{0.0F, 0.0F}, {0.0F, 0.0F}},
                        Shape::Vertex{{1.0F, 0.0F}, {1.0F, 0.0F}},
                        Shape::Vertex{{0.0F, 1.0F}, {0.0F, 1.0F}},
                        Shape::Vertex{{1.0, 1.0F}, {1.0F, 1.0F}}
                },
                {
                        0, 1, 3, 0, 3, 2
                },
                {},
                shader_program
        };
    }

    static Shape generate_triangle(size_t id, ShaderProgram shader_program) {
        return Shape{
                id,
                {
                        Shape::Vertex{{0.0F, 0.0F}, {0.0F, 0.0F}},
                        Shape::Vertex{{1.0F, 0.0F}, {1.0F, 0.0F}},
                        Shape::Vertex{{0.5F, 1.0F}, {0.5F, 1.0F}},
                },
                {
                        0, 1, 2
                },
                {},
                shader_program
        };
    }
};
