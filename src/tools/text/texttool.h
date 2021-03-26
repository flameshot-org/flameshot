// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "src/tools/capturetool.h"
#include <QPoint>
#include <QPointer>

class TextWidget;
class TextConfig;

class TextTool : public CaptureTool
{
    Q_OBJECT
public:
    explicit TextTool(QObject* parent = nullptr);

    bool isValid() const override;
    bool closeOnButtonPressed() const override;
    bool isSelectable() const override;
    bool showMousePreview() const override;

    QIcon icon(const QColor& background, bool inEditor) const override;
    QString name() const override;
    QString description() const override;

    QWidget* widget() override;
    QWidget* configurationWidget() override;
    CaptureTool* copy(QObject* parent = nullptr) override;

    void process(QPainter& painter,
                 const QPixmap& pixmap,
                 bool recordUndo = false) override;
    void paintMousePreview(QPainter& painter,
                           const CaptureContext& context) override;
    void move(const QPoint& pos) override;
    const QPoint* pos() override;

protected:
    ToolType nameID() const override;
    QRect backupRect(const QPixmap& pixmap) const;

public slots:
    void drawEnd(const QPoint& p) override;
    void drawMove(const QPoint& p) override;
    void drawStart(const CaptureContext& context) override;
    void pressed(const CaptureContext& context) override;
    void colorChanged(const QColor& c) override;
    void thicknessChanged(const int th) override;

private slots:
    void updateText(const QString& s);
    void setFont(const QFont& f);
    void updateFamily(const QString& s);
    void updateFontUnderline(const bool underlined);
    void updateFontStrikeOut(const bool s);
    void updateFontWeight(const QFont::Weight w);
    void updateFontItalic(const bool italic);

private:
    QFont m_font;
    QString m_text;
    int m_size;
    QColor m_color;
    QRect m_backupArea;
    QPointer<TextWidget> m_widget;
    QPointer<TextConfig> m_confW;
    QPoint m_currentPos;
};
