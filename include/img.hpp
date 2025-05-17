#pragma once

#include <grrlib.h>
#include <array>
#include <stdexcept>

namespace ff::img {
    enum class ImageFormat : int {
        PNG = 0,
        JPG = 1,
        TPL = 2,
        BMP = 3,
        Auto = 4,
    };
    struct ImageParameters {
        int x{};
        int y{};
        int scale_x{1};
        int scale_y{1};
        int angle{};
    };

    template <typename T, std::size_t U>
    class ImageHandler {
        GRRLIB_texImg* img{};
    public:
        explicit ImageHandler(const std::array<T, U>& data, ImageFormat format = ImageFormat::Auto) {
            if (format == ImageFormat::PNG) {
                img = GRRLIB_LoadTexturePNG(data.data());
            } else if (format == ImageFormat::JPG) {
                img = GRRLIB_LoadTextureJPG(data.data());
            } else if (format == ImageFormat::TPL) {
                img = GRRLIB_LoadTextureTPL(data.data(), data.size());
            } else if (format == ImageFormat::BMP) {
                img = GRRLIB_LoadTextureBMP(data.data());
            } else if (format == ImageFormat::Auto) {
                img = GRRLIB_LoadTexture(data.data());
            } else {
                throw std::runtime_error("Unsupported image format");
            }
            if (!img) {
                throw std::runtime_error("Failed to load image");
            }
        }
        void draw(const ImageParameters& params = {}) const {
            if (!img) {
                throw std::runtime_error("Image not loaded");
            }
            GRRLIB_DrawImg(params.x, params.y, img, params.angle, params.scale_x, params.scale_y, 0xFFFFFFFF);
        }
        ~ImageHandler() noexcept {
            if (img) {
                GRRLIB_FreeTexture(img);
            }
        }
    };
    class TexImageHandler {
        GRRLIB_texImg* img{};
    public:
        explicit TexImageHandler(GRRLIB_texImg* img) : img(img) {
            if (!img) {
                throw std::runtime_error("Failed to load image");
            }
        }
        void draw(const ImageParameters& params) const {
            if (!img) {
                throw std::runtime_error("Image not loaded");
            }
            GRRLIB_DrawImg(params.x, params.y, img, params.angle, params.scale_x, params.scale_y, 0xFFFFFFFF);
        }
        ~TexImageHandler() noexcept {
            if (img) {
                GRRLIB_FreeTexture(img);
            }
        }
    };
}