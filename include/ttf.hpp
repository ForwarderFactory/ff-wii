#pragma once

#include <grrlib.h>
#include <array>
#include <stdexcept>
#include <memory>

namespace ff::ttf {
    struct TextParameters {
        int x{};
        int y{};
        int size{72};
        uint32_t color{0xFFFFFFFF};
        std::string text = "Hello World!";
    };

    template <typename T, std::size_t U>
    class TextHandler {
        GRRLIB_ttfFont* font{};
    public:
        explicit TextHandler(const std::array<T, U>& data) {
            font = GRRLIB_LoadTTF(data.data(), data.size());
            if (!font) {
                throw std::runtime_error("Failed to load font");
            }

            GRRLIB_Settings.antialias = true;
        }
        void draw(const TextParameters& params) const {
            if (!font) {
                throw std::runtime_error("Font not loaded");
            }
            if (params.size <= 0) {
                throw std::runtime_error("Invalid font size");
            }
            if (params.text.empty()) {
                throw std::runtime_error("Empty text");
            }
            GRRLIB_PrintfTTF(params.x, params.y, font, params.text.c_str(), params.size, params.color);
        }
        ~TextHandler() noexcept {
            if (font) {
                GRRLIB_FreeTTF(font);
            }
        }
    };
}