#include "cAudio.h"
#ifdef USE_MINI_MP3
#include "cMP3DecoderFactory.h"
#endif

#include <chrono>
#include <cmath>
#include <cstdio>
#include <memory>
#include <thread>
#include <vector>

using namespace cAudio;
using namespace std::chrono;

static bool nearTime(IAudioSource* source, float expected, float tolerance)
{
	const float actual = source->getCurrentAudioTime();
	if (std::abs(actual - expected) <= tolerance)
		return true;
	std::fprintf(stderr, "Expected playback time %.3f, got %.3f\n", expected, actual);
	return false;
}

static void service(IAudioManager& manager)
{
	if (!manager.isUpdateThreadRunning())
		manager.update();
}

static bool waitForTime(IAudioManager& manager, IAudioSource* source, float target)
{
	const auto deadline = steady_clock::now() + seconds(3);
	while (steady_clock::now() < deadline)
	{
		service(manager);
		if (source->getCurrentAudioTime() >= target)
			return true;
		std::this_thread::sleep_for(milliseconds(5));
	}
	return false;
}

int main(int argc, char** argv)
{
	std::unique_ptr<IAudioManager, decltype(&destroyAudioManager)> manager(createAudioManager(false), destroyAudioManager);
	if (!manager || !manager->initialize())
		return 1;

	const int rate = 48000;
	std::vector<char> pcm(4 * rate * 4); // Four seconds of stereo 16-bit PCM.
	auto* source = manager->createFromRaw("clock", pcm.data(), pcm.size(), rate, EAF_16BIT_STEREO);
	if (!source || !source->play2d() || !nearTime(source, 0.f, 0.1f))
		return 2;
	// Pass one full queue so the test exercises unqueue and refill as well as startup.
	if (!waitForTime(*manager, source, 1.2f))
		return 3;
	source->pause();
	const float paused = source->getCurrentAudioTime();
	if (!source->seek(0.5f, true) || !source->isPaused() || source->isPlaying())
		return 4;
	std::this_thread::sleep_for(milliseconds(60));
	if (!nearTime(source, paused + 0.5f, 0.001f) || !source->play() || !waitForTime(*manager, source, paused + 0.6f))
		return 5;
	if (!source->seek(0.25f, false) || !source->isPlaying() || !nearTime(source, 0.25f, 0.05f))
		return 6;

	source->stop();
	if (!nearTime(source, 0.f, 0.001f) || !source->seek(3.9f, false) || !source->play())
		return 7;
	const auto deadline = steady_clock::now() + seconds(2);
	while (source->getCurrentAudioTime() != 0.f && steady_clock::now() < deadline)
	{
		service(*manager);
		std::this_thread::sleep_for(milliseconds(5));
	}
	if (!source->isStopped() || !source->play() || !nearTime(source, 0.f, 0.1f))
		return 8;
	manager->release(source); // Release an active voice while its worker is running.

	auto* loop = manager->createFromRaw("loop", pcm.data(), rate * 2 / 8, rate, EAF_16BIT_MONO);
	if (!loop || !loop->play3d(cVector3(1.f, 0.f, -1.f), 1.f, true))
		return 9;
	for (int i = 0; i < 30; ++i)
	{
		std::this_thread::sleep_for(milliseconds(12));
		service(*manager);
		if (!loop->isPlaying() || loop->getCurrentAudioTime() > 0.126f)
			return 10;
	}
	manager->release(loop);

#ifdef USE_MINI_MP3
	if (argc > 1)
	{
		cMP3DecoderFactory factory;
		manager->registerAudioDecoder(&factory, "mp3");
		auto* music = manager->create("music", argv[1], true);
		if (!music || !music->play2d())
			return 11;
		music->pause();
		const float target = music->getTotalAudioTime() * 0.75f;
		if (!music->seek(target, false) || !nearTime(music, target, 0.001f) ||
			!music->play() || !waitForTime(*manager, music, target + 0.06f))
			return 12;
		manager->release(music);
		manager->unRegisterAudioDecoder("mp3");
	}
#endif
	return 0;
}
