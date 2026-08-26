// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#pragma once

#include "flameshotplugininterface.h"

#include <QIcon>
#include <QList>
#include <QObject>
#include <QPluginLoader>
#include <QString>
#include <QStringList>
#include <QVector>

class CaptureTool;

struct PluginMetadata
{
    QString filePath;
    QString fileName;
    QString id;
    QString name;
    QString version;
    QString author;
    QString description;
    QIcon icon;
    bool enabled{ true };
    FlameshotPluginInterface* instance{ nullptr };
    QPluginLoader* loader{ nullptr };
};

class PluginManager : public QObject
{
    Q_OBJECT

public:
    static PluginManager* instance();

    void scanAndLoadPlugins();
    void unloadPlugins();
    void reloadPlugins();

    const QVector<PluginMetadata>& plugins() const;
    QList<CaptureTool*> createPluginTools(QObject* parent = nullptr);

    bool isPluginEnabled(const QString& id) const;
    void setPluginEnabled(const QString& id, bool enabled);

    bool installPlugin(const QString& sourceFilePath,
                       QString* errorMsg = nullptr);
    bool removePlugin(const QString& id, QString* errorMsg = nullptr);

    QString userPluginsDirectory() const;
    QStringList pluginSearchPaths() const;

signals:
    void pluginsChanged();

private:
    explicit PluginManager(QObject* parent = nullptr);
    ~PluginManager() override;

    void loadPluginFromFile(const QString& path);

    static PluginManager* m_instance;
    QVector<PluginMetadata> m_plugins;
    QStringList m_disabledPluginIds;
};
