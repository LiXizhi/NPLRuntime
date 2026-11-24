#pragma once

#include "MidiSequence.h"

namespace ParaEngine {
namespace Midi {
constexpr unsigned SampleRate = 48000;
bool Open();
bool PlayMessage(uint32_t packed);
bool PlayFile(std::vector<Event> events, bool loop);
void StopFile(bool immediate);
void SetGain(float left, float right);
void Close();
}
}
