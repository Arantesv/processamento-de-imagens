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

        void Application::loadAndPrepareImages()
    {
        SurfacePtr loaded = loadImageAsRGBA32(imagePath_);
        originalWasGrayscale_ = isAlreadyGrayscale(loaded.get());
        grayscaleSurface_ = buildGrayscaleSurface(loaded.get());
        equalizedSurface_ = buildEqualizedSurface(grayscaleSurface_.get());
        showingEqualized_ = false;
    }

    void Application::createWindows()
    {
        if (!grayscaleSurface_)
        {
            throw std::runtime_error("A imagem em escala de cinza nao foi preparada.");
        }

        mainWindow_.reset(SDL_CreateWindow("Proj1 - Imagem", grayscaleSurface_->w, grayscaleSurface_->h, 0));
        if (!mainWindow_)
        {
            throw std::runtime_error(std::string("Falha ao criar a janela principal: ") + SDL_GetError());
        }

        SDL_SetWindowPosition(mainWindow_.get(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

        histogramWindow_.reset(SDL_CreatePopupWindow(mainWindow_.get(),
                                                     grayscaleSurface_->w + 24,
                                                     0,
                                                     ui::kPanelWidth,
                                                     ui::kPanelHeight,
                                                     SDL_WINDOW_POPUP_MENU));

        if (histogramWindow_)
        {
            secondaryIsPopup_ = true;
        }
        else
        {
            histogramWindow_.reset(SDL_CreateWindow("Proj1 - Histograma",
                                                    ui::kPanelWidth,
                                                    ui::kPanelHeight,
                                                    0));
            if (!histogramWindow_)
            {
                throw std::runtime_error(std::string("Falha ao criar a janela secundaria: ") + SDL_GetError());
            }
            secondaryIsPopup_ = false;
            syncSecondaryWindow();
        }

        mainWindowId_ = SDL_GetWindowID(mainWindow_.get());
        histogramWindowId_ = SDL_GetWindowID(histogramWindow_.get());

        if (mainWindowId_ == 0 || histogramWindowId_ == 0)
        {
            throw std::runtime_error(std::string("Falha ao obter IDs das janelas: ") + SDL_GetError());
        }
    }

    void Application::createRenderers()
    {
        mainRenderer_.reset(SDL_CreateRenderer(mainWindow_.get(), nullptr));
        if (!mainRenderer_)
        {
            throw std::runtime_error(std::string("Falha ao criar renderer da janela principal: ") + SDL_GetError());
        }

        histogramRenderer_.reset(SDL_CreateRenderer(histogramWindow_.get(), nullptr));
        if (!histogramRenderer_)
        {
            throw std::runtime_error(std::string("Falha ao criar renderer da janela secundaria: ") + SDL_GetError());
        }
    }

    void Application::createOrUpdateTexture()
    {
        imageTexture_.reset(SDL_CreateTextureFromSurface(mainRenderer_.get(), currentSurface()));
        if (!imageTexture_)
        {
            throw std::runtime_error(std::string("Falha ao criar textura da imagem: ") + SDL_GetError());
        }
    }

    void Application::render()
    {
        renderImageWindow();
        renderHistogramWindow();
    }

    void Application::renderImageWindow()
    {
        SDL_SetRenderDrawColor(mainRenderer_.get(), 20, 20, 20, 255);
        SDL_RenderClear(mainRenderer_.get());
        SDL_RenderTexture(mainRenderer_.get(), imageTexture_.get(), nullptr, nullptr);
        SDL_RenderPresent(mainRenderer_.get());
    }


    SDL_Surface *Application::currentSurface() const
    {
        return showingEqualized_ ? equalizedSurface_.get() : grayscaleSurface_.get();
    }

}