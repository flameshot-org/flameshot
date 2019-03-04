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

#include <QTextEdit>

class TextWidget : public QTextEdit
{
    Q_OBJECT
public:
    explicit TextWidget(QWidget *parent = nullptr);

    void adjustSize();
    void setFont(const QFont &f);

protected:
    void showEvent(QShowEvent *e);
    void resizeEvent(QResizeEvent *e);

signals:
    void textUpdated(const QString &s);

public slots:
    void updateFont(const QFont &f);
    void setTextColor(const QColor &c);
    void setFontPointSize(qreal s);

private slots:
    void emitTextUpdated();

private:
    QSize m_baseSize;
    QSize m_minSize;
};
