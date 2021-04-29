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
    ~TextTool();

    bool isValid() const override;
    bool closeOnButtonPressed() const override;
    bool isSelectable() const override;
    bool showMousePreview() const override;

    QIcon icon(const QColor& background, bool inEditor) const override;
    QString name() const override;
    QString description() const override;
    QString info() override;

    QWidget* widget() override;
    QWidget* configurationWidget() override;
    CaptureTool* copy(QObject* parent = nullptr) override;

    void process(QPainter& painter, const QPixmap& pixmap) override;
    void paintMousePreview(QPainter& painter,
                           const CaptureContext& context) override;
    void move(const QPoint& pos) override;
    const QPoint* pos() override;
    void drawObjectSelection(QPainter& painter) override;

    void setEditMode(bool b) override;
    bool isChanged() override;

protected:
    void copyParams(const TextTool* from, TextTool* to);
    ToolType nameID() const override;

public slots:
    void drawEnd(const QPoint& p) override;
    void drawMove(const QPoint& p) override;
    void drawStart(const CaptureContext& context) override;
    void pressed(const CaptureContext& context) override;
    void colorChanged(const QColor& c) override;
    void thicknessChanged(int th) override;
    virtual int thickness() override { return m_size; };

private slots:
    void updateText(const QString& s);
    void updateFamily(const QString& s);
    void updateFontUnderline(const bool underlined);
    void updateFontStrikeOut(const bool s);
    void updateFontWeight(const QFont::Weight w);
    void updateFontItalic(const bool italic);

private:
    void closeEditor();

    QFont m_font;
    QString m_text;
    QString m_textOld;
    int m_size;
    QColor m_color;
    QRect m_textArea;
    QPointer<TextWidget> m_widget;
    QPointer<TextConfig> m_confW;
    QPoint m_currentPos;

    QString m_tempString;
};
