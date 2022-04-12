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
            m_status |= (1 << 0);
            printf("A Button pressed\n");
        }

        if (keys == m_btn_b)
        {
            m_status |= (1 << 1);
            printf("B Button pressed\n");
        }

        if (keys == m_btn_select)
        {
            m_status |= (1 << 2);
            printf("Select Button pressed\n");
        }

        if (keys == m_btn_start)
        {
            m_status |= (1 << 3);
            printf("Start Button pressed\n");
        }

        if (keys == m_btn_up)
        {
            m_status |= (1 << 4);
            printf("Up Button pressed\n");
        }

        if (keys == m_btn_down)
        {
            m_status |= (1 << 5);
            printf("Down Button pressed\n");
        }

        if (keys == m_btn_left)
        {
            m_status |= (1 << 6);
            printf("Left Button pressed\n");
        }

        if (keys == m_btn_right)
        {
            m_status |= (1 << 7);
            printf("Right Button pressed\n");
        }
    }
}

auto controller::get_status() -> uint8_t const
{
    return m_status;
//    return (0x20);
}
