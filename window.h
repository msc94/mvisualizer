#pragma once

#include "capture.h"

#include <SDL2/SDL.h>
#include <boost/noncopyable.hpp>

struct WindowRenderingContext {
    std::vector<Channel> data;
};

class Window : private boost::noncopyable {
  public:
    Window();
    ~Window();

    bool render(const std::vector<float> &data);

  private:
    const int width_{640};
    const int height_{480};

    SDL_Window *window_;
    SDL_Renderer *renderer_;
};
