#include "MidiPlayback.h"
#include "MidiOutput.h"
#include <algorithm>
#include <array>
#include <memory>
#include <mutex>

#define OPL_IMPLEMENTATION
#include "../../externals/midi/opl.h"

namespace ParaEngine {
namespace Midi {
namespace {
struct Channel {
    int program = 0, volume = 127, expression = 127;
    bool sustain = false;
    std::array<bool, 128> deferred{};
    std::array<uint8_t, 128> velocity{};
};

class Synth {
public:
    bool init() {
        chip.reset(opl_create());
        return chip != nullptr;
    }
    void reset() {
        opl_reset(chip.get());
        channels = {};
        tail = 0;
    }
    void send(uint32_t message, bool packed = false) {
        int c = message & 15, a = (message >> 8) & 127, b = (message >> 16) & 127;
        Channel& ch = channels[c];
        switch (message & 0xf0) {
        case 0x90:
            if (b) {
                int program = packed && (message >> 24) ? (message >> 24) & 127 : ch.program;
                opl_midi_changeprog(chip.get(), c, program);
                ch.deferred[a] = false;
                ch.velocity[a] = b;
                opl_midi_noteon(chip.get(), c, a, b);
                if (ch.volume != 127 || ch.expression != 127) updateVolume(c);
                break;
            }
            // A note-on with zero velocity releases the key.
            // fall through
        case 0x80:
            if (ch.sustain) ch.deferred[a] = true;
            else opl_midi_noteoff(chip.get(), c, a);
            break;
        case 0xb0:
            if (a == 120 || a == 123) {
                ch.deferred.fill(false);
                opl_midi_controller(chip.get(), c, a, b);
            } else if (a == 64 || a == 121) {
                ch.sustain = a == 64 && b >= 64;
                if (!ch.sustain) {
                    for (int key = 0; key < 128; ++key)
                        if (ch.deferred[key]) opl_midi_noteoff(chip.get(), c, key);
                    ch.deferred.fill(false);
                }
                if (a == 121) {
                    ch.expression = 127;
                    opl_midi_pitchwheel(chip.get(), c, 0);
                    updateVolume(c);
                }
            } else if (a == 7 || a == 11) {
                if (a == 7) ch.volume = b;
                else ch.expression = b;
                updateVolume(c);
            }
            break;
        case 0xc0: ch.program = a; break;
        case 0xe0: opl_midi_pitchwheel(chip.get(), c, ((a | (b << 7)) - 8192) / 64); break;
        }
        tail = SampleRate;
    }
    void render(float* out, int count, const float* gain) {
        if (!chip || !tail) return;
        short pcm[512 * 2];
        opl_render(chip.get(), pcm, count, 1.0f);
        for (int i = 0; i < count * 2; ++i)
            out[i] += pcm[i] * gain[i & 1];
        for (const auto& voice : chip->voices2notes)
            if (voice.channel >= 0) return;
        tail = std::max(0, tail - count);
    }
private:
    void updateVolume(int c) {
        const auto& ch = channels[c];
        for (int i = 0; i < voicescount; ++i) {
            const auto& v = chip->voices2notes[i];
            if (v.channel == c)
                voicevolume(chip.get(), i, &chip->opl_gmtimbres[v.timbreid],
                    ch.velocity[v.note] * ch.volume * ch.expression / (127 * 127));
        }
    }
    std::unique_ptr<opl_t, decltype(&opl_destroy)> chip{nullptr, opl_destroy};
    std::array<Channel, 16> channels{};
    // Continue rendering the release envelope after the last voice is unassigned.
    int tail = 0;
};

struct Player {
    Synth live, song;
    std::vector<Event> events;
    size_t index = 0;
    uint64_t position = 0;
    bool loop = false;

    void render(float* out, int frames, const float* gain) {
        while (frames > 0) {
            while (index < events.size() && events[index].frame == position) {
                const uint32_t message = events[index++].message;
                if (message) song.send(message);
                else if (loop && position > 0) {
                    song.reset();
                    position = index = 0;
                } else {
                    for (int c = 0; c < 16; ++c) song.send(0x7bb0 | c);
                }
            }
            int count = std::min(frames, 512);
            if (index < events.size())
                count = static_cast<int>(std::min<uint64_t>(count, events[index].frame - position));
            std::fill_n(out, count * 2, 0.f);
            live.render(out, count, gain);
            song.render(out, count, gain);
            for (int i = 0; i < count * 2; ++i)
                out[i] = std::max(-1.f, std::min(1.f, out[i]));
            position += count;
            out += count * 2;
            frames -= count;
        }
    }
};

std::mutex controlMutex, mixMutex;
std::unique_ptr<Player> player;
float gain[2] = {1.f / 32768, 1.f / 32768};

void render(float* out, int frames, int channels) {
    std::lock_guard<std::mutex> lock(mixMutex);
    if (player && channels == 2) player->render(out, frames, gain);
    else std::fill_n(out, frames * channels, 0.f);
}

// Called under controlMutex. The device can invoke render synchronously during startup.
bool openLocked() {
    if (player) return true;
    std::unique_ptr<Player> next(new Player);
    if (!next->live.init() || !next->song.init() || !OpenOutput(render, SampleRate)) return false;
    std::lock_guard<std::mutex> lock(mixMutex);
    player = std::move(next);
    return true;
}
}

bool Open() {
    std::lock_guard<std::mutex> control(controlMutex);
    return openLocked();
}

bool PlayMessage(uint32_t packed) {
    const int status = packed & 255;
    if (status < 0x80 || status >= 0xf0) return false;
    std::lock_guard<std::mutex> control(controlMutex);
    if (!openLocked()) return false;
    std::lock_guard<std::mutex> mix(mixMutex);
    player->live.send(packed, true);
    return true;
}

bool PlayFile(std::vector<Event> events, bool loop) {
    if (events.empty()) return false;
    std::lock_guard<std::mutex> control(controlMutex);
    if (!openLocked()) return false;
    std::lock_guard<std::mutex> mix(mixMutex);
    player->song.reset();
    player->events.swap(events);
    player->index = player->position = 0;
    player->loop = loop;
    return true;
}

void StopFile(bool immediate) {
    std::lock_guard<std::mutex> lock(mixMutex);
    if (player) {
        player->loop = false;
        if (!immediate) return;
        std::vector<Event>().swap(player->events);
        player->index = 0;
        player->song.reset();
    }
}

void SetGain(float left, float right) {
    std::lock_guard<std::mutex> lock(mixMutex);
    gain[0] = left / 32768.f;
    gain[1] = right / 32768.f;
}

void Close() {
    std::lock_guard<std::mutex> control(controlMutex);
    if (!player) return;
    CloseOutput();
    std::lock_guard<std::mutex> mix(mixMutex);
    player.reset();
}
}
}
