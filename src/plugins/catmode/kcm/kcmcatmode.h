/*
    SPDX-FileCopyrightText: 2026 Xavier Besson <developer@xavi-b.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KQuickManagedConfigModule>

#include "catmodesettings.h"

class CatModeData;

class KcmCatMode : public KQuickManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(CatModeSettings *settings READ settings CONSTANT)

public:
    explicit KcmCatMode(QObject *parent, const KPluginMetaData &metaData);

    CatModeSettings *settings() const
    {
        return m_settings;
    }

private:
    CatModeData *const m_data;
    CatModeSettings *const m_settings;
};
