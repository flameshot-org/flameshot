// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "src/tools/capturetool.h"
#include "textconfig.h"
#include <QPoint>
#include <QPointer>
class TextWidget;
class TextConfig;

class TextTool : public CaptureTool
{
    Q_OBJECT
public:
    explicit TextTool(QObject* parent = nullptr);
    ~TextTool() override;

    [[nodiscard]] bool isValid() const override;
    [[nodiscard]] bool closeOnButtonPressed() const override;
    [[nodiscard]] bool isSelectable() const override;
    [[nodiscard]] bool showMousePreview() const override;
    [[nodiscard]] QRect boundingRect() const override;

    [[nodiscard]] QIcon icon(const QColor& background,
                             bool inEditor) const override;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString description() const override;
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

    void setEditMode(bool editMode) override;
    bool isChanged() override;

protected:
    void copyParams(const TextTool* from, TextTool* to);
    [[nodiscard]] CaptureTool::Type type() const override;

public slots:
    void drawEnd(const QPoint& point) override;
    void drawMove(const QPoint& point) override;
    void drawStart(const CaptureContext& context) override;
    void pressed(CaptureContext& context) override;
    void onColorChanged(const QColor& color) override;
    void onSizeChanged(int size) override;
    int size() const override { return m_size; };

private slots:
    void updateText(const QString& string);
    void updateFamily(const QString& string);
    void updateFontUnderline(bool underlined);
    void updateFontStrikeOut(bool strikeout);
    void updateFontWeight(QFont::Weight weight);
    void updateFontItalic(bool italic);
    void updateAlignment(Qt::AlignmentFlag alignment);

private:
    void closeEditor();

    QFont m_font;
    Qt::AlignmentFlag m_alignment;
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
