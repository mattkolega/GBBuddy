#pragma once

#include <memory>
#include <optional>

#include <SDL3/SDL.h>

class Window {
public:
    static std::optional<Window> create();

    SDL_Window* handle() const { return m_window.get(); }

private:
    struct Deleter {
        void operator()(SDL_Window* w) const { SDL_DestroyWindow(w); }
    };

    explicit Window(SDL_Window* w) : m_window(w) {}

    std::unique_ptr<SDL_Window, Deleter> m_window;
};
