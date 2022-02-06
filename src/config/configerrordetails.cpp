#include "src/config/configerrordetails.h"

#include "src/utils/abstractlogger.h"
#include "src/utils/confighandler.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QTextEdit>
#include <QVBoxLayout>

ConfigErrorDetails::ConfigErrorDetails(QWidget* parent)
  : QDialog(parent)
{
    // Generate error log message
    QString str;
    AbstractLogger stream(str, AbstractLogger::Error);
    ConfigHandler().checkForErrors(&stream);

    // Set up dialog
    setWindowTitle(tr("Configuration errors"));
    setLayout(new QVBoxLayout(this));

    // Add text display
    auto* textDisplay = new QTextEdit(this);
    textDisplay->setPlainText(str);
    textDisplay->setReadOnly(true);
    layout()->addWidget(textDisplay);

    // Add Ok button
    using BBox = QDialogButtonBox;
    BBox* buttons = new BBox(BBox::Ok);
    layout()->addWidget(buttons);
    connect(buttons, &BBox::clicked, this, [this]() { close(); });

    show();

    qApp->processEvents();
    QPoint center = geometry().center();
    QRect dialogRect(0, 0, 600, 400);
    dialogRect.moveCenter(center);
    setGeometry(dialogRect);
}
