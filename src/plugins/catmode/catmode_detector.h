/*
    SPDX-FileCopyrightText: 2026 Xavier Besson <developer@xavi-b.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <QList>
#include <QPair>
#include <QSet>

#include <chrono>

namespace KWin
{

/**
 * Pure heuristics for detecting cat-like keyboard activity.
 * Independent of the input filter so it can be unit-tested.
 */
class CatModeDetector
{
public:
    enum class Sensitivity {
        Low,
        Medium,
        High,
    };

    enum class Reason {
        None,
        PawPress,
        HoldSit,
        WalkBurst,
    };

    enum class KeyState {
        Released,
        Pressed,
        Repeated,
    };

    struct Thresholds
    {
        int pawPressKeys = 4;
        int holdSitRepeats = 8;
        int holdSitDistinctKeys = 2;
        std::chrono::milliseconds holdSitWindow{1000};
        int walkUniqueKeys = 8;
        int walkPresses = 12;
        std::chrono::milliseconds walkWindow{800};
    };

    void setSensitivity(Sensitivity sensitivity);
    Sensitivity sensitivity() const;
    Thresholds thresholds() const;

    void reset();

    /**
     * Feed a key event. Returns a detection reason when a cat pattern matches,
     * otherwise Reason::None. State is updated regardless.
     */
    Reason processKey(Qt::Key key, KeyState state, std::chrono::microseconds timestamp);

    int pressedNonModifierCount() const;
    static bool isModifier(Qt::Key key);

private:
    void pruneHistory(std::chrono::microseconds now);
    Reason evaluate(std::chrono::microseconds now) const;
    Reason evaluatePawPress() const;
    Reason evaluateHoldSit(std::chrono::microseconds now) const;
    Reason evaluateWalkBurst(std::chrono::microseconds now) const;

    Sensitivity m_sensitivity = Sensitivity::Medium;
    Thresholds m_thresholds;

    QSet<int> m_pressedNonModifiers;
    QList<QPair<std::chrono::microseconds, int>> m_recentPresses; // timestamp, Qt::Key
    QList<QPair<std::chrono::microseconds, int>> m_recentRepeats; // timestamp, Qt::Key
};

} // namespace KWin
