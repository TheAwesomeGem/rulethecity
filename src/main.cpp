#define GLM_FORCE_RADIANS 1
#define SDL_MAIN_HANDLED

#include <cstdio>
#include <SDL2/SDL.h>
#include <glad/glad.h>
#include "renderer/Screen.h"
#include "renderer/Renderer.h"
#include "renderer/ShapeGenerator.h"


// Globals
static SDL_Window* window = nullptr;
static SDL_GLContext main_context;
// =======

// TODO:
// ============
// TODO: Figure out how to do performant circle drawing with triangles and without fragment shader
// TODO: Port the stage area from the other project to this game.
// TODO: Add text rendering.
// TODO: Create a simple UI for the terminal.
// ============

static void sdl_die(const char* message) {
    fprintf(stderr, "%s: %s\n", message, SDL_GetError());
    exit(2);
}

void init_screen() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0) {
        sdl_die("Couldn't initialize SDL");

        return;
    }

    SDL_GL_LoadLibrary(nullptr); // Default OpenGL is fine.

    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    // Request a debug context.
    SDL_GL_SetAttribute(
            SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG
    );

    window = SDL_CreateWindow(
            "Rule The City",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            Screen::WIDTH, Screen::HEIGHT, SDL_WINDOW_OPENGL
    );

    if (window == nullptr) {
        sdl_die("Couldn't set video mode");

        return;
    }

    main_context = SDL_GL_CreateContext(window);
    if (main_context == nullptr) {
        sdl_die("Failed to create OpenGL context");

        return;
    }


    printf("\n");
}

void postinit_screen() {
    int maj;
    int min;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &maj);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &min);
    printf("SDL2 Context     : %d.%d\n", maj, min);

    glGetIntegerv(GL_MAJOR_VERSION, &maj);
    glGetIntegerv(GL_MINOR_VERSION, &min);
    printf("GLAD Context     : %d.%d\n", maj, min);


    SDL_GL_SetSwapInterval(1); // V-Sync

    printf("\n");
}

void destroy_screen() {
    SDL_Quit();
}

int main(int argc, char* args[]) {
    init_screen();

    Renderer renderer;
    renderer.init(SDL_GL_GetProcAddress);

    postinit_screen();

    Texture fill_cell = Texture::load("texture/fill_cell.png");
    Texture empty_cell = Texture::load("texture/empty_cell.png");
    Texture stage_border = Texture::load("texture/stage_border.png");

    SDL_Event event;
    bool quit = false;

    /**
                glm::vec3{-1.0F, 1.0F, 0.0F},       // BOTTOM LEFT
                glm::vec3{1.0F, 1.0F, 0.0F},        // BOTTOM RIGHT
                glm::vec3{-1.0F, -1.0F, 0.0F},      // TOP LEFT
                glm::vec3{1.0, -1.0F, 0.0F}         // TOP RIGHT

     */

    Shape quad = ShapeGenerator::generate_quad();
    Shape triangle = ShapeGenerator::generate_triangle();

    ShaderInfo basic_shader{
            ShaderType::FILLED_QUAD,
            {}
    };

    while (!quit) {
        // Event
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }

            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        quit = true;
                        break;
                    default:
                        break;
                }
            }
        }

        // Update

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw
        renderer.batch_begin();
        renderer.draw(quad, basic_shader, glm::vec2{0.0F, 0.0F}, 0.0F, glm::vec2{64.0F, 64.0F}, {1.0F, 0.0F, 0.0F, 1.0F}, empty_cell);
        renderer.draw(quad, basic_shader, glm::vec2{65.0F, 65.0F}, 0.0F, glm::vec2{64.0F, 64.0F}, {1.0F, 0.0F, 0.0F, 1.0F}, fill_cell);
        renderer.draw(quad, basic_shader, glm::vec2{130.0F, 130.0F}, 0.0F, glm::vec2{400.0F, 100.0F}, {1.0F, 1.0F, 1.0F, 1.0F}, stage_border);
        renderer.batch_end();

        SDL_GL_SwapWindow(window);
    }

    destroy_screen();

    return 0;
}
