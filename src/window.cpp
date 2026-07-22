#include "window.h"

#include "common/log.h"

static constexpr int WINDOW_WIDTH  {800};
static constexpr int WINDOW_HEIGHT {600};

std::optional<Window> Window::create() {
    static constexpr auto WINDOW_FLAGS = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    SDL_Window* window = SDL_CreateWindow(
        "GBBuddy",
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        WINDOW_FLAGS
    );

    if (!window) {
        log::fatal("Failed to create SDL window. SDL_Error: {}", SDL_GetError());
        return std::nullopt;
    }

    return Window{window};
}
