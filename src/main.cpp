#include <iostream>
#include <thread>
#include <chrono>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <grrlib.h>
#include <img.jpg.hpp>

template <typename T, std::size_t U>
class ImageHandler {
    GRRLIB_texImg* img;
public:
    explicit ImageHandler(const std::array<T, U>& data) {
        img = GRRLIB_LoadTextureJPG(data.data());
        if (!img) {
            throw std::runtime_error("Failed to load image");
        }
    }
    void draw() const {
        if (!img) {
            throw std::runtime_error("Image not loaded");
        }
        GRRLIB_DrawImg(10, 50, img, 0, 1, 1, 0xFFFFFFFF);
        GRRLIB_Render();
    }
    ~ImageHandler() noexcept {
        if (img) {
            GRRLIB_FreeTexture(img);
        }
    }
};

void init() {
    VIDEO_Init();
    WPAD_Init();

    /* seemingly only needed for console?
    const auto rmode = VIDEO_GetPreferredMode(nullptr);
    const auto xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

    console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);

    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();

    if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
    */

    GRRLIB_Init();
}

int main() {
    init();

    const auto ptr = std::make_unique<ImageHandler<std::uint8_t, img_jpg.size()>>(img_jpg);
    ptr->draw();

    std::this_thread::sleep_for(std::chrono::seconds(10));
}