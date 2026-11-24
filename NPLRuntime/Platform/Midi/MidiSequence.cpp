#include "MidiSequence.h"
#include <algorithm>
#include <cstring>
#include <limits>

namespace ParaEngine {
namespace Midi {
namespace {
struct Reader {
    const uint8_t* pos;
    const uint8_t* end;

    bool take(size_t count, const uint8_t*& bytes) {
        if (count > static_cast<size_t>(end - pos)) return false;
        bytes = pos;
        pos += count;
        return true;
    }

    bool variable(uint32_t& value) {
        value = 0;
        for (int i = 0; i < 4 && pos != end; ++i) {
            uint8_t byte = *pos++;
            value = (value << 7) | (byte & 127);
            if (!(byte & 128)) return true;
        }
        return false;
    }
};

uint32_t bigEndian(const uint8_t* bytes, int count) {
    uint32_t value = 0;
    while (count--) value = (value << 8) | *bytes++;
    return value;
}

bool readTrack(Reader track, std::vector<Event>& events, uint64_t& lastTick) {
    uint64_t tick = 0;
    uint8_t running = 0;
    while (track.pos < track.end) {
        uint32_t delta;
        if (!track.variable(delta) || track.pos == track.end) return false;
        tick += delta;
        uint8_t status = *track.pos;
        if (status >= 128) {
            ++track.pos;
            running = status < 0xf0 ? status : 0;
        } else {
            if (!running) return false;
            status = running;
        }
        const uint8_t* bytes;
        if (status < 0xf0) {
            const unsigned kind = status & 0xf0;
            const int length = (kind == 0xc0 || kind == 0xd0) ? 1 : 2;
            if (!track.take(length, bytes) || bytes[0] >= 128 || (length == 2 && bytes[1] >= 128)) return false;
            uint32_t message = status | (bytes[0] << 8);
            if (length == 2) message |= bytes[1] << 16;
            events.push_back({tick, message});
        } else {
            int type = -1;
            if (status == 0xff) {
                if (track.pos == track.end) return false;
                type = *track.pos++;
            } else if (status != 0xf0 && status != 0xf7) {
                return false;
            }
            uint32_t length;
            if (!track.variable(length) || !track.take(length, bytes)) return false;
            if (type == 0x51) {
                if (length != 3) return false;
                uint32_t tempo = bigEndian(bytes, 3);
                if (!tempo) return false;
                events.push_back({tick, (tempo << 8) | 0xff});
            }
            if (type == 0x2f) {
                if (length) return false;
                break;
            }
        }
    }
    lastTick = std::max(lastTick, tick);
    return true;
}
}

bool ReadSequence(const void* data, size_t size, unsigned sampleRate, std::vector<Event>& result) {
    if (!data || size < 14 || !sampleRate) return false;
    auto start = static_cast<const uint8_t*>(data);
    Reader file{start, start + size};
    const uint8_t* header;
    file.take(8, header);
    if (std::memcmp(header, "MThd", 4)) return false;
    unsigned headerSize = bigEndian(header + 4, 4);
    if (headerSize < 6 || !file.take(headerSize, header)) return false;
    unsigned format = bigEndian(header, 2), tracks = bigEndian(header + 2, 2);
    unsigned division = bigEndian(header + 4, 2);
    if (format > 1 || !tracks || (!format && tracks != 1) || !division) return false;

    double framesPerTick = 0.5 * sampleRate / division;
    bool smpte = (division & 0x8000) != 0;
    if (smpte) {
        int fps = 256 - (division >> 8);
        if ((fps != 24 && fps != 25 && fps != 29 && fps != 30) || !(division & 255)) return false;
        framesPerTick = sampleRate / ((fps == 29 ? 30000.0 / 1001.0 : fps) * (division & 255));
    }

    std::vector<Event> events;
    uint64_t lastTick = 0;
    while (tracks) {
        const uint8_t* bytes;
        if (!file.take(8, header) || !file.take(bigEndian(header + 4, 4), bytes)) return false;
        if (std::memcmp(header, "MTrk", 4)) continue; // skip unknown SMF chunks
        if (!readTrack({bytes, file.pos}, events, lastTick)) return false;
        --tracks;
    }
    std::stable_sort(events.begin(), events.end(), [](const Event& a, const Event& b) { return a.frame < b.frame; });
    events.push_back({lastTick, 0});
    uint64_t tick = 0;
    double frame = 0;
    size_t output = 0;
    for (auto event : events) {
        frame += (event.frame - tick) * framesPerTick;
        if (frame >= static_cast<double>(std::numeric_limits<uint64_t>::max())) return false;
        tick = event.frame;
        if ((event.message & 255) == 0xff) {
            if (!smpte) framesPerTick = (event.message >> 8) * (sampleRate / 1000000.0) / division;
        } else {
            events[output++] = {static_cast<uint64_t>(frame + 0.5), event.message};
        }
    }
    events.resize(output);
    result.swap(events);
    return true;
}
}
}
