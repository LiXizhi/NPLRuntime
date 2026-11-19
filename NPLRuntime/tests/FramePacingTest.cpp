#include "Core/FramePacing.h"
#include <cstdio>
#include <initializer_list>

int main()
{
    struct Case { double interval, work; int expected; };
    const Case cases[] = {
        {1.0 / 60.0, 0.004, 12},
        {1.0 / 30.0, 0.004, 29},
        {1.0 / 120.0, 0.004, 4},
        {1.0 / 60.0, 0.016, 1},
        {1.0 / 60.0, 0.040, 1},
        {0.0, 0.004, 1},
        {-1.0, 0.004, 1},
    };
    for (const auto& c : cases)
    {
        const int delay = ParaEngine::GetRemainingFrameDelayMs(c.interval, c.work);
        if (delay != c.expected)
        {
            std::fprintf(stderr, "interval=%g work=%g: expected %d ms, got %d\n",
                c.interval, c.work, c.expected, delay);
            return 1;
        }
    }
    // Across a range of frame costs, pacing should consume the remaining
    // budget, rather than adding a constant sleep to each frame's work.
    for (int fps : {30, 60, 120})
    {
        const double budgetMs = 1000.0 / fps;
        for (int workMs = 0; workMs < 50; ++workMs)
        {
            const int delay = ParaEngine::GetRemainingFrameDelayMs(1.0 / fps, workMs / 1000.0);
            const double totalMs = workMs + delay;
            if ((workMs < budgetMs - 1 && (totalMs > budgetMs || totalMs < budgetMs - 1)) ||
                (workMs >= budgetMs && delay != 1))
                return 1;
        }
    }
    return 0;
}
