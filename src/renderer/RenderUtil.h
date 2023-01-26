#pragma once

#include "Renderer.h"


void draw_solid_rect_color(Renderer& renderer, glm::vec2 position, glm::vec2 size, glm::vec4 color) {
    renderer.draw(
            DrawableQuad{
                    position,
                    size,
                    color
            }
    );
}

void draw_border_rect_color(Renderer& renderer, glm::vec2 position, glm::vec2 size, float border_thickness, glm::vec4 color) {
    draw_solid_rect_color(renderer, position, glm::vec2{size.x, border_thickness}, color); // TOP

    draw_solid_rect_color(renderer, glm::vec2{
            position.x,
            position.y + size.y - border_thickness
    }, glm::vec2{size.x, border_thickness}, color); // BOTTOM

    draw_solid_rect_color(renderer, position, glm::vec2{
            border_thickness,
            size.y
    }, color); // LEFT

    draw_solid_rect_color(renderer, glm::vec2{
            position.x + size.x - border_thickness,
            position.y
    }, glm::vec2{
            border_thickness,
            size.y
    }, color); // RIGHT
}

void draw_solid_circle_color(Renderer& renderer, glm::vec2 position, float radius, glm::vec4 color) {
    glm::vec2 size{radius * 2.0F, radius * 2.0F};

    renderer.draw(
            DrawableQuad{
                    position,
                    size,
                    color
            },
            renderer.round_texture
    );
}
