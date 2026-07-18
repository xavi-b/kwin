/*
    SPDX-FileCopyrightText: 2026 Xavier Besson <developer@xavi-b.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include "catmode_detector.h"
#include "input.h"
#include "input_event.h"
#include "plugin.h"

#include <QElapsedTimer>
#include <QSet>
#include <QTimer>

class CatModeFilter : public KWin::Plugin, public KWin::InputEventFilter
{
    Q_OBJECT

public:
    explicit CatModeFilter();

    bool keyboardKey(KWin::KeyboardKeyEvent *event) override;

private:
    void loadConfig(const KConfigGroup &group);
    void lock(KWin::CatModeDetector::Reason reason, std::chrono::microseconds timestamp);
    void unlock();
    void releasePressedKeys(std::chrono::microseconds timestamp);
    bool inCooldown() const;
    bool isUnlockChordComplete() const;
    void sendNotification(const QString &eventId, const QString &text);

    KConfigWatcher::Ptr m_configWatcher;
    KWin::CatModeDetector m_detector;
    QTimer m_unlockTimer;
    QElapsedTimer m_cooldownTimer;

    bool m_enabled = true;
    bool m_locked = false;
    bool m_neutralizingKeys = false;
    int m_lockDurationMs = 30000;
    int m_cooldownMs = 5000;

    QSet<int> m_pressedModifiers;
    bool m_unlockKeyHeld = false;
};
