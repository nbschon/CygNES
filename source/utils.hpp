//
// Created by Noah Schonhorn on 4/22/21.
//

#ifndef CYGNES_UTILS_H
#define CYGNES_UTILS_H

#include "SDL.h"
#include <memory>

bool init(std::shared_ptr<SDL_Window>& window, std::shared_ptr<SDL_Renderer>& renderer, int screen_width, int screen_height);

#endif //CYGNES_UTILS_H
