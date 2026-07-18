/*
    SPDX-FileCopyrightText: 2026 Xavier Besson <developer@xavi-b.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root

    Kirigami.Form {
        Kirigami.FormGroup {
            title: i18nc("@title:group", "Detection")

            Kirigami.FormEntry {
                contentItem: QQC2.CheckBox {
                    id: enabledCheck
                    text: i18nc("@option:check", "Enable cat mode")
                    checked: kcm.settings.enabled
                    onToggled: kcm.settings.enabled = checked

                    KCM.SettingStateBinding {
                        configObject: kcm.settings
                        settingName: "Enabled"
                    }
                }
                subtitle: i18nc("@info:usagetip", "Automatically block keyboard input when cat-like key activity is detected. Unlock with Ctrl+Alt+K.")
            }

            Kirigami.FormEntry {
                contentItem: QQC2.ComboBox {
                    id: sensitivityCombo
                    Kirigami.FormData.label: i18nc("@label:listbox", "Sensitivity:")
                    model: [
                        { text: i18nc("@item:inlistbox", "Low"), value: "low" },
                        { text: i18nc("@item:inlistbox", "Medium"), value: "medium" },
                        { text: i18nc("@item:inlistbox", "High"), value: "high" },
                    ]
                    textRole: "text"
                    valueRole: "value"
                    currentIndex: {
                        switch (kcm.settings.sensitivity) {
                        case "low":
                            return 0;
                        case "high":
                            return 2;
                        default:
                            return 1;
                        }
                    }
                    onActivated: kcm.settings.sensitivity = currentValue

                    KCM.SettingStateBinding {
                        configObject: kcm.settings
                        settingName: "Sensitivity"
                    }
                }
            }
        }

        Kirigami.FormGroup {
            title: i18nc("@title:group", "Locking")

            Kirigami.FormEntry {
                contentItem: QQC2.SpinBox {
                    id: lockDurationSpin
                    readonly property string label: i18nc("@label:spinbox", "Lock duration:")
                    Kirigami.FormData.label: label
                    Accessible.name: label

                    from: 0
                    to: 600
                    stepSize: 5
                    editable: true

                    value: Math.round(kcm.settings.lockDurationMs / 1000)
                    onValueModified: kcm.settings.lockDurationMs = value * 1000

                    textFromValue: (value, locale) => {
                        return value === 0
                            ? i18nc("@item:intext lock until unlock chord", "Until Ctrl+Alt+K")
                            : i18ncp("@item:intext seconds", "%1 second", "%1 seconds", value);
                    }
                    valueFromText: (text, locale) => {
                        const match = text.match(/\d+/);
                        return match ? Number(match[0]) : 0;
                    }

                    KCM.SettingStateBinding {
                        configObject: kcm.settings
                        settingName: "LockDurationMs"
                    }
                }
                subtitle: i18nc("@info:usagetip", "How long input stays blocked after detection. Set to zero to require the unlock shortcut.")
            }

            Kirigami.FormEntry {
                contentItem: QQC2.SpinBox {
                    id: cooldownSpin
                    readonly property string label: i18nc("@label:spinbox", "Cooldown after unlock:")
                    Kirigami.FormData.label: label
                    Accessible.name: label

                    from: 0
                    to: 120
                    stepSize: 1
                    editable: true

                    value: Math.round(kcm.settings.cooldownMs / 1000)
                    onValueModified: kcm.settings.cooldownMs = value * 1000

                    textFromValue: (value, locale) => {
                        return i18ncp("@item:intext seconds", "%1 second", "%1 seconds", value);
                    }
                    valueFromText: (text, locale) => {
                        const match = text.match(/\d+/);
                        return match ? Number(match[0]) : 0;
                    }

                    KCM.SettingStateBinding {
                        configObject: kcm.settings
                        settingName: "CooldownMs"
                    }
                }
                subtitle: i18nc("@info:usagetip", "Ignore detection briefly after unlocking so normal typing does not immediately re-lock.")
            }
        }
    }
}
