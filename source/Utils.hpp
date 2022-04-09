//
// Created by Noah Schonhorn on 4/22/21.
//

#ifndef NES_CPP_UTILS_H
#define NES_CPP_UTILS_H

#include "SDL.h"
#include <memory>

bool init(std::shared_ptr<SDL_Window>& window, std::shared_ptr<SDL_Renderer>& renderer, int screenWidth, int screenHeight);

#endif //NES_CPP_UTILS_H
