// Copyright 2017 Alejandro Sirgo Rica
//
// This file is part of Flameshot.
//
//     Flameshot is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Flameshot is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Flameshot.  If not, see <http://www.gnu.org/licenses/>.

#include "graphicalscreenshotsaver.h"
#include "src/utils/confighandler.h"
#include "src/utils/systemnotification.h"
#include "src/utils/filenamehandler.h"
#include <QFileDialog>
#include <QImageWriter>
#include <QMessageBox>
#include <QShortcut>
#include <QVBoxLayout>

/*
 * Añadir la captura de pantalla a la derecha y boton de copiar
 *
 */

GraphicalScreenshotSaver::GraphicalScreenshotSaver(const QPixmap &capture,
                                                   QWidget *parent) :
    QWidget(parent), m_pixmap(capture)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(QObject::tr("Save As"));

    new QShortcut(Qt::Key_Escape, this, SLOT(close()));

    m_layout = new QVBoxLayout(this);
    m_fileDialog = new QFileDialog();
    initFileDialog();
    m_layout->addWidget(m_fileDialog);

}

void GraphicalScreenshotSaver::initFileDialog() {
    m_fileDialog->setOption(QFileDialog::DontUseNativeDialog, true);
    m_fileDialog->setFileMode(QFileDialog::AnyFile);
    m_fileDialog->setAcceptMode(QFileDialog::AcceptSave);
    QString fileName, directory;
    FileNameHandler().absoluteSavePath(directory, fileName);
    m_fileDialog->selectFile(fileName);
    m_fileDialog->setDirectory(directory);

    QStringList mimeTypes;
    for (const QByteArray &bf: QImageWriter::supportedMimeTypes())
        mimeTypes.append(QLatin1String(bf));
    m_fileDialog->setMimeTypeFilters(mimeTypes);
    m_fileDialog->selectMimeTypeFilter("image/png");
    m_fileDialog->setDefaultSuffix("png");

    connect(m_fileDialog, &QFileDialog::rejected,
            this, &GraphicalScreenshotSaver::close);
    connect(m_fileDialog, &QFileDialog::accepted,
            this, &GraphicalScreenshotSaver::checkSaveAcepted);
}

void GraphicalScreenshotSaver::showErrorMessage(const QString &msg) {
    QMessageBox saveErrBox(
                QMessageBox::Warning,
                QObject::tr("Save Error"),
                msg);
    saveErrBox.setWindowIcon(QIcon(":img/flameshot.png"));
    saveErrBox.exec();
}

void GraphicalScreenshotSaver::checkSaveAcepted() {
    m_fileDialog->show();
    QString path = m_fileDialog->selectedFiles().first();
    bool ok = m_pixmap.save(path);
    if (ok) {
        QString pathNoFile = path.left(path.lastIndexOf("/"));
        ConfigHandler().setSavePath(pathNoFile);
        QString msg = QObject::tr("Capture saved as ") + path;
        SystemNotification().sendMessage(msg);
        close();
    } else {
        QString msg = QObject::tr("Error trying to save as ") + path;
        showErrorMessage(msg);
    }
}
