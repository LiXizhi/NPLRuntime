#pragma once
#include <algorithm>
#include <cmath>
#include <limits>

namespace ParaEngine
{
    // Adapt from a time window, not an uninterrupted streak of fast frames:
    // periodic brief stalls must not permanently starve a crowded scene.
    class AdaptiveCharacterBudget
    {
    public:
        void Reset(int triangles)
        {
            m_configured = m_effective = triangles;
            ResetWindow();
            m_culled = m_lastCulled = 0;
        }
        int GetConfigured() const { return m_configured; }
        int GetEffective() const { return m_effective; }
        int GetCulled() const { return m_lastCulled; }
        bool IsEnabled() const { return m_enabled; }
        void SetEnabled(bool enabled) { m_enabled = enabled; Reset(m_configured); }
        void ReportExceeded() { ++m_culled; }

        // Called once per main scene frame, before any reflection/shadow passes.
        void Update(double fps, double seconds, bool active)
        {
            m_lastCulled = m_culled;
            m_culled = 0;
            if (!m_enabled || m_configured <= 0 || !active ||
                !std::isfinite(fps) || fps <= 0 || !std::isfinite(seconds) || seconds <= 0 || seconds > 0.5)
            {
                // Discard background/loading intervals and stale FPS samples.
                ResetWindow();
                return;
            }
            m_windowSeconds += seconds;
            // Include brief stalls, conservatively accounting for the current
            // frame when the application's once-per-second FPS is still stale.
            m_weightedFPS += (std::min)(fps, 1.0 / seconds) * seconds;
            m_windowExceeded = m_windowExceeded || m_lastCulled > 0;
            if (m_windowSeconds >= 2.0)
            {
                const double averageFPS = m_weightedFPS / m_windowSeconds;
                if (averageFPS >= 50 && m_windowExceeded)
                {
                    const int room = (std::numeric_limits<int>::max)() - m_effective;
                    m_effective += (std::min)(room, (std::max)(1, m_effective / 4));
                }
                else if (averageFPS < 35)
                {
                    m_effective = (std::max)(m_configured,
                        m_effective - (std::max)(1, m_effective / 5));
                }
                ResetWindow();
            }
        }

    private:
        void ResetWindow()
        {
            m_windowSeconds = m_weightedFPS = 0;
            m_windowExceeded = false;
        }
        int m_configured = 50000;
        int m_effective = 50000;
        bool m_enabled = true;
        int m_culled = 0;
        int m_lastCulled = 0;
        double m_windowSeconds = 0;
        double m_weightedFPS = 0;
        bool m_windowExceeded = false;
    };
}
