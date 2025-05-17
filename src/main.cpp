#include <iostream>
#include <chrono>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <grrlib.h>
#include <ogc/system.h>
#include <nlohmann/json.hpp>

#include <img.hpp>
#include <ttf.hpp>
#include <font.ttf.hpp>
#include <pointer.png.hpp>

namespace ff {
    class Context {
        uint32_t buttons{};
        int width{};
        int height{};
        double pointer_x{320};
        double pointer_y{240};
        double pointer_a{0};
        bool pointer_validity{false};
        std::unique_ptr<ff::ttf::TextHandler<std::uint8_t, font_ttf.size()>> ttf_ctx{};
        std::unique_ptr<ff::img::ImageHandler<std::uint8_t, pointer_png.size()>> pointer_drawable{};
    public:
        explicit Context(bool console = false) {
            if (!console) {
                GRRLIB_Init();
                WPAD_Init();
                return;
            }

            VIDEO_Init();
            WPAD_Init();

            const auto rmode = VIDEO_GetPreferredMode(nullptr);
            const auto xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

            width = rmode->fbWidth;
            height = rmode->efbHeight;

            console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);

            VIDEO_Configure(rmode);
            VIDEO_SetNextFramebuffer(xfb);
            VIDEO_SetBlack(FALSE);
            VIDEO_Flush();
            VIDEO_WaitVSync();

            if(rmode->viTVMode&VI_NON_INTERLACE) {
                VIDEO_WaitVSync();
            }
        }

        static void flush() noexcept {
            GRRLIB_Render();
            GRRLIB_FillScreen(0x000000FF);
        }

        ~Context() {
            GRRLIB_Exit();
            WPAD_Shutdown();
        }

        void draw() {
            if (this->pointer_validity) {
                this->pointer_drawable->draw(img::ImageParameters{
                    .x = static_cast<int>(this->pointer_x - 48),
                    .y = static_cast<int>(this->pointer_y - 48),
                    .scale_x = 1,
                    .scale_y = 1,
                    .angle = static_cast<int>(pointer_a),
                });
            }

            nlohmann::json test_json{};
            test_json["xpos"] = this->pointer_x;
            test_json["ypos"] = this->pointer_y;
            test_json["angle"] = this->pointer_a;
            test_json["valid"] = this->pointer_validity;

            this->ttf_ctx->draw(ff::ttf::TextParameters{
                .x = 0,
                .y = 0,
                .size = 12,
                .color = 0xFFFFFFFF,
                .text = test_json.dump(2),
            });

            flush();
        }

        void main() {
            WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
            WPAD_SetVRes(0, 640, 480);

            this->ttf_ctx = std::make_unique<ff::ttf::TextHandler<std::uint8_t, font_ttf.size()>>(font_ttf);

            const auto get_cursor_pos = [this]() {
                ir_t ir{};
                WPAD_IR(0, &ir);
                if (ir.valid) {
                    this->pointer_x = ir.x;
                    this->pointer_y = ir.y;
                    this->pointer_a = ir.angle;
                }
                this->pointer_validity = ir.valid;
            };

            const auto get_buttons = [this]() {
                WPAD_ScanPads();
                this->buttons = WPAD_ButtonsDown(0);
            };

            this->pointer_drawable = std::make_unique<ff::img::ImageHandler<std::uint8_t, pointer_png.size()>>(pointer_png, ff::img::ImageFormat::PNG);
            if (!this->pointer_drawable) {
                throw std::runtime_error("Failed to load pointer image");
            }

            while (true) {
                get_cursor_pos();
                get_buttons();
                draw();

                if (this->buttons & WPAD_BUTTON_HOME) {
                    break;
                }

                // is it not a problem not to sleep at all?
            }
        }
    };

    std::shared_ptr<ff::Context> context = nullptr;
}

int main() {
    try {
        ff::context = std::make_shared<ff::Context>(false);
        ff::context->main();
    } catch (const std::exception&) {
        // TODO: print debug message on the screen
    }
}