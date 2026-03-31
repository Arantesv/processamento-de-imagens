#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <memory>
#include <string>

#include "image_processing.hpp"
#include "ui.hpp"

namespace proj1
{

    struct WindowDeleter
    {
        void operator()(SDL_Window *window) const;
    };

    struct RendererDeleter
    {
        void operator()(SDL_Renderer *renderer) const;
    };

    struct TextureDeleter
    {
        void operator()(SDL_Texture *texture) const;
    };

    struct FontDeleter
    {
        void operator()(TTF_Font *font) const;
    };

    using WindowPtr = std::unique_ptr<SDL_Window, WindowDeleter>;
    using RendererPtr = std::unique_ptr<SDL_Renderer, RendererDeleter>;
    using TexturePtr = std::unique_ptr<SDL_Texture, TextureDeleter>;
    using FontPtr = std::unique_ptr<TTF_Font, FontDeleter>;

    class Application
    {
    public:
        explicit Application(std::string imagePath);
        ~Application();

        int run();

    private:
        void initializeLibraries();
        void loadAndPrepareImages();
        void createWindows();
        void createRenderers();
        void loadFonts();
        void createOrUpdateTexture();
        void updatePanelState();

        void processEvent(const SDL_Event &event, bool &running);
        void handleKeyDown(const SDL_Event &event, bool &running);
        void handleMouseMotion(const SDL_Event &event);
        void handleMouseButtonDown(const SDL_Event &event);
        void handleMouseButtonUp(const SDL_Event &event);
        void handleWindowEvent(const SDL_Event &event, bool &running);

        void render();
        void renderImageWindow();
        void renderHistogramWindow();

        void toggleEqualization();
        void saveCurrentImage() const;
        void syncSecondaryWindow();

        SDL_Surface *currentSurface() const;
        std::string resolveFontPath() const;

        std::string imagePath_;
        std::string fontPath_;

        bool librariesInitialized_ = false;
        bool ttfInitialized_ = false;
        bool originalWasGrayscale_ = false;
        bool showingEqualized_ = false;
        bool togglePressed_ = false;
        bool secondaryIsPopup_ = false;

        WindowPtr mainWindow_;
        WindowPtr histogramWindow_;
        RendererPtr mainRenderer_;
        RendererPtr histogramRenderer_;
        TexturePtr imageTexture_;
        FontPtr bodyFont_;
        FontPtr titleFont_;

        SurfacePtr grayscaleSurface_;
        SurfacePtr equalizedSurface_;

        ImageAnalysis currentAnalysis_;
        ui::Button toggleButton_;

        SDL_WindowID mainWindowId_ = 0;
        SDL_WindowID histogramWindowId_ = 0;
    };

}