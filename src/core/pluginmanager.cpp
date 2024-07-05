#ifdef USE_PLUGIN_MANAGER

#include "pluginmanager.h"
#include "corepluginInterface.h"
#include <QDir>
#include <QObject>
#include <yaml-cpp/yaml.h>

PluginManager::PluginManager() {}

PluginManager* PluginManager::getInstance()
{
    static PluginManager pluginManager;
    qDebug()
      << QObject::tr("Get the plugin manager interface: ").toStdString().c_str()
      << &pluginManager;
    return &pluginManager;
}

void PluginManager::LoopDirsPlugins(QString Base,
                                    std::function<CallbackPluginLoads> Callback)
{
    QDir dirs(Base);
    if (dirs.exists()) {
        dirs.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
        QFileInfoList fileList = dirs.entryInfoList();
        foreach (QFileInfo fileInfo, fileList) {
            if (Base != pluginDir && fileInfo.isDir()) {
                QFileInfo infocheck(fileInfo.absolutePath() + PluginDefineYaml);
                if (!infocheck.exists()) {
                    LoopDirsPlugins(fileInfo.absoluteFilePath(), Callback);
                }
            } else if (Base == pluginDir && fileInfo.isDir()) {
                LoopDirsPlugins(fileInfo.absoluteFilePath(), Callback);
            } else if (fileInfo.isFile()) {
                if (fileInfo.fileName() == PluginDefineYaml) {
                    Callback(fileInfo.absoluteFilePath());
                }
            }
        }
    }
}

int PluginManager::LoadPlugins()
{
    int count = 0;
    LoopDirsPlugins(pluginDir, [&](QString PluginInfoDefaultYaml) {
        qDebug() << QObject::tr("Get Plugin Definde: ").toStdString().c_str()
                 << PluginInfoDefaultYaml;
        YAML::Node config;
        try {
            config = YAML::LoadFile(PluginInfoDefaultYaml.toStdString());
        } catch (std::exception& e) {
            qDebug() << QObject::tr("Get Plugin Error: ").toStdString().c_str()
                     << e.what() << PluginInfoDefaultYaml;
            return;
        }

        PluginInfo Info;
        Info.PluginInfoFullPath = PluginInfoDefaultYaml;
        Info.PluginName =
          QString::fromStdString(config["plugin"]["name"].as<std::string>());
        if (config["plugin"]["type"].as<std::string>() == "qt") {
            Info.PluginType = PluginInfo::QTPlugin;
        }

        QString plugin =
          QFileInfo(PluginInfoDefaultYaml).absolutePath() + "/" +
          QString::fromStdString(config["plugin"]["file"].as<std::string>());
        Info.Pluginfile = plugin;

        std::map<std::string, std::string> PluginConfig =
          config["plugin"]["config"].as<std::map<std::string, std::string>>();

        QPluginLoader* Loader = new QPluginLoader(Info.Pluginfile);
        if (Loader) {
            if (Loader->load()) {
                CorePluginInterface* Interface =
                  qobject_cast<CorePluginInterface*>(Loader->instance());
                if (Interface->load(PluginConfig)) {
                    Info.PluginLoader = Loader;
                }
            }

            this->PluginLists.append(Info);

            count++;
        }
    });
    qDebug() << QObject::tr("Get Plugin Count: ").toStdString().c_str()
             << count;
    return count;
}

bool PluginManager::UnLoadPlugins()
{
    foreach (auto Info, this->PluginLists) {
        if (Info.PluginType == PluginInfo::QTPlugin) {
            QPluginLoader* Loader =
              reinterpret_cast<QPluginLoader*>(Info.PluginLoader);
            if (Loader) {
                delete Loader;
                Info.PluginLoader = nullptr;
            }
        }
    }
    this->PluginLists.clear();
    return false;
}

bool PluginManager::CallImagePost(QPixmap& pixmap)
{
    bool Result = false;
    foreach (auto Info, this->PluginLists) {
        if (Info.PluginType == PluginInfo::QTPlugin) {
            qDebug()
              << QObject::tr("Call Plugin(ImagePost): ").toStdString().c_str()
              << Info.PluginName;
            qDebug() << QObject::tr("Call Plugin(ImagePost) YAML FileName: ")
                          .toStdString()
                          .c_str()
                     << Info.PluginInfoFullPath;
            qDebug() << QObject::tr("Call Plugin(ImagePost) Qt Plugin: ")
                          .toStdString()
                          .c_str()
                     << Info.Pluginfile;
            if (Info.PluginType == PluginInfo::QTPlugin) {
                QPluginLoader* Loader =
                  reinterpret_cast<QPluginLoader*>(Info.PluginLoader);
                if (Loader) {
                    CorePluginInterface* Interface =
                      qobject_cast<CorePluginInterface*>(Loader->instance());
                    Interface->ImagePost(pixmap);
                    Result = true;
                }
            }
        }
    }
    return Result;
}

bool PluginManager::CallImageToPDFPost(QPixmap& pixmap)
{
    bool Result = false;
    foreach (auto Info, this->PluginLists) {
        if (Info.PluginType == PluginInfo::QTPlugin) {
            qDebug() << QObject::tr("Call Plugin(ImageToPDFPost): ")
                          .toStdString()
                          .c_str()
                     << Info.PluginName;
            qDebug() << QObject::tr(
                          "Call Plugin(ImageToPDFPost) YAML FileName: ")
                          .toStdString()
                          .c_str()
                     << Info.PluginInfoFullPath;
            qDebug() << QObject::tr("Call Plugin(ImageToPDFPost) Qt Plugin: ")
                          .toStdString()
                          .c_str()
                     << Info.Pluginfile;
            if (Info.PluginType == PluginInfo::QTPlugin) {
                QPluginLoader* Loader =
                  reinterpret_cast<QPluginLoader*>(Info.PluginLoader);
                if (Loader) {
                    CorePluginInterface* Interface =
                      qobject_cast<CorePluginInterface*>(Loader->instance());
                    Interface->ImageToPDFPost(pixmap);
                    Result = true;
                }
            }
        }
    }
    return Result;
}

bool PluginManager::CallPrintPre(QPixmap& pixmap)
{
    bool Result = false;
    foreach (auto Info, this->PluginLists) {
        if (Info.PluginType == PluginInfo::QTPlugin) {
            qDebug()
              << QObject::tr("Call Plugin(PrintPre): ").toStdString().c_str()
              << Info.PluginName;
            qDebug() << QObject::tr("Call Plugin(PrintPre) YAML FileName: ")
                          .toStdString()
                          .c_str()
                     << Info.PluginInfoFullPath;
            qDebug() << QObject::tr("Call Plugin(PrintPre) Qt Plugin: ")
                          .toStdString()
                          .c_str()
                     << Info.Pluginfile;
            if (Info.PluginType == PluginInfo::QTPlugin) {
                QPluginLoader* Loader =
                  reinterpret_cast<QPluginLoader*>(Info.PluginLoader);
                if (Loader) {
                    CorePluginInterface* Interface =
                      qobject_cast<CorePluginInterface*>(Loader->instance());
                    Interface->PrintPre(pixmap);
                    Result = true;
                }
            }
        }
    }
    return Result;
}
#endif // USE_PLUGIN_MANAGER
