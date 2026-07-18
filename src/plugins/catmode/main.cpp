/*
    SPDX-FileCopyrightText: 2026 Xavier Besson <developer@xavi-b.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "plugin.h"

#include "catmode.h"

class KWIN_EXPORT CatModeFactory : public KWin::PluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginFactory_iid FILE "metadata.json")
    Q_INTERFACES(KWin::PluginFactory)

public:
    std::unique_ptr<KWin::Plugin> create() const override
    {
        return std::make_unique<CatModeFilter>();
    }
};

#include "main.moc"
