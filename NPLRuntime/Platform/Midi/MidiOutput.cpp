#include "MidiOutput.h"

#if defined(HARMONY_OS)
#include <ohaudio/native_audiostreambuilder.h>
#include <ohaudio/native_audiorenderer.h>
#elif defined(__linux__) && !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)
#include <SDL.h>
#else
#define SOKOL_AUDIO_IMPL
#if defined(__ANDROID__)
#define SAUDIO_ANDROID_SLES
#endif
// The host also uses this session for audio recording.
#define SAUDIO_IOS_EXTERNAL_AUDIO_SESSION
#define SAUDIO_OSX_USE_SYSTEM_HEADERS
#include "../../externals/midi/sokol_audio.h"
#endif

namespace ParaEngine {
namespace Midi {
#if defined(HARMONY_OS)
static OH_AudioRenderer* device = nullptr;
static RenderCallback callback;

bool OpenOutput(RenderCallback render, int sampleRate) {
    OH_AudioStreamBuilder* builder = nullptr;
    if (OH_AudioStreamBuilder_Create(&builder, AUDIOSTREAM_TYPE_RENDERER) != AUDIOSTREAM_SUCCESS) return false;
    callback = render;
    OH_AudioRenderer_Callbacks callbacks{};
    callbacks.OH_AudioRenderer_OnWriteData = [](OH_AudioRenderer*, void*, void* buffer, int32_t bytes) -> int32_t {
        callback(static_cast<float*>(buffer), bytes / (2 * sizeof(float)), 2);
        return 0;
    };
    bool ok = OH_AudioStreamBuilder_SetSamplingRate(builder, sampleRate) == AUDIOSTREAM_SUCCESS
        && OH_AudioStreamBuilder_SetChannelCount(builder, 2) == AUDIOSTREAM_SUCCESS
        && OH_AudioStreamBuilder_SetSampleFormat(builder, AUDIOSTREAM_SAMPLE_F32LE) == AUDIOSTREAM_SUCCESS
        && OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, nullptr) == AUDIOSTREAM_SUCCESS
        && OH_AudioStreamBuilder_GenerateRenderer(builder, &device) == AUDIOSTREAM_SUCCESS;
    OH_AudioStreamBuilder_Destroy(builder);
    if (ok && OH_AudioRenderer_Start(device) == AUDIOSTREAM_SUCCESS) return true;
    if (device) OH_AudioRenderer_Release(device);
    device = nullptr;
    return false;
}

void CloseOutput() {
    OH_AudioRenderer_Stop(device);
    OH_AudioRenderer_Release(device);
    device = nullptr;
}
#elif defined(__linux__) && !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)
static SDL_AudioDeviceID device;
static RenderCallback callback;

bool OpenOutput(RenderCallback render, int sampleRate) {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) return false;
    SDL_AudioSpec spec{};
    spec.freq = sampleRate;
    spec.channels = 2;
    spec.format = AUDIO_F32SYS;
    spec.samples = 512;
    callback = render;
    spec.callback = [](void*, Uint8* stream, int bytes) {
        callback(reinterpret_cast<float*>(stream), bytes / (2 * sizeof(float)), 2);
    };
    device = SDL_OpenAudioDevice(nullptr, 0, &spec, nullptr, 0);
    if (!device) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return false;
    }
    SDL_PauseAudioDevice(device, 0);
    return true;
}

void CloseOutput() {
    SDL_CloseAudioDevice(device);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}
#else
bool OpenOutput(RenderCallback render, int sampleRate) {
    saudio_desc desc{};
    desc.sample_rate = sampleRate;
    desc.num_channels = 2;
    desc.buffer_frames = 512;
    desc.stream_cb = render;
    saudio_setup(&desc);
    return saudio_isvalid();
}

void CloseOutput() {
    saudio_shutdown();
}
#endif
}
}
