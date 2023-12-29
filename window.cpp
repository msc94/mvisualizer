#include "window.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>

#include <spdlog/spdlog.h>
#include <stdexcept>

Window::Window() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0 || TTF_Init() != 0) {
        throw std::runtime_error("Failed initiailizing SDL");
    }

    window_ = SDL_CreateWindow("MVisualizer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width_, height_,
                               SDL_WINDOW_SHOWN);

    if (!window_) {
        throw std::runtime_error("Failed creating SDL window");
    }

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer_) {
        throw std::runtime_error("Failed creating SDL renderer");
    }
}

Window::~Window() {
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
}

bool Window::render(const std::vector<float> &data) {
    SDL_Event event{};

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return false;
        }
    }

    SDL_SetRenderDrawColor(renderer_, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer_);

    SDL_SetRenderDrawColor(renderer_, 0xFF, 0x00, 0x00, 0xFF);

    int bar_width = width_ / data.size();

    for (std::size_t i = 0; i < data.size(); i++) {
        float d = data[i] * 150.0f;

        SDL_Rect rect;
        rect.x = bar_width * i;
        rect.y = height_ - d;
        rect.w = bar_width;
        rect.h = d;

        SDL_RenderFillRect(renderer_, &rect);
    }

    SDL_RenderPresent(renderer_);
    return true;
}
