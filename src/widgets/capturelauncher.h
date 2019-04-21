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

#pragma once

#include <QWidget>

class QCheckBox;
class QPushButton;
class QVBoxLayout;
class QComboBox;
class QSpinBox;
class QLabel;
class ImageLabel;

class CaptureLauncher : public QWidget
{
    Q_OBJECT
public:
    explicit CaptureLauncher(QWidget *parent = nullptr);

private slots:
    void startCapture();
    void startDrag();
    void captureTaken(uint id, QPixmap p);
    void captureFailed(uint id);

private:

    QSpinBox *m_delaySpinBox;
    QComboBox *m_captureType;
    QVBoxLayout *m_mainLayout;
    QPushButton *m_launchButton;
    QLabel *m_CaptureModeLabel;
    ImageLabel *m_imageLabel;
    uint m_id;
};
