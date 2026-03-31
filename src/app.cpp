#include "app.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace proj1
{

    void WindowDeleter::operator()(SDL_Window *window) const
    {
        if (window)
        {
            SDL_DestroyWindow(window);
        }
    }

    void RendererDeleter::operator()(SDL_Renderer *renderer) const
    {
        if (renderer)
        {
            SDL_DestroyRenderer(renderer);
        }
    }

    void TextureDeleter::operator()(SDL_Texture *texture) const
    {
        if (texture)
        {
            SDL_DestroyTexture(texture);
        }
    }

    void FontDeleter::operator()(TTF_Font *font) const
    {
        if (font)
        {
            TTF_CloseFont(font);
        }
    }

    Application::Application(std::string imagePath) : imagePath_(std::move(imagePath))
    {
        toggleButton_.rect = SDL_FRect{30.0f, 620.0f, 460.0f, 56.0f};
        toggleButton_.label = "Equalizar";
    }

    Application::~Application()
    {
        imageTexture_.reset();
        bodyFont_.reset();
        titleFont_.reset();
        histogramRenderer_.reset();
        mainRenderer_.reset();
        histogramWindow_.reset();
        mainWindow_.reset();
        equalizedSurface_.reset();
        grayscaleSurface_.reset();

        if (ttfInitialized_)
        {
            TTF_Quit();
        }
        if (librariesInitialized_)
        {
            SDL_Quit();
        }
    }

    int Application::run()
    {
        try
        {
            initializeLibraries();
            loadAndPrepareImages();
            createWindows();
            createRenderers();
            loadFonts();
            createOrUpdateTexture();
            updatePanelState();

            bool running = true;
            while (running)
            {
                SDL_Event event;
                while (SDL_PollEvent(&event))
                {
                    processEvent(event, running);
                }

                render();
                SDL_Delay(16);
            }

            return 0;
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Erro: " << ex.what() << '\n';
            return 1;
        }
    }

    void Application::initializeLibraries()
    {
        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            throw std::runtime_error(std::string("Falha ao inicializar SDL: ") + SDL_GetError());
        }
        librariesInitialized_ = true;

        if (!TTF_Init())
        {
            throw std::runtime_error(std::string("Falha ao inicializar SDL_ttf: ") + SDL_GetError());
        }
        ttfInitialized_ = true;
    }

    void Application::processEvent(const SDL_Event &event, bool &running)
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            running = false;
            break;
        case SDL_EVENT_KEY_DOWN:
            handleKeyDown(event, running);
            break;
        case SDL_EVENT_MOUSE_MOTION:
            handleMouseMotion(event);
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            handleMouseButtonDown(event);
            break;
        case SDL_EVENT_MOUSE_BUTTON_UP:
            handleMouseButtonUp(event);
            break;
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        case SDL_EVENT_WINDOW_MOVED:
            handleWindowEvent(event, running);
            break;
        default:
            break;
        }
    }

}