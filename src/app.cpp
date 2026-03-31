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

    void Application::loadFonts()
    {
        fontPath_ = resolveFontPath();
        if (fontPath_.empty())
        {
            throw std::runtime_error(
                "Nenhuma fonte TrueType foi encontrada. Defina a variavel de ambiente CV_FONT_PATH apontando para um arquivo .ttf/.ttc valido.");
        }

        titleFont_.reset(TTF_OpenFont(fontPath_.c_str(), 24.0f));
        bodyFont_.reset(TTF_OpenFont(fontPath_.c_str(), 17.0f));

        if (!titleFont_ || !bodyFont_)
        {
            throw std::runtime_error(std::string("Falha ao carregar a fonte '") + fontPath_ + "': " + SDL_GetError());
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

    void Application::updatePanelState()
    {
        currentAnalysis_ = analyzeGrayscaleSurface(currentSurface());
        toggleButton_.label = showingEqualized_ ? "Ver original" : "Equalizar";
    }

    void Application::handleKeyDown(const SDL_Event &event, bool &running)
    {
        const SDL_Keycode key = event.key.key;
        if (key == SDLK_ESCAPE)
        {
            running = false;
            return;
        }

        if (key == SDLK_S)
        {
            saveCurrentImage();
        }
    }

    void Application::handleMouseMotion(const SDL_Event &event)
    {
        if (event.motion.windowID != histogramWindowId_)
        {
            return;
        }

        const bool inside = ui::pointInRect(event.motion.x, event.motion.y, toggleButton_.rect);
        if (togglePressed_)
        {
            toggleButton_.state = inside ? ui::ButtonVisualState::Pressed : ui::ButtonVisualState::Normal;
        }
        else
        {
            toggleButton_.state = inside ? ui::ButtonVisualState::Hover : ui::ButtonVisualState::Normal;
        }
    }

    void Application::handleMouseButtonDown(const SDL_Event &event)
    {
        if (event.button.windowID != histogramWindowId_)
        {
            return;
        }
        if (event.button.button != SDL_BUTTON_LEFT)
        {
            return;
        }

        if (ui::pointInRect(event.button.x, event.button.y, toggleButton_.rect))
        {
            togglePressed_ = true;
            toggleButton_.state = ui::ButtonVisualState::Pressed;
        }
    }

    void Application::handleMouseButtonUp(const SDL_Event &event)
    {
        if (event.button.windowID != histogramWindowId_)
        {
            return;
        }
        if (event.button.button != SDL_BUTTON_LEFT)
        {
            return;
        }

        const bool inside = ui::pointInRect(event.button.x, event.button.y, toggleButton_.rect);
        const bool shouldToggle = togglePressed_ && inside;

        togglePressed_ = false;
        toggleButton_.state = inside ? ui::ButtonVisualState::Hover : ui::ButtonVisualState::Normal;

        if (shouldToggle)
        {
            toggleEqualization();
        }
    }

    void Application::handleWindowEvent(const SDL_Event &event, bool &running)
    {
        if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
        {
            if (event.window.windowID == mainWindowId_ || event.window.windowID == histogramWindowId_)
            {
                running = false;
            }
            return;
        }

        if (event.type == SDL_EVENT_WINDOW_MOVED && event.window.windowID == mainWindowId_)
        {
            syncSecondaryWindow();
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

    void Application::renderHistogramWindow()
    {
        SDL_SetRenderDrawColor(histogramRenderer_.get(), 238, 241, 245, 255);
        SDL_RenderClear(histogramRenderer_.get());

        const SDL_Color titleColor{20, 20, 20, 255};
        const SDL_Color bodyColor{35, 35, 35, 255};
        const SDL_Color infoColor{70, 70, 70, 255};

        ui::renderText(histogramRenderer_.get(), titleFont_.get(), "Processamento de Histograma", 260, 18, titleColor, true, 0);

        const SDL_FRect histogramRect{30.0f, 72.0f, 460.0f, 260.0f};
        ui::drawHistogram(histogramRenderer_.get(), currentAnalysis_.histogram, histogramRect);

        ui::renderText(histogramRenderer_.get(), bodyFont_.get(), "0", 30, 338, bodyColor, false, 0);
        ui::renderText(histogramRenderer_.get(), bodyFont_.get(), "255", 470, 338, bodyColor, false, 0);
        ui::renderText(histogramRenderer_.get(), bodyFont_.get(), "Intensidade", 260, 338, bodyColor, true, 0);

        const std::string imageType = std::string("Imagem original: ") + (originalWasGrayscale_ ? "ja estava em escala de cinza" : "colorida (convertida para escala de cinza)");
        const std::string meanText = "Media de intensidade: " + formatDouble(currentAnalysis_.mean) + " (" + currentAnalysis_.brightnessClass + ")";
        const std::string stddevText = "Desvio padrao: " + formatDouble(currentAnalysis_.stddev) + " (contraste " + currentAnalysis_.contrastClass + ")";
        const std::string stateText = std::string("Visualizacao atual: ") + (showingEqualized_ ? "histograma equalizado" : "imagem original em cinza");
        const std::string saveText = "Tecla S: salva a imagem atual em output_image.png";

        ui::renderText(histogramRenderer_.get(), bodyFont_.get(), imageType, 30, 382, bodyColor, false, 460);
        ui::renderText(histogramRenderer_.get(), bodyFont_.get(), meanText, 30, 432, bodyColor, false, 460);
        ui::renderText(histogramRenderer_.get(), bodyFont_.get(), stddevText, 30, 468, bodyColor, false, 460);
        ui::renderText(histogramRenderer_.get(), bodyFont_.get(), stateText, 30, 504, bodyColor, false, 460);
        ui::renderText(histogramRenderer_.get(), bodyFont_.get(), saveText, 30, 540, infoColor, false, 460);

        ui::drawButton(histogramRenderer_.get(), bodyFont_.get(), toggleButton_);

        SDL_RenderPresent(histogramRenderer_.get());
    }

    void Application::toggleEqualization()
    {
        showingEqualized_ = !showingEqualized_;
        createOrUpdateTexture();
        updatePanelState();
    }

    void Application::saveCurrentImage() const
    {
        if (!IMG_SavePNG(currentSurface(), "output_image.png"))
        {
            std::cerr << "Falha ao salvar output_image.png: " << SDL_GetError() << '\n';
            return;
        }

        std::cout << "Imagem salva com sucesso em output_image.png\n";
    }

    void Application::syncSecondaryWindow()
    {
        if (secondaryIsPopup_ || !mainWindow_ || !histogramWindow_)
        {
            return;
        }

        int mainX = 0;
        int mainY = 0;
        int mainW = 0;
        int mainH = 0;
        SDL_GetWindowPosition(mainWindow_.get(), &mainX, &mainY);
        SDL_GetWindowSize(mainWindow_.get(), &mainW, &mainH);

        const int gap = 24;
        SDL_SetWindowPosition(histogramWindow_.get(), mainX + mainW + gap, mainY);
    }

    SDL_Surface *Application::currentSurface() const
    {
        return showingEqualized_ ? equalizedSurface_.get() : grayscaleSurface_.get();
    }

    std::string Application::resolveFontPath() const
    {
        if (const char *envFont = std::getenv("CV_FONT_PATH"))
        {
            if (TTF_Font *font = TTF_OpenFont(envFont, 16.0f))
            {
                TTF_CloseFont(font);
                return envFont;
            }
        }

        std::vector<std::string> candidates = {
            "./font.ttf",
            "./assets/font.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf",
            "/usr/share/fonts/TTF/DejaVuSans.ttf",
            "C:/Windows/Fonts/arial.ttf",
            "C:/Windows/Fonts/segoeui.ttf",
            "/System/Library/Fonts/Supplemental/Arial.ttf",
            "/System/Library/Fonts/Supplemental/Arial Unicode.ttf",
            "/System/Library/Fonts/Supplemental/Helvetica.ttc"};

        for (const std::string &path : candidates)
        {
            if (TTF_Font *font = TTF_OpenFont(path.c_str(), 16.0f))
            {
                TTF_CloseFont(font);
                return path;
            }
        }

        return std::string();
    }

}