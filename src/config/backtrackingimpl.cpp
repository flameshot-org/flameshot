#include "backtrackingimpl.h"

#include "abstractlogger.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QtCore/qstandardpaths.h>
#include <QtCore/qstring.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qgroupbox.h>
#include <QtWidgets/qlayoutitem.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qsizepolicy.h>
#include <QtWidgets/qspinbox.h>
#include <QtWidgets/qwidget.h>
#include <cacheutils.h>
#include <confighandler.h>

namespace btk {
class BackTrackerConfigPrivate
{
    friend class BackTrackerConfigGroup;
    constexpr static int limits_max = 200;
    constexpr static int limits_min = 0;

public:
    QCheckBox* enableBackTrackerCheckBox;
    QLineEdit* backTrackerPath;
    QPushButton* choosePathBtn;
    QPushButton* rollbackBtn;
    QSpinBox* cacheNumbersSpinBox;
};

BackTrackerConfigGroup::BackTrackerConfigGroup(QWidget* parent)
  : QGroupBox(tr("BackTracking"), parent)
  , p(new BackTrackerConfigPrivate)
{
    init();
}
BackTrackerConfigGroup::~BackTrackerConfigGroup()
{
    delete p;
    p = nullptr;
}
void BackTrackerConfigGroup::init()
{
    p->enableBackTrackerCheckBox = new QCheckBox(tr("enable"), this);
    p->enableBackTrackerCheckBox->setToolTip(tr("If enable the backtraking"));
    p->enableBackTrackerCheckBox->setChecked(
      ConfigHandler().backtrackingEnable());
    connect(p->enableBackTrackerCheckBox,
            &QCheckBox::clicked,
            this,
            &BackTrackerConfigGroup::setEnable);
    setFlat(true);
    auto* vboxLayout = new QVBoxLayout();
    setLayout(vboxLayout);

    auto* firstHBoxLayout = new QHBoxLayout();
    vboxLayout->addLayout(firstHBoxLayout);
    firstHBoxLayout->addWidget(p->enableBackTrackerCheckBox);

    auto* spinboxLabel = new QLabel(tr("cache size"));
    spinboxLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    p->cacheNumbersSpinBox = new QSpinBox();
    p->cacheNumbersSpinBox->setValue(ConfigHandler().backtrackingCacheLimits());
    p->cacheNumbersSpinBox->setFixedWidth(50);
    p->cacheNumbersSpinBox->setMinimum(BackTrackerConfigPrivate::limits_min);
    p->cacheNumbersSpinBox->setMaximum(BackTrackerConfigPrivate::limits_max);

    connect(p->cacheNumbersSpinBox,
            &QSpinBox::textChanged,
            [this](QString text) { return setCacheLimits(text.toInt()); });

    auto* hSpacer =
      new QSpacerItem(40, 20, QSizePolicy::Preferred, QSizePolicy::Minimum);
    firstHBoxLayout->addItem(hSpacer);
    firstHBoxLayout->addWidget(p->cacheNumbersSpinBox);
    firstHBoxLayout->addWidget(spinboxLabel);

    auto* choosePathLayout = new QHBoxLayout();
    vboxLayout->addLayout(choosePathLayout);

    p->backTrackerPath = new QLineEdit();
    p->backTrackerPath->setReadOnly(true);
    p->backTrackerPath->setPlaceholderText(
      tr("choose your path to save backtracking cache"));
    p->backTrackerPath->setToolTip(
      tr("choose your path to save backtracking cache\ndefaults to: %1")
        .arg(getCachePath()));
    p->backTrackerPath->setStyleSheet(R"(
    border-right-width: 0px;
    )");

    choosePathLayout->addWidget(p->backTrackerPath);

    p->rollbackBtn = new QPushButton(tr("↩️"));
    p->rollbackBtn->setFixedWidth(20);
    p->rollbackBtn->setToolTip(tr("set path to default"));
    p->rollbackBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    p->rollbackBtn->setStyleSheet(QString(R"(
   QPushButton{
         margin-right: 14px;
        // margin-top: 0px;
        // margin-bottom: 0px;
        border-width: 0px;
   }
    )"));
    connect(p->rollbackBtn,
            &QPushButton::clicked,
            this,
            &BackTrackerConfigGroup::pathRollBack);
    choosePathLayout->addWidget(p->rollbackBtn);
    choosePathLayout->setSpacing(0);

    p->choosePathBtn = new QPushButton(tr("Change.."));
    choosePathLayout->addWidget(p->choosePathBtn);
    connect(p->choosePathBtn,
            &QPushButton::clicked,
            this,
            &BackTrackerConfigGroup::browseFolder);

    if (ConfigHandler().backtrackingCachePath().isEmpty()) {
        ConfigHandler().setBacktrackingCachePath(getCachePath());
    }
    p->backTrackerPath->setText(ConfigHandler().backtrackingCachePath());
}
QString BackTrackerConfigGroup::chooseFolder(const QString& defaultPath)
{
    QString path = defaultPath;
    if (defaultPath.isEmpty()) {
        path = getCachePath();
    }
    path = QFileDialog::getExistingDirectory(
      this,
      tr("Choose a Folder"),
      path,
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (path.isEmpty()) {
        return path;
    }

    if (!QFileInfo(path).isWritable()) {
        QMessageBox::about(
          this, tr("Error"), tr("Unable to write to directory."));
        return {};
    }

    return path;
}
void BackTrackerConfigGroup::browseFolder()
{
    auto targetFolder = chooseFolder(ConfigHandler().backtrackingCachePath());
    if (targetFolder.isEmpty()) {
        AbstractLogger::error()
          << "backtracking: cache folder path you choose is empty";
        return;
    }
    p->backTrackerPath->setText(targetFolder);
    ConfigHandler().setBacktrackingCachePath(targetFolder);
}
void BackTrackerConfigGroup::setEnable(bool value)
{
    ConfigHandler().setBacktrackingEnable(value);
}
void BackTrackerConfigGroup::pathRollBack()
{
    ConfigHandler().setBacktrackingCachePath(getCachePath());
    p->backTrackerPath->setText(getCachePath());
}
void BackTrackerConfigGroup::setCacheLimits(int val)
{
    if (val > BackTrackerConfigPrivate::limits_max ||
        val < BackTrackerConfigPrivate::limits_min) {
        return;
    }
    ConfigHandler().setBacktrackingCacheLimits(val);
}
} // btk