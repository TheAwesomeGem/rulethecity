#pragma once

#include "Renderer.h"


namespace ShapeGenerator {
    static Shape generate_quad() {
        return Shape{
                {
                        {0.0F, 0.0F, 0.0F},
                        {1.0F, 0.0F, 0.0F},
                        {0.0F, 1.0F, 0.0F},
                        {1.0, 1.0F, 0.0F}
                },
                {
                        0, 1, 3, 0, 3, 2
                },
                {
                        {0.0F, 0.0F},
                        {1.0F, 0.0F},
                        {0.0F, 1.0F},
                        {1.0F, 1.0F},
                }
        };
    }

    static Shape generate_triangle() {
        return Shape{
                {
                        {0.0F, 0.0F, 0.0F},
                        {1.0F, 0.0F, 0.0F},
                        {0.5F, 1.0F, 0.0F},
                },
                {
                        0, 1, 2
                },
                {
                        {0.0F, 0.0F},
                        {1.0F, 0.0F},
                        {0.5F, 1.0F}
                }
        };
    }
};
