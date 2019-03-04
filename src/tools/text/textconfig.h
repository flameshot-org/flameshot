// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
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

class QVBoxLayout;
class QPushButton;

class TextConfig : public QWidget {
    Q_OBJECT
public:
    explicit TextConfig(QWidget *parent = nullptr);

    void setUnderline(const bool u);
    void setStrikeOut(const bool s);
    void setWeight(const int w);
    void setItalic(const bool i);

signals:
    void fontFamilyChanged(const QString &f);
    void fontUnderlineChanged(const bool underlined);
    void fontStrikeOutChanged(const bool dashed);
    void fontWeightChanged(const QFont::Weight w);
    void fontItalicChanged(const bool italic);

public slots:

private slots:
    void weightButtonPressed(const bool w);

private:
    QVBoxLayout *m_layout;
    QPushButton *m_strikeOutButton;
    QPushButton *m_underlineButton;
    QPushButton *m_weightButton;
    QPushButton *m_italicButton;
};
