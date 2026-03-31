#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>

#include <array>
#include <memory>
#include <string>

namespace proj1
{

    struct SurfaceDeleter
    {
        void operator()(SDL_Surface *surface) const;
    };

    using SurfacePtr = std::unique_ptr<SDL_Surface, SurfaceDeleter>;
    using Histogram = std::array<int, 256>;

    struct ImageAnalysis
    {
        Histogram histogram{};
        double mean = 0.0;
        double stddev = 0.0;
        std::string brightnessClass;
        std::string contrastClass;
    };

    SurfacePtr loadImageAsRGBA32(const std::string &imagePath);
    bool isAlreadyGrayscale(SDL_Surface *rgbaSurface);
    SurfacePtr buildGrayscaleSurface(SDL_Surface *rgbaSurface);
    SurfacePtr buildEqualizedSurface(SDL_Surface *grayscaleSurface);
    ImageAnalysis analyzeGrayscaleSurface(SDL_Surface *grayscaleSurface);
    std::string formatDouble(double value, int precision = 2);

} // namespace proj1
