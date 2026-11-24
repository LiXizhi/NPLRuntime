#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace ParaEngine {
namespace Midi {
struct Event {
    uint64_t frame;
    uint32_t message;
};

// Standard MIDI files, formats 0 and 1. A zero message marks the end of the song.
bool ReadSequence(const void* data, size_t size, unsigned sampleRate, std::vector<Event>& result);
}
}
