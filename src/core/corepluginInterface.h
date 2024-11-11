#ifndef COREPLUGININTERFACE_H
#define COREPLUGININTERFACE_H

#include <QtPlugin>

class CorePluginInterface
{
public:
    virtual ~CorePluginInterface() = 0;
    virtual bool load(std::map<std::string, std::string>& PluginConfig) = 0;
    virtual void unload() = 0;
    virtual bool ImagePost(QPixmap& pixmap) = 0;
    virtual bool ImageToPDFPost(QPixmap& pixmap) = 0;
    virtual bool PrintPre(QPixmap& pixmap) = 0;
};

#define FlameshotPlugin_iid "FlameshotPlugin.CorePluginInterface"

Q_DECLARE_INTERFACE(CorePluginInterface, FlameshotPlugin_iid)

#endif // COREPLUGININTERFACE_H
