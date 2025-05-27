#pragma once

#include <array>
#include <stdexcept>
#include <mp3player.h>

namespace ff::audio {
    enum class AudioFormat : int {
        MP3 = 0,
        Auto = MP3,
    };

    template <typename T, std::size_t U>
    class AudioHandler {
        const std::array<T, U>& data;
        AudioFormat format{AudioFormat::Auto};
    public:
        explicit AudioHandler(const std::array<T, U>& data, AudioFormat format = AudioFormat::Auto) : data(data), format(format) {
            MP3Player_Init();
        }
        void play() {
            if (data.empty()) {
                throw std::runtime_error("No audio data provided");
            }

            if (format == AudioFormat::MP3) {
                MP3Player_PlayBuffer(data.data(), data.size(), nullptr);
            } else {
                throw std::runtime_error("Unsupported audio format");
            }
        }
        void pause(const bool p = ASND_Is_Paused()) {
            SND_Pause(p);
        }
        [[nodiscard]] bool is_paused() const {
            return ASND_Is_Paused();
        }
        [[nodiscard]] bool is_playing() const {
            return MP3Player_IsPlaying();
        }
        void stop() {
            MP3Player_Stop();
        }
        ~AudioHandler() = default;
    };
}