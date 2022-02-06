#include "src/config/configresolver.h"
#include "src/config/configerrordetails.h"
#include "src/utils/confighandler.h"

#include "src/utils/valuehandler.h"
#include <QDialogButtonBox>
#include <QLabel>
#include <QSplitter>
#include <QVBoxLayout>

ConfigResolver::ConfigResolver(QWidget* parent)
  : QDialog(parent)
{
    setWindowTitle(tr("Resolve configuration errors"));
    setMinimumSize({ 250, 200 });
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    populate();
    connect(ConfigHandler::getInstance(),
            &ConfigHandler::fileChanged,
            this,
            [this]() { populate(); });
}

QGridLayout* ConfigResolver::layout()
{
    return dynamic_cast<QGridLayout*>(QDialog::layout());
}

void ConfigResolver::populate()
{
    ConfigHandler config;
    QList<QString> unrecognized;
    QList<QString> semanticallyWrong;

    config.checkUnrecognizedSettings(nullptr, &unrecognized);
    config.checkSemantics(nullptr, &semanticallyWrong);

    // Remove previous layout and children, if any
    resetLayout();

    bool anyErrors = !semanticallyWrong.isEmpty() || !unrecognized.isEmpty();
    int row = 0;

    // No errors detected
    if (!anyErrors) {
        accept();
    } else {
        layout()->addWidget(
          new QLabel(
            tr("<b>You must resolve all errors before continuing:</b>")),
          0,
          0,
          1,
          2);
        ++row;
    }

    // List semantically incorrect settings with a "Reset" button
    for (const auto& key : semanticallyWrong) {
        auto* label = new QLabel(key);
        auto* reset = new QPushButton(tr("Reset"));
        label->setToolTip("This setting has a bad value.");
        reset->setToolTip(tr("Reset to the default value."));
        layout()->addWidget(label, row, 0);
        layout()->addWidget(reset, row, 1);
        reset->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        connect(reset, &QPushButton::clicked, this, [key]() {
            ConfigHandler().resetValue(key);
        });

        ++row;
    }
    // List unrecognized settings with a "Remove" button
    for (const auto& key : unrecognized) {
        auto* label = new QLabel(key);
        auto* remove = new QPushButton(tr("Remove"));
        label->setToolTip("This setting is unrecognized.");
        remove->setToolTip(tr("Remove this setting."));
        layout()->addWidget(label, row, 0);
        layout()->addWidget(remove, row, 1);
        connect(remove, &QPushButton::clicked, this, [key]() {
            ConfigHandler().remove(key);
        });
        ++row;
    }

    if (!config.checkShortcutConflicts()) {
        auto* conflicts = new QLabel(
          tr("Some keyboard shortcuts have conflicts.\n"
             "This will NOT prevent flameshot from starting.\n"
             "Please solve them manually in the configuration file."));
        conflicts->setWordWrap(true);
        conflicts->setMaximumWidth(geometry().width());
        conflicts->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
        layout()->addWidget(conflicts, row, 0, 1, 2, Qt::AlignCenter);
        ++row;
    }

    auto* separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    layout()->addWidget(separator, row, 0, 1, 2);
    ++row;

    using BBox = QDialogButtonBox;

    // Add button box at the bottom
    auto* buttons = new BBox(this);
    layout()->addWidget(buttons, row, 0, 1, 2, Qt::AlignCenter);
    if (anyErrors) {
        auto* resolveAll = new QPushButton(tr("Resolve all"));
        resolveAll->setToolTip(tr("Resolve all listed errors."));
        buttons->addButton(resolveAll, BBox::ResetRole);
        connect(resolveAll, &QPushButton::clicked, this, [=]() {
            for (const auto& key : semanticallyWrong) {
                ConfigHandler().resetValue(key);
            }
            for (const auto& key : unrecognized) {
                ConfigHandler().remove(key);
            }
        });
    }

    auto* details = new QPushButton(tr("Details"));
    buttons->addButton(details, BBox::HelpRole);
    connect(details, &QPushButton::clicked, this, [this]() {
        (new ConfigErrorDetails(this))->exec();
    });

    buttons->addButton(BBox::Cancel);

    connect(buttons, &BBox::rejected, this, [this]() { reject(); });
}

void ConfigResolver::resetLayout()
{
    for (auto* child : children()) {
        child->deleteLater();
    }
    delete layout();
    setLayout(new QGridLayout());
    layout()->setSizeConstraint(QLayout::SetFixedSize);
}
