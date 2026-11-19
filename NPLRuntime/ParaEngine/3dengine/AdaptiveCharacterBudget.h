#pragma once
#include <algorithm>
#include <cmath>
#include <limits>

namespace ParaEngine
{
    // Grow only when characters were omitted, and use separated FPS thresholds
    // and sustained intervals so one expensive frame cannot toggle a crowd.
    class AdaptiveCharacterBudget
    {
    public:
        void Reset(int triangles)
        {
            m_configured = m_effective = triangles;
            m_fastSeconds = m_slowSeconds = 0;
            m_exceeded = false;
        }
        int GetConfigured() const { return m_configured; }
        int GetEffective() const { return m_effective; }
        bool IsEnabled() const { return m_enabled; }
        void SetEnabled(bool enabled) { m_enabled = enabled; Reset(m_configured); }
        void ReportExceeded() { m_exceeded = true; }

        // Called once per main scene frame, before any reflection/shadow passes.
        void Update(double fps, double seconds, bool active)
        {
            const bool exceeded = m_exceeded;
            m_exceeded = false;
            if (!m_enabled || m_configured <= 0 || !active ||
                !std::isfinite(fps) || fps <= 0 || !std::isfinite(seconds) || seconds <= 0)
            {
                m_fastSeconds = m_slowSeconds = 0;
                return;
            }
            // Don't treat a loading stall as sustained evidence from a stale FPS sample.
            const double elapsed = (std::min)(seconds, 0.25);
            if (fps >= 50 && seconds <= 0.1 && exceeded)
            {
                m_slowSeconds = 0;
                m_fastSeconds += elapsed;
                if (m_fastSeconds >= 2.0)
                {
                    const int room = (std::numeric_limits<int>::max)() - m_effective;
                    m_effective += (std::min)(room, (std::max)(1, m_effective / 4));
                    m_fastSeconds = 0;
                }
            }
            else if (fps < 35)
            {
                m_fastSeconds = 0;
                m_slowSeconds += elapsed;
                if (m_slowSeconds >= 1.0)
                {
                    m_effective = (std::max)(m_configured,
                        m_effective - (std::max)(1, m_effective / 5));
                    m_slowSeconds = 0;
                }
            }
            else
                m_fastSeconds = m_slowSeconds = 0;
        }

    private:
        int m_configured = 50000;
        int m_effective = 50000;
        bool m_enabled = true;
        bool m_exceeded = false;
        double m_fastSeconds = 0;
        double m_slowSeconds = 0;
    };
}
