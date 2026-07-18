# SPDX-FileCopyrightText: 2026 Xavier Besson <developer@xavi-b.fr>
# SPDX-License-Identifier: BSD-3-Clause

add_definitions(-DTRANSLATION_DOMAIN=\"kcm_catmode\")

kcmutils_generate_module_data(
    kcm_catmode_PART_SRCS
    MODULE_DATA_HEADER catmodedata.h
    MODULE_DATA_CLASS_NAME CatModeData
    SETTINGS_HEADERS catmodesettings.h
    SETTINGS_CLASSES CatModeSettings
)

kconfig_add_kcfg_files(kcm_catmode_PART_SRCS catmodesettings.kcfgc GENERATE_MOC)
kcmutils_add_qml_kcm(kcm_catmode SOURCES kcmcatmode.cpp ${kcm_catmode_PART_SRCS})

target_link_libraries(kcm_catmode PRIVATE
    KF6::I18n
    KF6::KCMUtils
    KF6::KCMUtilsQuick
)

install(FILES catmodesettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
