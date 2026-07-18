/*
    SPDX-FileCopyrightText: 2026 Xavier Besson <developer@xavi-b.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcmcatmode.h"

#include <qqml.h>

#include <KPluginFactory>

#include "catmodedata.h"

K_PLUGIN_FACTORY_WITH_JSON(KcmCatModeFactory, "kcm_catmode.json", registerPlugin<KcmCatMode>(); registerPlugin<CatModeData>();)

KcmCatMode::KcmCatMode(QObject *parent, const KPluginMetaData &metaData)
    : KQuickManagedConfigModule(parent, metaData)
    , m_data(new CatModeData(this))
    , m_settings(new CatModeSettings(m_data))
{
    registerSettings(m_settings);
    qmlRegisterAnonymousType<CatModeSettings>("org.kde.kwin.catmodesettings", 1);
}

#include "kcmcatmode.moc"
#include "moc_kcmcatmode.cpp"
