#include <iostream>
#include <chrono>
#include <nlohmann/json.hpp>

#include <net.hpp>
#include <img.hpp>
#include <ttf.hpp>
#include <sys.hpp>
#include <font.ttf.hpp>
#include <pointer.png.hpp>

int main() {
    ff::sys::Context ctx{
        ff::sys::ContextParams::Graphics | ff::sys::ContextParams::ControllerInput | ff::sys::ContextParams::IR | ff::sys::ContextParams::Filesystem,
        [&ctx]() {
            ff::ttf::TextHandler<std::uint8_t, ::font_ttf.size()> ttf_ctx(::font_ttf);
            ff::img::ImageHandler<std::uint8_t, ::pointer_png.size()> pointer_drawable(::pointer_png, ff::img::ImageFormat::PNG);

            while (true) {
                ctx.poll();

                if (ctx.get_buttons().get_pressed() == ff::sys::ControllerButton::ButtonHome) {
                    ff::sys::Context::exit(ff::sys::ShutdownType::ReturnToMenu);
                    return EXIT_FAILURE; // unreachable anyway
                }

                pointer_drawable.draw(ff::img::ImageParameters{
                    .x = static_cast<int>(ctx.get_ir().get_x() - 48),
                    .y = static_cast<int>(ctx.get_ir().get_y() - 48),
                    .scale_x = 1,
                    .scale_y = 1,
                    .angle = static_cast<int>(ctx.get_ir().get_angle()),
                });

                ttf_ctx.draw(ff::ttf::TextParameters{
                    .x = 0,
                    .y = 0,
                    .size = 12,
                    .color = 0xFFFFFFFF,
                    .text = "Hello World!",
                });

                ff::sys::Context::flush();
            }
        },
        [](const std::string& err) {
            SYS_Report("Error: %s\n", err.c_str());
        }
    };
}