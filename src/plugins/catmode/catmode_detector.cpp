/*
    SPDX-FileCopyrightText: 2026 Xavier Besson <developer@xavi-b.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "catmode_detector.h"

#include <Qt>

#include <algorithm>

namespace KWin
{

static CatModeDetector::Thresholds thresholdsFor(CatModeDetector::Sensitivity sensitivity)
{
    CatModeDetector::Thresholds t;
    switch (sensitivity) {
    case CatModeDetector::Sensitivity::Low:
        t.pawPressKeys = 5;
        t.holdSitRepeats = 12;
        t.holdSitDistinctKeys = 3;
        t.walkUniqueKeys = 10;
        t.walkPresses = 16;
        break;
    case CatModeDetector::Sensitivity::High:
        t.pawPressKeys = 3;
        t.holdSitRepeats = 6;
        t.holdSitDistinctKeys = 2;
        t.walkUniqueKeys = 6;
        t.walkPresses = 9;
        break;
    case CatModeDetector::Sensitivity::Medium:
    default:
        break;
    }
    return t;
}

void CatModeDetector::setSensitivity(Sensitivity sensitivity)
{
    m_sensitivity = sensitivity;
    m_thresholds = thresholdsFor(sensitivity);
}

CatModeDetector::Sensitivity CatModeDetector::sensitivity() const
{
    return m_sensitivity;
}

CatModeDetector::Thresholds CatModeDetector::thresholds() const
{
    return m_thresholds;
}

void CatModeDetector::reset()
{
    m_pressedNonModifiers.clear();
    m_recentPresses.clear();
    m_recentRepeats.clear();
}

bool CatModeDetector::isModifier(Qt::Key key)
{
    switch (key) {
    case Qt::Key_Shift:
    case Qt::Key_Control:
    case Qt::Key_Alt:
    case Qt::Key_Meta:
    case Qt::Key_AltGr:
    case Qt::Key_CapsLock:
    case Qt::Key_NumLock:
    case Qt::Key_ScrollLock:
        return true;
    default:
        return false;
    }
}

int CatModeDetector::pressedNonModifierCount() const
{
    return m_pressedNonModifiers.size();
}

void CatModeDetector::pruneHistory(std::chrono::microseconds now)
{
    const auto maxWindow = std::max(m_thresholds.holdSitWindow, m_thresholds.walkWindow);
    const auto cutoff = now - std::chrono::duration_cast<std::chrono::microseconds>(maxWindow);

    while (!m_recentPresses.isEmpty() && m_recentPresses.first().first < cutoff) {
        m_recentPresses.removeFirst();
    }
    while (!m_recentRepeats.isEmpty() && m_recentRepeats.first().first < cutoff) {
        m_recentRepeats.removeFirst();
    }
}

CatModeDetector::Reason CatModeDetector::evaluatePawPress() const
{
    if (m_pressedNonModifiers.size() >= m_thresholds.pawPressKeys) {
        return Reason::PawPress;
    }
    return Reason::None;
}

CatModeDetector::Reason CatModeDetector::evaluateHoldSit(std::chrono::microseconds now) const
{
    const auto cutoff = now - std::chrono::duration_cast<std::chrono::microseconds>(m_thresholds.holdSitWindow);
    QSet<int> keys;
    int repeats = 0;
    for (const auto &entry : m_recentRepeats) {
        if (entry.first < cutoff) {
            continue;
        }
        ++repeats;
        keys.insert(entry.second);
    }
    if (repeats >= m_thresholds.holdSitRepeats && keys.size() >= m_thresholds.holdSitDistinctKeys) {
        return Reason::HoldSit;
    }
    return Reason::None;
}

CatModeDetector::Reason CatModeDetector::evaluateWalkBurst(std::chrono::microseconds now) const
{
    const auto cutoff = now - std::chrono::duration_cast<std::chrono::microseconds>(m_thresholds.walkWindow);
    QSet<int> uniqueKeys;
    int presses = 0;
    for (const auto &entry : m_recentPresses) {
        if (entry.first < cutoff) {
            continue;
        }
        ++presses;
        uniqueKeys.insert(entry.second);
    }
    if (uniqueKeys.size() >= m_thresholds.walkUniqueKeys && presses >= m_thresholds.walkPresses) {
        return Reason::WalkBurst;
    }
    return Reason::None;
}

CatModeDetector::Reason CatModeDetector::evaluate(std::chrono::microseconds now) const
{
    if (const auto reason = evaluatePawPress(); reason != Reason::None) {
        return reason;
    }
    if (const auto reason = evaluateHoldSit(now); reason != Reason::None) {
        return reason;
    }
    if (const auto reason = evaluateWalkBurst(now); reason != Reason::None) {
        return reason;
    }
    return Reason::None;
}

CatModeDetector::Reason CatModeDetector::processKey(Qt::Key key, KeyState state, std::chrono::microseconds timestamp)
{
    pruneHistory(timestamp);

    if (isModifier(key)) {
        return Reason::None;
    }

    switch (state) {
    case KeyState::Pressed: {
        m_pressedNonModifiers.insert(key);
        m_recentPresses.append({timestamp, key});
        break;
    }
    case KeyState::Repeated:
        m_recentRepeats.append({timestamp, key});
        break;
    case KeyState::Released:
        m_pressedNonModifiers.remove(key);
        return Reason::None;
    }

    return evaluate(timestamp);
}

} // namespace KWin
