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
    namespace
    {

        class SurfaceLockGuard
        {
        public:
            explicit SurfaceLockGuard(SDL_Surface *surface) : surface_(surface), locked_(false)
            {
                if (surface_ && SDL_MUSTLOCK(surface_))
                {
                    if (!SDL_LockSurface(surface_))
                    {
                        throw std::runtime_error(std::string("Falha ao bloquear surface: ") + SDL_GetError());
                    }
                    locked_ = true;
                }
            }

            ~SurfaceLockGuard()
            {
                if (locked_)
                {
                    SDL_UnlockSurface(surface_);
                }
            }

        private:
            SDL_Surface *surface_;
            bool locked_;
        };

        struct PixelContext
        {
            const SDL_PixelFormatDetails *details = nullptr;
            SDL_Palette *palette = nullptr;
        };

        PixelContext getPixelContext(SDL_Surface *surface)
        {
            PixelContext ctx;
            ctx.details = SDL_GetPixelFormatDetails(surface->format);
            ctx.palette = SDL_GetSurfacePalette(surface);
            return ctx;
        }

        Uint32 *pixelAt(SDL_Surface *surface, int x, int y)
        {
            Uint8 *row = static_cast<Uint8 *>(surface->pixels) + static_cast<size_t>(y) * static_cast<size_t>(surface->pitch);
            return reinterpret_cast<Uint32 *>(row) + x;
        }

        Uint8 readGray(SDL_Surface *surface, int x, int y, const PixelContext &ctx)
        {
            Uint8 r = 0;
            Uint8 g = 0;
            Uint8 b = 0;
            Uint8 a = 0;
            SDL_GetRGBA(*pixelAt(surface, x, y), ctx.details, ctx.palette, &r, &g, &b, &a);
            return r;
        }

        void writeGray(SDL_Surface *surface, int x, int y, Uint8 value, Uint8 alpha, const PixelContext &ctx)
        {
            *pixelAt(surface, x, y) = SDL_MapRGBA(ctx.details, ctx.palette, value, value, value, alpha);
        }

        Uint8 computeLuminance(Uint8 r, Uint8 g, Uint8 b)
        {
            const double luminance = 0.2125 * static_cast<double>(r) +
                                     0.7154 * static_cast<double>(g) +
                                     0.0721 * static_cast<double>(b);
            const long rounded = std::lround(luminance);
            if (rounded < 0)
            {
                return 0;
            }
            if (rounded > 255)
            {
                return 255;
            }
            return static_cast<Uint8>(rounded);
        }

        std::string classifyBrightness(double mean)
        {
            if (mean < 85.0)
            {
                return "escura";
            }
            if (mean > 170.0)
            {
                return "clara";
            }
            return "media";
        }

        std::string classifyContrast(double stddev)
        {
            if (stddev < 42.5)
            {
                return "baixo";
            }
            if (stddev > 85.0)
            {
                return "alto";
            }
            return "medio";
        }

    }

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

    bool isAlreadyGrayscale(SDL_Surface *rgbaSurface)
    {
        if (!rgbaSurface)
        {
            throw std::runtime_error("Surface invalida ao verificar escala de cinza.");
        }

        SurfaceLockGuard lock(rgbaSurface);
        const PixelContext ctx = getPixelContext(rgbaSurface);

        for (int y = 0; y < rgbaSurface->h; ++y)
        {
            for (int x = 0; x < rgbaSurface->w; ++x)
            {
                Uint8 r = 0;
                Uint8 g = 0;
                Uint8 b = 0;
                Uint8 a = 0;
                SDL_GetRGBA(*pixelAt(rgbaSurface, x, y), ctx.details, ctx.palette, &r, &g, &b, &a);
                if (!(r == g && g == b))
                {
                    return false;
                }
            }
        }

        return true;
    }

    SurfacePtr buildGrayscaleSurface(SDL_Surface *rgbaSurface)
    {
        if (!rgbaSurface)
        {
            throw std::runtime_error("Surface invalida ao converter para escala de cinza.");
        }

        SDL_Surface *clone = SDL_DuplicateSurface(rgbaSurface);
        if (!clone)
        {
            throw std::runtime_error(std::string("Nao foi possivel duplicar a imagem: ") + SDL_GetError());
        }

        SurfacePtr grayscale(clone);
        SurfaceLockGuard sourceLock(rgbaSurface);
        SurfaceLockGuard targetLock(grayscale.get());

        const PixelContext sourceCtx = getPixelContext(rgbaSurface);
        const PixelContext targetCtx = getPixelContext(grayscale.get());

        for (int y = 0; y < rgbaSurface->h; ++y)
        {
            for (int x = 0; x < rgbaSurface->w; ++x)
            {
                Uint8 r = 0;
                Uint8 g = 0;
                Uint8 b = 0;
                Uint8 a = 0;
                SDL_GetRGBA(*pixelAt(rgbaSurface, x, y), sourceCtx.details, sourceCtx.palette, &r, &g, &b, &a);
                const Uint8 gray = computeLuminance(r, g, b);
                writeGray(grayscale.get(), x, y, gray, a, targetCtx);
            }
        }

        return grayscale;
    }

    ImageAnalysis analyzeGrayscaleSurface(SDL_Surface *grayscaleSurface)
    {
        if (!grayscaleSurface)
        {
            throw std::runtime_error("Surface invalida ao analisar histograma.");
        }

        ImageAnalysis analysis;
        const double totalPixels = static_cast<double>(grayscaleSurface->w) * static_cast<double>(grayscaleSurface->h);

        SurfaceLockGuard lock(grayscaleSurface);
        const PixelContext ctx = getPixelContext(grayscaleSurface);

        for (int y = 0; y < grayscaleSurface->h; ++y)
        {
            for (int x = 0; x < grayscaleSurface->w; ++x)
            {
                const Uint8 gray = readGray(grayscaleSurface, x, y, ctx);
                ++analysis.histogram[gray];
            }
        }

        double weightedSum = 0.0;
        for (int intensity = 0; intensity < 256; ++intensity)
        {
            weightedSum += static_cast<double>(intensity) * static_cast<double>(analysis.histogram[intensity]);
        }
        analysis.mean = weightedSum / totalPixels;

        double variance = 0.0;
        for (int intensity = 0; intensity < 256; ++intensity)
        {
            const double delta = static_cast<double>(intensity) - analysis.mean;
            variance += delta * delta * static_cast<double>(analysis.histogram[intensity]);
        }
        variance /= totalPixels;
        analysis.stddev = std::sqrt(variance);

        analysis.brightnessClass = classifyBrightness(analysis.mean);
        analysis.contrastClass = classifyContrast(analysis.stddev);

        return analysis;
    }

    SurfacePtr buildEqualizedSurface(SDL_Surface *grayscaleSurface)
    {
        if (!grayscaleSurface)
        {
            throw std::runtime_error("Surface invalida ao equalizar histograma.");
        }

        const ImageAnalysis analysis = analyzeGrayscaleSurface(grayscaleSurface);
        const int totalPixels = grayscaleSurface->w * grayscaleSurface->h;

        std::vector<Uint8> mapping(256, 0);
        int cumulative = 0;
        for (int intensity = 0; intensity < 256; ++intensity)
        {
            cumulative += analysis.histogram[intensity];
            const double sk = 255.0 * static_cast<double>(cumulative) / static_cast<double>(totalPixels);
            mapping[intensity] = static_cast<Uint8>(std::lround(sk));
        }

        SDL_Surface *clone = SDL_DuplicateSurface(grayscaleSurface);
        if (!clone)
        {
            throw std::runtime_error(std::string("Nao foi possivel duplicar a imagem em tons de cinza: ") + SDL_GetError());
        }

        SurfacePtr equalized(clone);
        SurfaceLockGuard sourceLock(grayscaleSurface);
        SurfaceLockGuard targetLock(equalized.get());

        const PixelContext sourceCtx = getPixelContext(grayscaleSurface);
        const PixelContext targetCtx = getPixelContext(equalized.get());

        for (int y = 0; y < grayscaleSurface->h; ++y)
        {
            for (int x = 0; x < grayscaleSurface->w; ++x)
            {
                Uint8 r = 0;
                Uint8 g = 0;
                Uint8 b = 0;
                Uint8 a = 0;
                SDL_GetRGBA(*pixelAt(grayscaleSurface, x, y), sourceCtx.details, sourceCtx.palette, &r, &g, &b, &a);
                const Uint8 newGray = mapping[r];
                writeGray(equalized.get(), x, y, newGray, a, targetCtx);
            }
        }

        return equalized;
    }

    std::string formatDouble(double value, int precision)
    {
        std::ostringstream out;
        out << std::fixed << std::setprecision(precision) << value;
        return out.str();
    }

}
