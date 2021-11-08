#ifndef GRADIENT_SLIDER_PLUGIN_HPP
#define GRADIENT_SLIDER_PLUGIN_HPP

#include <QtUiPlugin/QDesignerCustomWidgetInterface>

class GradientSlider_Plugin : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    GradientSlider_Plugin(QObject *parent = 0);

    void initialize(QDesignerFormEditorInterface *core);
    bool isInitialized() const;

    QWidget *createWidget(QWidget *parent);

    QString name() const;
    QString group() const;
    QIcon icon() const;
    QString toolTip() const;
    QString whatsThis() const;
    bool isContainer() const;

    QString domXml() const;

    QString includeFile() const;

private:
    bool initialized;
};

#endif // GRADIENT_SLIDER_PLUGIN_HPP
