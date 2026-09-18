// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#pragma once

#include <QIcon>
#include <QList>
#include <QObject>
#include <QString>
#include <QtPlugin>

class CaptureTool;

/**
 * @brief FlameshotPluginInterface is the root interface that all Flameshot
 * dynamic plugins (.so / .dll / .dylib) must implement.
 */
class FlameshotPluginInterface
{
public:
    virtual ~FlameshotPluginInterface() = default;

    /**
     * @brief Unique reverse-DNS identifier for the plugin (e.g.,
     * "org.flameshot.ocr")
     */
    virtual QString pluginId() const = 0;

    /**
     * @brief Human-readable name of the plugin
     */
    virtual QString pluginName() const = 0;

    /**
     * @brief Version string of the plugin (e.g., "1.0.0")
     */
    virtual QString pluginVersion() const = 0;

    /**
     * @brief Author or maintainer of the plugin
     */
    virtual QString pluginAuthor() const = 0;

    /**
     * @brief Description of what the plugin does
     */
    virtual QString pluginDescription() const = 0;

    /**
     * @brief Optional icon representing the plugin in the configuration manager
     */
    virtual QIcon pluginIcon() const { return {}; }

    /**
     * @brief Factory method returning custom CaptureTools provided by this
     * plugin.
     * @param parent The parent QObject for lifetime management.
     */
    virtual QList<CaptureTool*> createTools(QObject* parent = nullptr) = 0;

    /**
     * @brief Optional lifecycle hook called when the plugin is loaded and
     * initialized.
     */
    virtual void onPluginLoaded() {}

    /**
     * @brief Optional lifecycle hook called before the plugin is unloaded.
     */
    virtual void onPluginUnloaded() {}
};

#define FlameshotPluginInterface_iid                                           \
    "org.flameshot.FlameshotPluginInterface/1.0"
Q_DECLARE_INTERFACE(FlameshotPluginInterface, FlameshotPluginInterface_iid)
