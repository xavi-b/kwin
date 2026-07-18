# SPDX-FileCopyrightText: 2026 Xavier Besson <developer@xavi-b.fr>
# SPDX-License-Identifier: BSD-3-Clause

# Supports both in-tree KWin builds and a standalone build against an installed KWin.
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    cmake_minimum_required(VERSION 3.20)
    project(kwin-catmode)

    set(KF6_MIN_VERSION "6.0.0")
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
    set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

    include(FeatureSummary)
    include(KDEInstallDirs)
    include(KDECMakeSettings)
    include(KDECompilerSettings NO_POLICY_SCOPE)
    include(ECMQtDeclareLoggingCategory)

    find_package(Qt6 REQUIRED COMPONENTS Core Widgets Quick)
    find_package(KF6 REQUIRED COMPONENTS Config I18n WindowSystem KCMUtils)
    find_package(KF6 QUIET COMPONENTS Notifications)
    find_package(KWin REQUIRED COMPONENTS kwin)

    set(KWIN_BUILD_NOTIFICATIONS ${KF6Notifications_FOUND})
    set(CATMODE_STANDALONE TRUE)
endif()

kcoreaddons_add_plugin(CatModePlugin INSTALL_NAMESPACE "kwin/plugins")

ecm_qt_declare_logging_category(CatModePlugin
    HEADER catmode_debug.h
    IDENTIFIER KWIN_CATMODE
    CATEGORY_NAME kwin_catmode
    DEFAULT_SEVERITY Warning
)

target_sources(CatModePlugin PRIVATE
    main.cpp
    catmode.cpp
    catmode_detector.cpp
)

target_compile_definitions(CatModePlugin PRIVATE -DTRANSLATION_DOMAIN=\"kwin\")

if (CATMODE_STANDALONE)
    target_link_libraries(CatModePlugin PRIVATE
        KWin::kwin
        KF6::ConfigCore
        KF6::ConfigGui
        KF6::WindowSystem
        KF6::I18n
    )
    if (KWIN_BUILD_NOTIFICATIONS)
        target_compile_definitions(CatModePlugin PRIVATE KWIN_BUILD_NOTIFICATIONS=1)
        target_link_libraries(CatModePlugin PRIVATE KF6::Notifications)
    endif()
else()
    target_link_libraries(CatModePlugin PRIVATE kwin KF6::WindowSystem KF6::I18n)
    if (KWIN_BUILD_NOTIFICATIONS)
        target_link_libraries(CatModePlugin PRIVATE KF6::Notifications)
    endif()
endif()

add_subdirectory(kcm)

install(FILES catmode.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})

if (CATMODE_STANDALONE)
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
