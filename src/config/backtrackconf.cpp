#include "backtrackconf.h"

#include "abstractlogger.h"

#include <QVBoxLayout>
#include <QtCore/qstandardpaths.h>
#include <QtCore/qstring.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qgroupbox.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlayoutitem.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qsizepolicy.h>
#include <QtWidgets/qspinbox.h>
#include <QtWidgets/qwidget.h>
#include <cacheutils.h>
#include <confighandler.h>

class BacktrackConfigPrivate
{
    friend class BacktrackConfigGroup;
    constexpr static int limits_max = 200;
    constexpr static int limits_min = 0;

public:
    QCheckBox* enableCheckBox;
    QLineEdit* cachePath;
    QPushButton* choosePathBtn;
    QPushButton* setCachePath2DefaultBtn;
    QSpinBox* cacheNumbersSpinBox;
};

BacktrackConfigGroup::BacktrackConfigGroup(QWidget* parent)
  : QGroupBox(tr("BackTracking"), parent)
  , p(new BacktrackConfigPrivate)
  , m_configHandler(ConfigHandler::getInstance())
{
    init();
}
BacktrackConfigGroup::~BacktrackConfigGroup()
{
    p = nullptr;
}
void BacktrackConfigGroup::init()
{
    p->enableCheckBox = new QCheckBox(tr("enable"), this);
    p->enableCheckBox->setToolTip(tr("If enable the backtraking"));
    p->enableCheckBox->setChecked(ConfigHandler().backtrackEnable());
    connect(p->enableCheckBox,
            &QCheckBox::clicked,
            this,
            &BacktrackConfigGroup::setEnable);
    setFlat(true);
    auto* vboxLayout = new QVBoxLayout();
    setLayout(vboxLayout);

    auto* firstHBoxLayout = new QHBoxLayout();
    vboxLayout->addLayout(firstHBoxLayout);
    firstHBoxLayout->addWidget(p->enableCheckBox);

    auto* spinboxLabel = new QLabel(tr("cache size"));
    spinboxLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    p->cacheNumbersSpinBox = new QSpinBox();
    p->cacheNumbersSpinBox->setValue(
      m_configHandler->backtrackCacheLimits());
    p->cacheNumbersSpinBox->setFixedWidth(50);
    p->cacheNumbersSpinBox->setMinimum(BacktrackConfigPrivate::limits_min);
    p->cacheNumbersSpinBox->setMaximum(BacktrackConfigPrivate::limits_max);

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

    p->cachePath = new QLineEdit();
    p->cachePath->setReadOnly(true);
    p->cachePath->setPlaceholderText(
      tr("choose your path to save backtracking cache"));
    p->cachePath->setToolTip(
      tr("choose your path to save backtracking cache\ndefaults to: %1")
        .arg(getCachePath()));
    p->cachePath->setStyleSheet(R"(
    border-right-width: 0px;
    )");

    choosePathLayout->addWidget(p->cachePath);

    p->setCachePath2DefaultBtn = new QPushButton(tr("↩️"));
    p->setCachePath2DefaultBtn->setFixedWidth(20);
    p->setCachePath2DefaultBtn->setToolTip(tr("set path to default"));
    p->setCachePath2DefaultBtn->setSizePolicy(QSizePolicy::Fixed,
                                              QSizePolicy::Preferred);
    p->setCachePath2DefaultBtn->setStyleSheet(QString(R"(
   QPushButton{
         margin-right: 14px;
        // margin-top: 0px;
        // margin-bottom: 0px;
        border-width: 0px;
   }
    )"));
    connect(p->setCachePath2DefaultBtn,
            &QPushButton::clicked,
            this,
            &BacktrackConfigGroup::setPath2Default);
    choosePathLayout->addWidget(p->setCachePath2DefaultBtn);
    choosePathLayout->setSpacing(0);

    p->choosePathBtn = new QPushButton(tr("Change.."));
    choosePathLayout->addWidget(p->choosePathBtn);
    connect(p->choosePathBtn,
            &QPushButton::clicked,
            this,
            &BacktrackConfigGroup::browseFolder);

    if (ConfigHandler::getInstance()->backtrackCachePath().isEmpty()) {
        ConfigHandler::getInstance()->setBacktrackCachePath(getCachePath());
    }
    p->cachePath->setText(ConfigHandler::getInstance()->backtrackCachePath());
}
QString BacktrackConfigGroup::chooseFolder(const QString& defaultPath)
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
void BacktrackConfigGroup::browseFolder()
{
    auto targetFolder = chooseFolder(ConfigHandler().backtrackCachePath());
    if (targetFolder.isEmpty()) {
        AbstractLogger::error()
          << "backtracking: cache folder path you choose is empty";
        return;
    }
    p->cachePath->setText(targetFolder);
    ConfigHandler().setBacktrackCachePath(targetFolder);
}
void BacktrackConfigGroup::setEnable(bool value)
{
    m_configHandler->setBacktrackEnable(value);
}
void BacktrackConfigGroup::setPath2Default()
{
    m_configHandler->setBacktrackCachePath(getCachePath());
    p->cachePath->setText(getCachePath());
}
void BacktrackConfigGroup::setCacheLimits(int val)
{
    if (val > BacktrackConfigPrivate::limits_max ||
        val < BacktrackConfigPrivate::limits_min) {
        return;
    }
    m_configHandler->setBacktrackCacheLimits(val);
}