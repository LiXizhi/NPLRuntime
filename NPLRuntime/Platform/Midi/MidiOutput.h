#pragma once

namespace ParaEngine {
namespace Midi {
// CloseOutput waits for outstanding callbacks. Samples are interleaved stereo.
using RenderCallback = void (*)(float*, int, int);
bool OpenOutput(RenderCallback render, int sampleRate);
void CloseOutput();
}
}
