#include <glad/glad.h>
#include <stb_image.h>
#include "Renderer.h"
#include "Screen.h"


void Renderer::init(void* (* proc)(const char*)) {
    init_gl(proc);

    Texture::init();
}

void Renderer::draw(const Shape* shape, glm::vec2 position, glm::vec2 scale, glm::vec4 tint_color, std::optional<Texture> texture) {
    // Batch not found for this shape. Create one!
    if (batches.find(shape->id) == batches.end()) {
        auto [batch, _] = batches.emplace(shape->id, RenderBatch{shape});
        batch->second.init();

        printf("Initializing batch.\n");
    }

    RenderBatch& batch = batches.at(shape->id);
    batch.queue(
            position, scale, tint_color, texture
    );
}

void Renderer::flush() {
    for (auto& [_, batch]: batches) {
        batch.flush();
    }
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
