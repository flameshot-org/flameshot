#include "src/config/configresolver.h"
#include "src/config/configerrordetails.h"
#include "src/utils/confighandler.h"

#include "src/utils/valuehandler.h"
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

ConfigResolver::ConfigResolver(QWidget* parent)
  : QDialog(parent)
{
    setWindowTitle("Resolve configuration errors");
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
    int i = 0;

    // No errors detected
    if (!anyErrors) {
        accept();
    } else {
        layout()->addWidget(
          new QLabel("<b>You must resolve all errors before continuing:</b>"),
          0,
          0,
          1,
          2,
          Qt::AlignCenter);
        ++i;
    }

    // List semantically incorrect settings with a "Reset" button
    for (const auto& key : semanticallyWrong) {
        auto* label = new QLabel(key);
        auto* reset = new QPushButton("Reset");
        reset->setToolTip("Reset to the default value.");
        layout()->addWidget(label, i, 0, Qt::AlignRight);
        layout()->addWidget(reset, i, 1);
        reset->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        connect(reset, &QPushButton::clicked, this, [key]() {
            ConfigHandler().resetValue(key);
        });

        ++i;
    }
    // List unrecognized settings with a "Remove" button
    for (const auto& key : unrecognized) {
        auto* label = new QLabel(key);
        auto* remove = new QPushButton("Remove");
        remove->setToolTip("Remove this setting.");
        layout()->addWidget(label, i, 0, Qt::AlignRight);
        layout()->addWidget(remove, i, 1);
        connect(remove, &QPushButton::clicked, this, [key]() {
            ConfigHandler().remove(key);
        });
        ++i;
    }

    if (!config.checkShortcutConflicts()) {
        auto* conflicts =
          new QLabel("Some keyboard shortcuts have conflicts.\n"
                     "This will NOT prevent flameshot from starting.\n"
                     "Please solve them manually in the configuration file.");
        conflicts->setWordWrap(true);
        conflicts->setMaximumWidth(geometry().width());
        conflicts->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
        layout()->addWidget(conflicts, i, 0, 1, 2, Qt::AlignCenter);
        ++i;
    }

    using BBox = QDialogButtonBox;

    // Add button box at the bottom
    auto* buttons = new BBox(this);
    layout()->addWidget(buttons, i, 0, 1, 2, Qt::AlignCenter);
    if (anyErrors) {
        QPushButton* resolveAll = new QPushButton("Resolve all");
        resolveAll->setToolTip("Resolve all listed errors.");
        buttons->addButton(resolveAll, BBox::ResetRole);
        connect(resolveAll, &QPushButton::clicked, this, [=]() {
            for (const auto& key : semanticallyWrong)
                ConfigHandler().resetValue(key);
            for (const auto& key : unrecognized)
                ConfigHandler().remove(key);
        });
    }

    QPushButton* details = new QPushButton("Details");
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
