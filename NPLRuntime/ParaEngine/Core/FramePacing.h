#pragma once

namespace ParaEngine
{
    // Budget includes event processing, simulation, rendering and presentation.
    // Always yield to the event loop, even when a frame overruns its budget or
    // the caller requests uncapped rendering.
    inline int GetRemainingFrameDelayMs(double intervalSeconds, double elapsedSeconds)
    {
        const double remainingMs = (intervalSeconds - elapsedSeconds) * 1000.0;
        return remainingMs > 1.0 ? static_cast<int>(remainingMs) : 1;
    }
}
