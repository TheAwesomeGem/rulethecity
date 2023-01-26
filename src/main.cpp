#define GLM_FORCE_RADIANS 1
#define SDL_MAIN_HANDLED

#include <cstdio>
#include <SDL2/SDL.h>
#include <glad/glad.h>
#include "renderer/Renderer.h"
#include "renderer/Screen.h"
#include "renderer/RenderUtil.h"


// Globals
static SDL_Window* window = nullptr;
static SDL_GLContext main_context;
// =======

// TODO:
// TODO: Add bordered circle to the renderer
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

    Texture wall = Texture::load("texture/wall.jpg");
    Texture container = Texture::load("texture/container.jpg");

    SDL_Event event;
    bool quit = false;

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
        //        renderer.draw(DrawableQuad{glm::vec2{200.0F, 200.0F}, glm::vec2{100.0F, 100.0F}, glm::vec4{1.0, 1.0, 1.0, 1.0}}, wall);
        //        renderer.draw(DrawableQuad{glm::vec2{400.0F, 400.0F}, glm::vec2{100.0F, 100.0F}, glm::vec4{1.0, 0.0, 0.0, 1.0}});
        //        renderer.draw(DrawableQuad{glm::vec2{600.0F, 600.0F}, glm::vec2{100.0F, 100.0F}, glm::vec4{0.0, 0.0, 1.0, 1.0}});
        draw_border_rect_color(
                renderer,
                glm::vec2{200.0F, 200.0F},
                glm::vec2{300.0F, 300.0F},
                1.0F,
                glm::vec4{1.0F, 0.0F, 0.0F, 1.0F}
        );
        draw_solid_circle_color(renderer, glm::vec2{0.0F, 0.0F}, 20.0F, glm::vec4{0.0F, 1.0F, 0.0F, 1.0F});
        renderer.batch_end();

        SDL_GL_SwapWindow(window);
    }

    destroy_screen();

    return 0;
}