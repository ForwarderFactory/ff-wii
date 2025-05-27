#pragma once

#include <thread>
#include <mutex>
#include <grrlib.h>
#include <ogc/system.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <fat.h>
#include <asndlib.h>

namespace ff::sys {
    enum class ContextParams {
        Console = 1 << 0,
        Graphics = 1 << 1,
        StdioReport = 1 << 2,
        Filesystem = 1 << 3,
        Audio = 1 << 4,
        GenericInput = 1 << 5,
        IR = GenericInput | 1 << 6,
        ControllerInput = IR,
        Default = Graphics | ControllerInput | Filesystem | Audio,
    };

    inline ContextParams operator|(ContextParams lhs, ContextParams rhs) {
        using T = std::underlying_type_t<ContextParams>;
        return static_cast<ContextParams>(static_cast<T>(lhs) | static_cast<T>(rhs));
    }

    inline bool operator&(ContextParams lhs, ContextParams rhs) {
        using T = std::underlying_type_t<ContextParams>;
        return static_cast<T>(lhs) & static_cast<T>(rhs);
    }

    enum class CallbackType {
        OnFrame = 0,
        OnError = 1,
        OnExit = 2,
    };

    enum class ShutdownType {
        ReturnToMenu = 0,
        ReturnToLoader = 1,
        Shutdown = 2,
        Reboot = 3,
    };

    enum class ControllerButton {
        None = 0,
        ButtonA = WPAD_BUTTON_A,
        ButtonB = WPAD_BUTTON_B,
        ButtonMinus = WPAD_BUTTON_MINUS,
        ButtonPlus = WPAD_BUTTON_PLUS,
        ButtonOne = WPAD_BUTTON_1,
        ButtonTwo = WPAD_BUTTON_2,
        ButtonHome = WPAD_BUTTON_HOME,
        ButtonUp = WPAD_BUTTON_UP,
        ButtonDown = WPAD_BUTTON_DOWN,
        ButtonLeft = WPAD_BUTTON_LEFT,
        ButtonRight = WPAD_BUTTON_RIGHT,
    };

    static constexpr long long make_title_id(const std::string& ascii) noexcept {
        if (ascii.size() != 4) {
            return 0;
        }

        long long id = 0x0001000100000000LL;
        for (size_t i = 0; i < 4; ++i) {
            id |= static_cast<long long>(static_cast<unsigned char>(ascii[i])) << (8 * (3 - i));
        }
        return id;
    }

    enum class TitleID : long long {
        HAXX = make_title_id("HAXX"),
        JODI = make_title_id("JODI"),
        LULZ = make_title_id("LULZ"),
        OHBC = make_title_id("OHBC"),
    };

    class ScreenDimensions {
        int width{};
        int height{};

        ScreenDimensions(int w, int h) : width(w), height(h) {}
    public:
        [[nodiscard]] int get_width() const noexcept {
            return width;
        }
        [[nodiscard]] int get_height() const noexcept {
            return height;
        }

        friend class Context;
    };

    class IR {
        IR() = default;

        void assign_values(double x, double y, double angle, bool validity) {
            pointer_x = x;
            pointer_y = y;
            pointer_a = angle;
            pointer_validity = validity;
        }

        double pointer_x{320};
        double pointer_y{240};
        double pointer_a{0};
        bool pointer_validity{false};
    public:
        friend class Context;

        [[nodiscard]] double get_x() const noexcept {
            return pointer_x;
        }
        [[nodiscard]] double get_y() const noexcept {
            return pointer_y;
        }
        [[nodiscard]] double get_angle() const noexcept {
            return pointer_a;
        }
        [[nodiscard]] bool is_valid() const noexcept {
            return pointer_validity;
        }
    };

    // Buttons class is used to get the state of the controller buttons
    // It should NOT be static, because we only want the functions to work
    // if ContextParams::GenericInput is set
    struct Buttons final {
        [[nodiscard]] ControllerButton get_pressed() {
            static_cast<void*>(this);
            return static_cast<ControllerButton>(WPAD_ButtonsDown(0));
        }
        [[nodiscard]] ControllerButton get_held() {
            static_cast<void*>(this);
            return static_cast<ControllerButton>(WPAD_ButtonsHeld(0));
        }
        [[nodiscard]] ControllerButton get_up() {
            static_cast<void*>(this);
            return static_cast<ControllerButton>(WPAD_ButtonsUp(0));
        }

        friend class Context;
    private: // must not be initialized and used aside from by Context
        Buttons() = default;
    };

