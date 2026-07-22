#include "renderer.h"

#include <cstring>

#include <SDL3/SDL.h>

#include "common/log.h"

std::optional<Renderer> Renderer::create(SDL_Window* window, int framebufferWidth, int framebufferHeight) {
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);

    if (!renderer) {
        log::fatal("Failed to create SDL renderer. SDL_Error: {}", SDL_GetError());
        return std::nullopt;
    }

    SDL_Texture* framebuffer = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        framebufferWidth,
        framebufferHeight
    );

    if (!framebuffer) {
        log::fatal("Failed to create SDL texture. SDL_Error: {}", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        return std::nullopt;
    }

    // Ensure pixels scale up cleanly using nearest neighbour
    if (!SDL_SetTextureScaleMode(framebuffer, SDL_SCALEMODE_NEAREST)) {
        log::err("Failed to set framebuffer scale mode. SDL_Error: {}", SDL_GetError());
    }

    const auto rendererName = SDL_GetRendererName(renderer);
    log::info("SDL Renderer: {}", rendererName);

    return Renderer{renderer, framebuffer, framebufferWidth, framebufferHeight};
}

void Renderer::beginFrame() {
    auto* renderer = sdlRenderer.get();

    SDL_SetRenderTarget(renderer, nullptr);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

void Renderer::drawPixelBuffer(std::span<const u32> pixels) {
    auto* renderer = sdlRenderer.get();
    auto* framebuffer = m_framebuffer.get();

    // Copy pixel data to framebuffer
    void* texturePixels;
    int pitch;

    if (!SDL_LockTexture(framebuffer, nullptr, &texturePixels, &pitch)) {
        log::err("Failed to lock framebuffer texture.");
        return;
    }

    const int srcPitch = m_framebufferWidth * static_cast<int>(sizeof(u32));
    auto dst = static_cast<u8*>(texturePixels);

    for (int row {0}; row < m_framebufferHeight; row++) {
        std::memcpy(dst + (row * pitch), pixels.data() + (row * m_framebufferWidth), srcPitch);
    }

    SDL_UnlockTexture(framebuffer);

    // Present the frame buffer using integer scaling to make it responsive to window size
    // Letterboxing will be applied to the window if the framebuffer doesn't scale cleanly
    SDL_SetRenderLogicalPresentation(renderer, m_framebufferWidth, m_framebufferHeight, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);

    // Render framebuffer to window
    SDL_SetRenderTarget(renderer, nullptr);
    SDL_RenderTexture(renderer, framebuffer, nullptr, nullptr);

    // Reset logical presentation so rendering of other elements works properly
    SDL_SetRenderLogicalPresentation(renderer, 0, 0, SDL_LOGICAL_PRESENTATION_DISABLED);
}

void Renderer::endFrame() {
    auto* renderer = sdlRenderer.get();

    SDL_RenderPresent(renderer);
}
