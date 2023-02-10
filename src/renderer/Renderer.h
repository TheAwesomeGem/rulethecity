#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <utility>
#include <vector>
#include <array>
#include <unordered_set>
#include <optional>
#include <unordered_map>
#include "ShaderProgram.h"
#include "Texture.h"
#include "Shape.h"
#include "RenderBatch.h"


class Renderer {
public:
    void init(void* (* proc)(const char*));

    void draw(const Shape* shape,
              glm::vec2 position,
              glm::vec2 scale,
              glm::vec4 tint_color = {1.0F, 1.0F, 1.0F, 1.0F},
              std::optional<Texture> texture = std::nullopt); // Adds the shape for rendering

    void flush(); // Executes the actual draw command

private:
    void init_gl(void* (* proc)(const char*));
private:
    std::unordered_map<size_t, RenderBatch> batches;
};