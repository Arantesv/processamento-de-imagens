#include "ui.hpp"

#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>

#include <algorithm>

namespace proj1
{
    namespace ui
    {

        bool pointInRect(float x, float y, const SDL_FRect &rect)
        {
            return x >= rect.x && y >= rect.y && x <= (rect.x + rect.w) && y <= (rect.y + rect.h);
        }

        bool renderText(SDL_Renderer *renderer,
                        TTF_Font *font,
                        const std::string &text,
                        int x,
                        int y,
                        SDL_Color color,
                        bool centered,
                        int wrapWidth)
        {
            if (!renderer || !font || text.empty())
            {
                return false;
            }

            SDL_Surface *textSurface = nullptr;
            if (wrapWidth > 0)
            {
                textSurface = TTF_RenderText_Blended_Wrapped(font, text.c_str(), text.size(), color, wrapWidth);
            }
            else
            {
                textSurface = TTF_RenderText_Blended(font, text.c_str(), text.size(), color);
            }

            if (!textSurface)
            {
                return false;
            }

            SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if (!textTexture)
            {
                SDL_DestroySurface(textSurface);
                return false;
            }

            SDL_FRect dest{};
            dest.w = static_cast<float>(textSurface->w);
            dest.h = static_cast<float>(textSurface->h);
            dest.x = static_cast<float>(x);
            dest.y = static_cast<float>(y);

            if (centered)
            {
                dest.x -= dest.w * 0.5f;
            }

            const bool ok = SDL_RenderTexture(renderer, textTexture, nullptr, &dest);

            SDL_DestroyTexture(textTexture);
            SDL_DestroySurface(textSurface);
            return ok;
        }

        void drawButton(SDL_Renderer *renderer, TTF_Font *font, const Button &button)
        {
            if (!renderer)
            {
                return;
            }

            Uint8 fillR = 45;
            Uint8 fillG = 125;
            Uint8 fillB = 246;

            switch (button.state)
            {
            case ButtonVisualState::Hover:
                fillR = 94;
                fillG = 168;
                fillB = 255;
                break;
            case ButtonVisualState::Pressed:
                fillR = 25;
                fillG = 85;
                fillB = 180;
                break;
            case ButtonVisualState::Normal:
            default:
                break;
            }

            SDL_SetRenderDrawColor(renderer, fillR, fillG, fillB, 255);
            SDL_RenderFillRect(renderer, &button.rect);

            SDL_SetRenderDrawColor(renderer, 18, 55, 110, 255);
            SDL_RenderRect(renderer, &button.rect);

            const int textX = static_cast<int>(button.rect.x + button.rect.w * 0.5f);
            const int textY = static_cast<int>(button.rect.y + button.rect.h * 0.5f - 10.0f);
            renderText(renderer, font, button.label, textX, textY, SDL_Color{255, 255, 255, 255}, true, 0);
        }

        void drawHistogram(SDL_Renderer *renderer, const Histogram &histogram, const SDL_FRect &plotArea)
        {
            if (!renderer)
            {
                return;
            }

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &plotArea);

            SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
            SDL_RenderRect(renderer, &plotArea);

            const int maxValue = *std::max_element(histogram.begin(), histogram.end());
            if (maxValue <= 0)
            {
                return;
            }

            const float barWidth = plotArea.w / 256.0f;

            SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
            for (int i = 0; i < 4; ++i)
            {
                const float y = plotArea.y + (plotArea.h / 4.0f) * static_cast<float>(i);
                SDL_RenderLine(renderer, plotArea.x, y, plotArea.x + plotArea.w, y);
            }

            SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
            for (int i = 0; i < 256; ++i)
            {
                const float normalized = static_cast<float>(histogram[i]) / static_cast<float>(maxValue);
                const float barHeight = normalized * plotArea.h;
                SDL_FRect bar{};
                bar.x = plotArea.x + static_cast<float>(i) * barWidth;
                bar.w = std::max(1.0f, barWidth);
                bar.h = barHeight;
                bar.y = plotArea.y + plotArea.h - barHeight;
                SDL_RenderFillRect(renderer, &bar);
            }
        }

    }
}