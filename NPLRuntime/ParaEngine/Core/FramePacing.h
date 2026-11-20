#pragma once

namespace ParaEngine
{
    // Budget includes event processing, simulation, rendering and presentation.
    // Ordinary frames keep the timer yield, including zero-interval requests.
    inline int GetRemainingFrameDelayMs(double intervalSeconds, double elapsedSeconds, bool profileUnlimitedFPS = false)
    {
        if (profileUnlimitedFPS)
            return 0;
        const double remainingMs = (intervalSeconds - elapsedSeconds) * 1000.0;
        return remainingMs > 1.0 ? static_cast<int>(remainingMs) : 1;
    }
    // Profiling bypasses only the frequency gate. Frozen clocks stay frozen.
    inline bool ShouldRunFrame(double controllerDelta, bool profileUnlimitedFPS,
        double previousTime, double currentTime, bool paused)
    {
        return controllerDelta > 0 || (profileUnlimitedFPS && currentTime > previousTime && !paused);
    }
}
