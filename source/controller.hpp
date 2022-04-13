//
// Created by Noah Schonhorn on 4/11/22.
//

#ifndef CYGNES_CONTROLLER_HPP
#define CYGNES_CONTROLLER_HPP

#include "SDL.h"

class controller
{
    uint8_t m_status;

    SDL_Event& m_input;

    // These should probably be rebindable in the future
    SDL_KeyCode m_btn_a = SDLK_s;
    SDL_KeyCode m_btn_b = SDLK_a;
    SDL_KeyCode m_btn_select = SDLK_RSHIFT;
    SDL_KeyCode m_btn_start = SDLK_RETURN;
    SDL_KeyCode m_btn_up = SDLK_UP;
    SDL_KeyCode m_btn_down = SDLK_DOWN;
    SDL_KeyCode m_btn_left = SDLK_LEFT;
    SDL_KeyCode m_btn_right = SDLK_RIGHT;

    bool m_a_pressed = false;
    bool m_b_pressed = false;
    bool m_select_pressed = false;
    bool m_start_pressed = false;
    bool m_up_pressed = false;
    bool m_down_pressed = false;
    bool m_left_pressed = false;
    bool m_right_pressed = false;

  public:
    controller(SDL_Event& input);
    auto get_input() -> void;
    auto get_status() -> uint8_t const;
};

#endif  // CYGNES_CONTROLLER_HPP
