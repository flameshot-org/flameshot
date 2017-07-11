#ifndef STRFTIMECHOOSERWIDGET_H
#define STRFTIMECHOOSERWIDGET_H

#include <QWidget>

class StrftimeChooserWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StrftimeChooserWidget(QWidget *parent = nullptr);

signals:
    void variableEmitted(const QString &);

private:
    static QStringList m_valuesStr;
    static QStringList m_buttonLabel;
};

#endif // STRFTIMECHOOSERWIDGET_H
