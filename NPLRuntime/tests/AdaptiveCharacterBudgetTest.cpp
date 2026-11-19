#include "3dengine/AdaptiveCharacterBudget.h"
#include <cstdio>
#include <limits>

using ParaEngine::AdaptiveCharacterBudget;
#define CHECK(condition) do { if (!(condition)) { std::fprintf(stderr, "line %d: %s\n", __LINE__, #condition); return 1; } } while (0)

static void Run(AdaptiveCharacterBudget& budget, double fps, int frames,
    bool exceeded = true, bool active = true, double delta = 1.0 / 60.0)
{
    for (int i = 0; i < frames; ++i)
    {
        if (exceeded) budget.ReportExceeded();
        budget.Update(fps, delta, active);
    }
}

int main()
{
    AdaptiveCharacterBudget budget;
    budget.Reset(150000);
    Run(budget, 60, 600, false);
    CHECK(budget.GetEffective() == 150000); // no growth without omitted characters
    budget.Reset(150000);
    Run(budget, 60, 60);
    CHECK(budget.GetEffective() == 150000); // one second is insufficient
    Run(budget, 60, 65);
    CHECK(budget.GetEffective() == 187500);
    CHECK(budget.GetConfigured() == 150000);
    Run(budget, 42, 600);
    CHECK(budget.GetEffective() == 187500); // stable hysteresis band
    Run(budget, 25, 30);
    CHECK(budget.GetEffective() == 187500); // brief dip doesn't hide characters
    Run(budget, 25, 100);
    CHECK(budget.GetEffective() == 150000);
    Run(budget, 25, 600);
    CHECK(budget.GetEffective() == 150000); // configured limit remains the floor

    Run(budget, 60, 600, true, false);
    Run(budget, 0, 600);
    Run(budget, std::numeric_limits<double>::quiet_NaN(), 600);
    Run(budget, 60, 600, true, true, 0);
    Run(budget, 60, 30, true, true, 3.0);
    CHECK(budget.GetEffective() == 150000); // background/loading/invalid samples
    // Regression: a brief dip every second used to reset growth forever.
    for (int i = 0; i < 6; ++i) { Run(budget, 60, 59); Run(budget, 40, 1); }
    CHECK(budget.GetEffective() >= 234375); // continues beyond the first increase
    budget.Reset(150000);
    for (int i = 0; i < 6; ++i) { Run(budget, 60, 59); Run(budget, 60, 1, true, true, 0.12); }
    CHECK(budget.GetEffective() >= 234375); // periodic 120ms stalls don't starve growth
    budget.Reset(150000);
    for (int i = 0; i < 250; ++i) Run(budget, 60, 1, i % 2 == 0);
    CHECK(budget.GetEffective() >= 234375); // intermittent budget pressure still counts
    budget.Reset(150000);
    for (int i = 0; i < 10; ++i) { Run(budget, 60, 30); Run(budget, 30, 30); }
    CHECK(budget.GetEffective() == 150000); // fast spikes cannot hide a mediocre average
    budget.ReportExceeded();
    budget.ReportExceeded();
    budget.Update(60, 1.0 / 60, true);
    CHECK(budget.GetCulled() == 2);
    budget.Update(60, 1.0 / 60, true);
    CHECK(budget.GetCulled() == 0); // diagnostic reflects the previous frame only

    Run(budget, 60, 250);
    CHECK(budget.GetEffective() > 150000);
    budget.SetEnabled(false);
    Run(budget, 60, 600);
    CHECK(budget.GetEffective() == 150000);
    budget.SetEnabled(true);
    budget.Reset(0);
    Run(budget, 60, 600);
    CHECK(budget.GetEffective() == 0);
    budget.Reset(-1);
    Run(budget, 60, 600);
    CHECK(budget.GetEffective() == -1);
    budget.Reset((std::numeric_limits<int>::max)() - 1);
    Run(budget, 60, 250);
    CHECK(budget.GetEffective() == (std::numeric_limits<int>::max)());
    budget.Reset(50000);
    CHECK(budget.GetEffective() == 50000);
    std::puts("Adaptive character budget checks passed");
    return 0;
}
