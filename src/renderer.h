#pragma once

#include <memory>
#include <optional>
#include <span>

#include <SDL3/SDL.h>

#include "common/types.h"

class Renderer {
private:
    struct RendererDeleter{
        void operator()(SDL_Renderer* r) const { SDL_DestroyRenderer(r); }
    };

    struct TextureDeleter {
        void operator()(SDL_Texture* t) const { SDL_DestroyTexture(t); }
    };

public:
    std::unique_ptr<SDL_Renderer, RendererDeleter> sdlRenderer;

    static std::optional<Renderer> create(SDL_Window* window, int framebufferWidth, int framebufferHeight);

    void beginFrame();
    void drawPixelBuffer(std::span<const u32> pixels);
    void endFrame();

private:
    explicit Renderer(SDL_Renderer* renderer, SDL_Texture* framebuffer, int framebufferWidth, int framebufferHeight)
        : sdlRenderer(renderer)
        , m_framebuffer(framebuffer)
        , m_framebufferWidth(framebufferWidth)
        , m_framebufferHeight(framebufferHeight) {}

    std::unique_ptr<SDL_Texture, TextureDeleter> m_framebuffer;

    int m_framebufferWidth{};
    int m_framebufferHeight{};
};
