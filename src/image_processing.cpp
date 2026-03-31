#include "image_processing.hpp"

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_surface.h>

#include <cmath>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace proj1
{

    void SurfaceDeleter::operator()(SDL_Surface *surface) const
    {
        if (surface)
        {
            SDL_DestroySurface(surface);
        }
    }

    SurfacePtr loadImageAsRGBA32(const std::string &imagePath)
    {
        SDL_Surface *loaded = IMG_Load(imagePath.c_str());
        if (!loaded)
        {
            throw std::runtime_error(std::string("Nao foi possivel carregar a imagem: ") + SDL_GetError());
        }

        SurfacePtr loadedPtr(loaded);
        SDL_Surface *converted = SDL_ConvertSurface(loaded, SDL_PIXELFORMAT_RGBA32);
        if (!converted)
        {
            throw std::runtime_error(std::string("Nao foi possivel converter a imagem para RGBA32: ") + SDL_GetError());
        }

        return SurfacePtr(converted);
    }
   
}
