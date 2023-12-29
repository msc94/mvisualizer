#include "window.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>

#include <SDL2/SDL_video.h>
#include <cstddef>
#include <spdlog/spdlog.h>
#include <stdexcept>

Window::Window(int num_bins) : data_(num_bins) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0 || TTF_Init() != 0) {
        throw std::runtime_error("Failed initiailizing SDL");
    }

    window_ = SDL_CreateWindow("MVisualizer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width_, height_,
                               SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

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

void Window::update(const std::vector<float> &values) {
    if (values.size() != data_.size()) {
        throw std::runtime_error("values.size() != data_.size()");
    }

    constexpr float smoothness = 6;
    for (std::size_t i = 0; i < data_.size(); i++) {
        float delta = values[i] - data_[i];
        data_[i] += delta / smoothness;
    }
}

bool Window::render() {
    SDL_GetWindowSize(window_, &width_, &height_);

    SDL_Event event{};
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return false;
        }
    }

    SDL_SetRenderDrawColor(renderer_, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer_);
    SDL_SetRenderDrawColor(renderer_, 0xFF, 0x00, 0x00, 0xFF);

    float bar_width = static_cast<float>(width_) / data_.size();

    for (std::size_t i = 0; i < data_.size(); i++) {
        float d = data_[i] * height_;

        SDL_FRect rect;
        rect.x = bar_width * i;
        rect.y = height_ - d;
        rect.w = bar_width;
        rect.h = d;

        SDL_RenderFillRectF(renderer_, &rect);
    }

    SDL_RenderPresent(renderer_);
    return true;
}
