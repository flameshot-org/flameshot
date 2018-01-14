// Copyright(c) 2017-2018 Alejandro Sirgo Rica & Contributors
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

#include "resourceexporter.h"
#include "src/capture/workers/imgur/imguruploader.h"
#include "src/capture/workers/screenshotsaver.h"
#include "src/capture/workers/launcher/openwithprogram.h"

ResourceExporter::ResourceExporter() {

}

void ResourceExporter::captureToClipboard(const QPixmap &p) {
    ScreenshotSaver().saveToClipboard(p);
}

void ResourceExporter::captureToFile(const QPixmap &p, const QString &path) {
    ScreenshotSaver().saveToFilesystem(p, path);
}

void ResourceExporter::captureToFileUi(const QPixmap &p) {
    ScreenshotSaver().saveToFilesystemGUI(p);
}

void ResourceExporter::captureToImgur(const QPixmap &p) {
    auto w = new ImgurUploader(p);
    w->show();
}

void ResourceExporter::captureToProgram(const QPixmap &p) {
    showOpenWithMenu(p);
}
