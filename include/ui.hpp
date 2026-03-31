#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <string>

#include "image_processing.hpp"

namespace proj1
{
    namespace ui
    {

        constexpr int kPanelWidth = 520;
        constexpr int kPanelHeight = 720;

        enum class ButtonVisualState
        {
            Normal,
            Hover,
            Pressed
        };

        struct Button
        {
            SDL_FRect rect{};
            ButtonVisualState state = ButtonVisualState::Normal;
            std::string label;
        };

        bool pointInRect(float x, float y, const SDL_FRect &rect);
        bool renderText(SDL_Renderer *renderer,
                        TTF_Font *font,
                        const std::string &text,
                        int x,
                        int y,
                        SDL_Color color,
                        bool centered = false,
                        int wrapWidth = 0);
        void drawButton(SDL_Renderer *renderer, TTF_Font *font, const Button &button);
        void drawHistogram(SDL_Renderer *renderer, const Histogram &histogram, const SDL_FRect &plotArea);

    } // namespace ui
} // namespace proj1