    class Context {
        ContextParams params{ContextParams::Default};
        Buttons buttons{};
        IR ir{};
        ScreenDimensions screen_dimensions{640, 480};

        std::function<void()> on_frame{};
        std::function<void(const std::string&)> on_error{};

        void raw_on_error(const std::string& str) const {
            on_error(str);
        }

        void raw_on_frame() const {
            try {
                if (this->on_frame) {
                    this->on_frame();
                }
            } catch (const std::exception& e) {
                this->raw_on_error(e.what());
            }
        }
    public:
        explicit Context(ContextParams params = ContextParams::Default,
            const std::function<void()>& frame = {},
            const std::function<void(const std::string&)>& err = {}
            ) : params(params), on_frame(frame), on_error(err) {
            SYS_STDIO_Report(params & ContextParams::StdioReport);

            if (params & ContextParams::Filesystem) {
                fatInitDefault();
            }

            if (params & ContextParams::Audio) {
                ASND_Init();
            }

            if ((params & ContextParams::Graphics))
                GRRLIB_Init();
            else
                VIDEO_Init();

            WII_Initialize();

            if (params & ContextParams::GenericInput) {
                WPAD_Init();
                WPAD_SetVRes(0, 640, 480);

                if (params & ContextParams::IR) {
                    WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
                }
            }

            if (!(params & ContextParams::Console)) {
                this->raw_on_frame();
                return;
            }

            const auto rmode = VIDEO_GetPreferredMode(nullptr);
            const auto xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

            // probably not even worth setting width and height
            // but let's do it anyway
            this->screen_dimensions = ScreenDimensions(rmode->fbWidth, rmode->efbHeight);

            console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);

            VIDEO_Configure(rmode);
            VIDEO_SetNextFramebuffer(xfb);
            VIDEO_SetBlack(FALSE);
            VIDEO_Flush();
            VIDEO_WaitVSync();

            if (rmode->viTVMode&VI_NON_INTERLACE) {
                VIDEO_WaitVSync();
            }

            this->raw_on_frame();
        }

        void load_title(TitleID id) const {
            static_cast<void>(this);
            WII_LaunchTitle(static_cast<long long>(id));
            throw std::runtime_error{"WII_LaunchTitle() failed; this likely means that the title is not installed on the console."};
        }

        [[nodiscard]] ScreenDimensions get_screen_dimensions() const noexcept {
            return screen_dimensions;
        }

        Buttons& get_buttons() {
            if (!(params & ContextParams::GenericInput)) {
                throw std::runtime_error{"ContextParams::GenericInput is not set, cannot get buttons"};
            }

            return buttons;
        }
        IR& get_ir() {
            if (!(params & ContextParams::GenericInput)) {
                throw std::runtime_error{"ContextParams::GenericInput is not set, cannot get IR"};
            }

            return ir;
        }
        void poll() {
            if (this->params & ContextParams::GenericInput) {
                WPAD_ScanPads();
            }
            if (this->params & ContextParams::IR) {
                ir_t r_ir{};
                WPAD_IR(0, &r_ir);
                this->ir.assign_values(r_ir.x, r_ir.y, r_ir.angle, r_ir.valid);
            }
        }

        // call after each frame change
        static void flush() noexcept {
            GRRLIB_Render();
            GRRLIB_FillScreen(0x000000FF);
        }

        Context(const Context&) = delete;
        Context& operator=(const Context&) = delete;
        Context(Context&&) = delete;
        Context& operator=(Context&&) = delete;

        static void exit(ShutdownType type = ShutdownType::ReturnToMenu) noexcept {
            switch (type) {
                case ShutdownType::ReturnToMenu:
                    SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
                    std::exit(EXIT_SUCCESS);
                case ShutdownType::ReturnToLoader:
                    for (const auto& it : { TitleID::HAXX, TitleID::JODI, TitleID::LULZ, TitleID::OHBC }) {
                        try {
                            WII_LaunchTitle(static_cast<long long>(it));
                        } catch (const std::runtime_error&) {
                        }
                    }

                    SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
                case ShutdownType::Shutdown:
                    SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
                    std::exit(EXIT_SUCCESS);
                case ShutdownType::Reboot:
                    SYS_ResetSystem(SYS_RESTART, 0, 0);
                    std::exit(EXIT_SUCCESS);
                default:
                    std::exit(EXIT_FAILURE);
            }
        }

        ~Context() {
            if (params & ContextParams::Graphics) {
                GRRLIB_Exit();
            }
            if (params & ContextParams::GenericInput) {
                WPAD_Shutdown();
            }
            if (params & ContextParams::Audio) {
                ASND_End();
            }
        }
    };
}