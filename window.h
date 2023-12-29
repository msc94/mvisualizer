#pragma once

#include "capture.h"

#include <SDL2/SDL.h>
#include <boost/noncopyable.hpp>

struct WindowRenderingContext {
    std::vector<Channel> data;
};

class Window : private boost::noncopyable {
  public:
    Window(int num_bins);
    ~Window();

    void update(const std::vector<float> &values);
    bool render();

  private:
    int width_{640};
    int height_{480};
    SDL_Window *window_;
    SDL_Renderer *renderer_;

    std::vector<float> data_;
};
