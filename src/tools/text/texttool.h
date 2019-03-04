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

#include "src/tools/capturetool.h"
#include <QPointer>

class TextWidget;
class TextConfig;

class TextTool : public CaptureTool {
    Q_OBJECT
public:
    explicit TextTool(QObject *parent = nullptr);

    bool isValid() const override;
    bool closeOnButtonPressed() const override;
    bool isSelectable() const override;
    bool showMousePreview() const override;

    QIcon icon(const QColor &background,
                       bool inEditor) const override;
    QString name() const override;
    static QString nameID();
    QString description() const override;

    QWidget* widget() override;
    QWidget* configurationWidget() override;
    CaptureTool* copy(QObject *parent = nullptr) override;

    void undo(QPixmap &pixmap) override;
    void process(
            QPainter &painter, const QPixmap &pixmap, bool recordUndo = false) override;
    void paintMousePreview(QPainter &painter, const CaptureContext &context) override;

public slots:
    void drawEnd(const QPoint &p) override;
    void drawMove(const QPoint &p) override;
    void drawStart(const CaptureContext &context) override;
    void pressed(const CaptureContext &context) override;
    void colorChanged(const QColor &c) override;
    void thicknessChanged(const int th) override;

private slots:
    void updateText(const QString &s);
    void setFont(const QFont &f);
    void updateFamily(const QString &s);
    void updateFontUnderline(const bool underlined);
    void updateFontStrikeOut(const bool s);
    void updateFontWeight(const QFont::Weight w);
    void updateFontItalic(const bool italic);

private:
    QFont m_font;
    QString m_text;
    int m_size;
    QColor m_color;
    QPixmap m_pixmapBackup;
    QRect m_backupArea;
    QPointer<TextWidget> m_widget;
    QPointer<TextConfig> m_confW;
};
