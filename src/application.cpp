#include "application.h"

#include <stdexcept>
#include <string>

#include <common/logger.h>

static constexpr int SCREEN_WIDTH  { 640 };
static constexpr int SCREEN_HEIGHT { 576 };

Application::~Application() {
    close();
}

void Application::init() {
    SDL_SetAppMetadata("GBBuddy", "0.1", "com.mattkolega.gbbuddy");

    if(!SDL_Init(SDL_INIT_VIDEO)) {
        throw std::runtime_error("SDL could not be initialised! SDL_Error: " + std::string(SDL_GetError()));
    }

    m_window = SDL_CreateWindow("GBBuddy", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (m_window == nullptr) {
        throw std::runtime_error("SDL window could not be created! SDL_Error: " + std::string(SDL_GetError()));
    }

    m_renderer = SDL_CreateRenderer(m_window, NULL);
    if (m_renderer == nullptr) {
        throw std::runtime_error("SDL rendering context could not be created! SDL_Error: " + std::string(SDL_GetError()));
    }

    Logger::info("SDL Renderer: {}", SDL_GetRendererName(m_renderer));

    m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_TARGET, SCREEN_HEIGHT, SCREEN_HEIGHT);
    if (m_texture == nullptr) {
        throw std::runtime_error("SDL texture could not be created! SDL_Error: " + std::string(SDL_GetError()));
    }
}

void Application::run() {
    while (!m_quit) {
        handleEvents();
        updateDisplay();
    }
}

void Application::handleEvents() {
    while (SDL_PollEvent(&m_event)) {
        switch (m_event.type) {
            case SDL_EVENT_KEY_DOWN:
                switch (m_event.key.scancode) {
                    case SDL_SCANCODE_ESCAPE:
                        m_quit = true;
                        break;
                    default:
                        break;
                }
                break;
            case SDL_EVENT_QUIT:
                m_quit = true;
                break;
        }
	}
}

void Application::updateDisplay() {
    // Render to texture
    SDL_SetRenderTarget(m_renderer, m_texture);
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);

    // Render to window
    SDL_SetRenderTarget(m_renderer, NULL);
    SDL_RenderTexture(m_renderer, m_texture, NULL, NULL);
    SDL_RenderPresent(m_renderer);
}

void Application::close() {
    SDL_DestroyTexture(m_texture);
    m_texture = nullptr;

    SDL_DestroyRenderer(m_renderer);
    m_renderer = nullptr;

    SDL_DestroyWindow(m_window);
    m_window = nullptr;

    SDL_Quit();
}