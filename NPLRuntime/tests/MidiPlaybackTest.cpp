// Exercise the actual render callback with a memory sink, including startup and teardown.
#include "../Platform/Midi/MidiPlayback.h"
#include "../Platform/Midi/MidiOutput.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>

using namespace ParaEngine::Midi;

static RenderCallback render;
static bool deviceAvailable = true;
#define CHECK(expr) do { if (!(expr)) { std::fprintf(stderr, "line %d: %s\n", __LINE__, #expr); std::exit(1); } } while (0)

bool ParaEngine::Midi::OpenOutput(RenderCallback callback, int rate)
{
    CHECK(rate == 48000 && !render);
    if (!deviceAvailable) return false;
    render = callback;
    float samples[32];
    render(samples, 16, 2); // Some native devices request buffers before OpenOutput returns.
    return true;
}

void ParaEngine::Midi::CloseOutput() { render = nullptr; }

static float peak(int frames)
{
    std::vector<float> samples(frames * 2);
    render(samples.data(), frames, 2);
    float maximum = 0;
    for (float sample : samples) maximum = std::max(maximum, std::abs(sample));
    return maximum;
}

int main()
{
    // Two tracks at 96 PPQN. The tempo doubles halfway through the held note.
    const uint8_t file[] = {
        'M','T','h','d',0,0,0,6,0,1,0,2,0,96,
        'M','T','r','k',0,0,0,11, 96,0xff,0x51,3,0x03,0xd0,0x90, 96,0xff,0x2f,0,
        'M','T','r','k',0,0,0,12, 0,0x91,69,100, 0x81,0x40,69,0, 0,0xff,0x2f,0
    };
    std::vector<Event> events;
    CHECK(ReadSequence(file, sizeof file, SampleRate, events));
    CHECK(events.size() == 3 && events[0].message == 0x644591);
    CHECK(events[1].frame == 36000 && events[1].message == 0x4591);
    CHECK(events.back().frame == 36000 && events.back().message == 0);
    for (size_t i = 0; i < sizeof file; ++i) CHECK(!ReadSequence(file, i, SampleRate, events));
    CHECK(events.size() == 3); // Failed parses leave the previous sequence intact.
    auto invalid = std::vector<uint8_t>(file, file + sizeof file);
    invalid[12] = invalid[13] = 0;
    CHECK(!ReadSequence(invalid.data(), invalid.size(), SampleRate, events));

    auto smpte = std::vector<uint8_t>(file, file + sizeof file);
    smpte[12] = 0xe7; // 25 frames per second, 40 ticks per frame
    smpte[13] = 40;
    std::vector<Event> timed;
    CHECK(ReadSequence(smpte.data(), smpte.size(), SampleRate, timed));
    CHECK(timed.back().frame == 9216); // SMPTE timing ignores tempo events.

    deviceAvailable = false;
    CHECK(!PlayMessage(0x644590));
    deviceAvailable = true;
    CHECK(PlayMessage(0x644590));
    CHECK(peak(4096) > 0.001f);
    CHECK(PlayMessage(0x7bb1)); // Stopping channel 1 leaves channel 0 audible.
    CHECK(peak(4096) > 0.001f);
    SetGain(0, 0);
    CHECK(peak(512) == 0);
    SetGain(1, 1);
    CHECK(PlayMessage(0x7f40b0)); // Pedal down.
    CHECK(PlayMessage(0x004590)); // Zero-velocity note-on releases the key.
    CHECK(peak(2048) > 0.001f);
    CHECK(PlayMessage(0x0040b0)); // Pedal up releases deferred notes.
    peak(SampleRate + 512);
    CHECK(peak(512) == 0);

    CHECK(PlayFile(events, true));
    CHECK(peak(2048) > 0.001f);
    peak(36000 - 2048);
    CHECK(peak(2048) > 0.001f); // Replay begins at the end-of-track timestamp.
    StopFile(false);
    peak(36000 + SampleRate);
    CHECK(peak(512) == 0);
    CHECK(PlayFile(events, false));
    StopFile(true);
    CHECK(peak(512) == 0);

    Close();
    CHECK(!render);
    CHECK(PlayMessage(0x353c92));
    CHECK(peak(2048) > 0.001f);
    Close();
}
