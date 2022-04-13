//
// Created by Noah Schonhorn on 4/11/22.
//

#include "controller.hpp"

controller::controller(SDL_Event& input) : m_input(input)
{

}

void controller::get_input()
{
    m_status = 0;

    if (m_input.type == SDL_KEYDOWN /*and m_input.key.repeat == 0*/)
    {
        auto keys = m_input.key.keysym.sym;

        if (keys == m_btn_a)
        {
            m_a_pressed = true;
        }

        if (keys == m_btn_b)
        {
            m_b_pressed = true;
        }

        if (keys == m_btn_select)
        {
            m_select_pressed = true;
        }

        if (keys == m_btn_start)
        {
            m_start_pressed = true;
        }

        if (keys == m_btn_up)
        {
            m_up_pressed = true;
        }

        if (keys == m_btn_down)
        {
            m_down_pressed = true;
        }

        if (keys == m_btn_left)
        {
            m_left_pressed = true;
        }

        if (keys == m_btn_right)
        {
            m_right_pressed = true;
        }
    }
    else if (m_input.type == SDL_KEYUP /*and m_input.key.repeat == 0*/)
    {
        auto keys = m_input.key.keysym.sym;

        if (keys == m_btn_a)
        {
            m_a_pressed = false;
        }

        if (keys == m_btn_b)
        {
            m_b_pressed = false;
        }

        if (keys == m_btn_select)
        {
            m_select_pressed = false;
        }

        if (keys == m_btn_start)
        {
            m_start_pressed = false;
        }

        if (keys == m_btn_up)
        {
            m_up_pressed = false;
        }

        if (keys == m_btn_down)
        {
            m_down_pressed = false;
        }

        if (keys == m_btn_left)
        {
            m_left_pressed = false;
        }

        if (keys == m_btn_right)
        {
            m_right_pressed = false;
        }
    }
}

auto controller::get_status() -> uint8_t const
{
    m_status = static_cast<uint8_t>(m_a_pressed)
            | static_cast<uint8_t>(m_b_pressed) << 1
            | static_cast<uint8_t>(m_select_pressed) << 2
            | static_cast<uint8_t>(m_start_pressed) << 3
            | static_cast<uint8_t>(m_up_pressed) << 4
            | static_cast<uint8_t>(m_down_pressed) << 5
            | static_cast<uint8_t>(m_left_pressed) << 6
            | static_cast<uint8_t>(m_right_pressed) << 7;
    return m_status;
}
