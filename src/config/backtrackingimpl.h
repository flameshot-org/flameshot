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
namespace btk {

class BackTrackerConfigPrivate;
class BackTrackerConfigGroup : public QGroupBox
{
    Q_OBJECT
public:
    explicit BackTrackerConfigGroup(QWidget* parent = nullptr);
    ~BackTrackerConfigGroup() override;

private:
    BackTrackerConfigPrivate* p;
    void init();
    QString chooseFolder(const QString& defaultPath);
public slots:
    void browseFolder();
    void setEnable(bool value);
    void pathRollBack();
    void setCacheLimits(int val);
};
} // btk

#endif // BACKTRACKINGIMPL_H
