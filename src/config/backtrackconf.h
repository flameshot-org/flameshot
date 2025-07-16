//
// Created by michael on 24-6-5.
//

#ifndef BACKTRACKINGIMPL_H
#define BACKTRACKINGIMPL_H

#include <QtWidgets/qgroupbox.h>
#include <QtWidgets/qwidget.h>
#include <memory>
#include <qobjectdefs.h>

class QVBoxLayout;
class QCheckBox;
class QPushButton;
class QLabel;
class QLineEdit;
class QSpinBox;
class QComboBox;

class BacktrackConfigGroup : public QGroupBox
{
    Q_OBJECT
public:
    explicit BacktrackConfigGroup(QWidget* parent = nullptr);
    ~BacktrackConfigGroup() override;

private:
    std::unique_ptr<class BacktrackConfigPrivate> p;
    void init();
    QString chooseFolder(const QString& defaultPath);
    class ConfigHandler* m_configHandler;
public slots:
    void browseFolder();
    void setEnable(bool value);
    void setPath2Default();
    void setCacheLimits(int val);
};

#endif // BACKTRACKINGIMPL_H
