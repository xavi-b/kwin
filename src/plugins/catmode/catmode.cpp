/*
    SPDX-FileCopyrightText: 2026 Xavier Besson <developer@xavi-b.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "catmode.h"
#include "catmode_debug.h"
#include "config-kwin.h"
#include "keyboard_input.h"

#include <KLocalizedString>
#if KWIN_BUILD_NOTIFICATIONS
#include <KNotification>
#endif

CatModeFilter::CatModeFilter()
    // Reuse BounceKeys weight so this builds against stock KWin without a new InputFilterOrder value.
    : KWin::InputEventFilter(KWin::InputFilterOrder::BounceKeys)
    , m_configWatcher(KConfigWatcher::create(KSharedConfig::openConfig(QStringLiteral("kwinrc"))))
{
    m_unlockTimer.setSingleShot(true);
    connect(&m_unlockTimer, &QTimer::timeout, this, [this]() {
        if (m_locked) {
            unlock();
        }
    });

    const QLatin1StringView groupName("CatMode");
    connect(m_configWatcher.get(), &KConfigWatcher::configChanged, this, [this, groupName](const KConfigGroup &group) {
        if (group.name() == groupName) {
            loadConfig(group);
        }
    });
    loadConfig(m_configWatcher->config()->group(groupName));
}

void CatModeFilter::loadConfig(const KConfigGroup &group)
{
    m_enabled = group.readEntry("Enabled", true);
    m_lockDurationMs = group.readEntry("LockDurationMs", 30000);
    m_cooldownMs = group.readEntry("CooldownMs", 5000);

    const QString sensitivity = group.readEntry("Sensitivity", QStringLiteral("medium")).toLower();
    if (sensitivity == QLatin1String("low")) {
        m_detector.setSensitivity(KWin::CatModeDetector::Sensitivity::Low);
    } else if (sensitivity == QLatin1String("high")) {
        m_detector.setSensitivity(KWin::CatModeDetector::Sensitivity::High);
    } else {
        m_detector.setSensitivity(KWin::CatModeDetector::Sensitivity::Medium);
    }

    if (!m_enabled && m_locked) {
        unlock();
    }

    KWin::input()->uninstallInputEventFilter(this);
    if (m_enabled) {
        KWin::input()->installInputEventFilter(this);
    }
}

bool CatModeFilter::inCooldown() const
{
    return m_cooldownTimer.isValid() && m_cooldownTimer.elapsed() < m_cooldownMs;
}

bool CatModeFilter::isUnlockChordComplete() const
{
    // Ctrl+Alt+K
    const bool ctrl = m_pressedModifiers.contains(Qt::Key_Control);
    const bool alt = m_pressedModifiers.contains(Qt::Key_Alt);
    return ctrl && alt && m_unlockKeyHeld;
}

void CatModeFilter::releasePressedKeys(std::chrono::microseconds timestamp)
{
    // Keys already delivered to clients keep repeating until they see a release.
    // Synthesize releases for every currently pressed key before we start eating input.
    m_neutralizingKeys = true;
    const QList<uint32_t> keys = KWin::input()->keyboard()->pressedKeys();
    for (const uint32_t key : keys) {
        KWin::input()->keyboard()->processKey(key, KWin::KeyboardKeyState::Released, timestamp);
    }
    m_neutralizingKeys = false;
}

void CatModeFilter::lock(KWin::CatModeDetector::Reason reason, std::chrono::microseconds timestamp)
{
    if (m_locked) {
        return;
    }

    m_locked = true;
    releasePressedKeys(timestamp);
    m_detector.reset();
    m_pressedModifiers.clear();
    m_unlockKeyHeld = false;
    m_cooldownTimer.invalidate();

    qCInfo(KWIN_CATMODE) << "Cat mode locked, reason:" << static_cast<int>(reason);

    if (m_lockDurationMs > 0) {
        m_unlockTimer.start(m_lockDurationMs);
    }

    sendNotification(QStringLiteral("catmodelocked"),
                     i18n("Cat mode enabled: keyboard input is blocked. Press Ctrl+Alt+K to unlock."));
}

void CatModeFilter::unlock()
{
    if (!m_locked) {
        return;
    }

    m_locked = false;
    m_unlockTimer.stop();
    m_detector.reset();
    m_pressedModifiers.clear();
    m_unlockKeyHeld = false;
    m_cooldownTimer.start();

    qCInfo(KWIN_CATMODE) << "Cat mode unlocked";

    sendNotification(QStringLiteral("catmodeunlocked"),
                     i18n("Cat mode disabled: keyboard input restored."));
}

void CatModeFilter::sendNotification(const QString &eventId, const QString &text)
{
#if KWIN_BUILD_NOTIFICATIONS
    KNotification *notification = new KNotification(eventId);
    notification->setComponentName(QStringLiteral("catmode"));
    notification->setTitle(i18n("Cat Mode"));
    notification->setText(text);
    notification->setIconName(QStringLiteral("input-keyboard"));
    notification->sendEvent();
#else
    Q_UNUSED(eventId);
    Q_UNUSED(text);
#endif
}

bool CatModeFilter::keyboardKey(KWin::KeyboardKeyEvent *event)
{
    if (m_neutralizingKeys) {
        // Let synthetic releases reach clients and stop compositor key repeat.
        return false;
    }

    if (KWin::CatModeDetector::isModifier(event->key)) {
        if (event->state == KWin::KeyboardKeyState::Pressed) {
            m_pressedModifiers.insert(event->key);
        } else if (event->state == KWin::KeyboardKeyState::Released) {
            m_pressedModifiers.remove(event->key);
        }
    }

    if (event->key == Qt::Key_K) {
        if (event->state == KWin::KeyboardKeyState::Pressed) {
            m_unlockKeyHeld = true;
        } else if (event->state == KWin::KeyboardKeyState::Released) {
            m_unlockKeyHeld = false;
        }
    }

    if (m_locked) {
        if (isUnlockChordComplete() && event->state == KWin::KeyboardKeyState::Pressed && event->key == Qt::Key_K) {
            unlock();
            return true;
        }
        return true;
    }

    if (!m_enabled) {
        return false;
    }

    KWin::CatModeDetector::KeyState detectorState = KWin::CatModeDetector::KeyState::Released;
    switch (event->state) {
    case KWin::KeyboardKeyState::Pressed:
        detectorState = KWin::CatModeDetector::KeyState::Pressed;
        break;
    case KWin::KeyboardKeyState::Repeated:
        detectorState = KWin::CatModeDetector::KeyState::Repeated;
        break;
    case KWin::KeyboardKeyState::Released:
        detectorState = KWin::CatModeDetector::KeyState::Released;
        break;
    }

    if (inCooldown()) {
        // Still track state so the detector stays consistent after cooldown.
        m_detector.processKey(event->key, detectorState, event->timestamp);
        return false;
    }

    const auto reason = m_detector.processKey(event->key, detectorState, event->timestamp);
    if (reason != KWin::CatModeDetector::Reason::None) {
        lock(reason, event->timestamp);
        return true;
    }

    return false;
}

#include "moc_catmode.cpp"
