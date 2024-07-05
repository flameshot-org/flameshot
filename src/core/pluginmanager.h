#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#ifdef USE_PLUGIN_MANAGER

#define GET_BUILD_SET(x) #x
#define BUILD_DEFINE_CONV_C_STRING(x) GET_BUILD_SET(x)

#include <QList>
#include <QPluginLoader>
#include <any>
#include <functional>

const QString pluginDir = BUILD_DEFINE_CONV_C_STRING(PLUGIN_DIRECTORY);

const QString PluginDefineYaml = "plugin.yaml";

typedef struct _PluginInfo
{
    QString PluginName;
    QString PluginInfoFullPath;
    enum Type
    {
        QTPlugin,
        PyPlugin, /*no used*/
        LuaPlugin /*no used*/
    } PluginType;
    QString Pluginfile;
    void* PluginLoader;
} PluginInfo;

class PluginManager
{
    typedef void CallbackPluginLoads(QString PluginInfoDefaultYaml);

private:
    PluginManager();

public:
    static PluginManager* getInstance();
    int LoadPlugins();
    bool UnLoadPlugins();
    bool CallImagePost(QPixmap& pixmap);
    bool CallImageToPDFPost(QPixmap& pixmap);
    bool CallPrintPre(QPixmap& pixmap);

private:
    static void LoopDirsPlugins(QString Base,
                                std::function<CallbackPluginLoads> Callback);

private:
    QList<PluginInfo> PluginLists;
};

#endif // USE_PLUGIN_MANAGER

#endif // PLUGINMANAGER_H
