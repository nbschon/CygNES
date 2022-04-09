//
// Created by Noah Schonhorn on 4/22/21.
//

#include "utils.hpp"

bool init(std::shared_ptr<SDL_Window>& window, std::shared_ptr<SDL_Renderer>& renderer, int screenWidth, int screenHeight)
{
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL Error %s\n", SDL_GetError());
        success = false;
    }
    else
    {
        //Set mTexture filtering to linear
        if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"))
        {
            printf("Warning: Linear texture filtering not enabled!");
        }

#ifdef WIN32
        if (!SDL_SetHint(SDL_HINT_RENDER_DRIVER, "directx"))
        {
            printf("Warning: Direct3D not being used!");
        }
#elif __APPLE__
        if (!SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl"))
        {
            printf("Warning: OpenGL not being used!");
        }
#endif

        //Create window
        window = std::shared_ptr<SDL_Window>(SDL_CreateWindow("nes_cpp", SDL_WINDOWPOS_UNDEFINED_DISPLAY(1), SDL_WINDOWPOS_UNDEFINED, screenWidth,
                                   screenHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE), SDL_DestroyWindow);
        if (window == nullptr)
        {
            printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            //Create renderer for window
            renderer = std::shared_ptr<SDL_Renderer>(SDL_CreateRenderer(
                    &*window, -1, SDL_RENDERER_ACCELERATED /*| SDL_RENDERER_PRESENTVSYNC*/), SDL_DestroyRenderer);
            if (renderer == nullptr)
            {
                printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
                success = false;
            }
            else
            {
                //Initialize renderer color
                SDL_SetRenderDrawColor(&*renderer, 0xFF, 0xFF, 0xFF, 0xFF);

                SDL_SetWindowMinimumSize(&*window, screenWidth, screenHeight);

//                //Initialize PNG loading
//                int imgFlags = IMG_INIT_PNG;
//                if (!(IMG_Init(imgFlags) & imgFlags))
//                {
//                    printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
//                    success = false;
//                }
            }
        }
    }

    return success;
}

void close(SDL_Window** window, SDL_Renderer** renderer)
{
    *window = nullptr;
    *renderer = nullptr;

    SDL_Quit();
    //IMG_Quit();
}