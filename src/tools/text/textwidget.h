// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QTextEdit>

class QEvent;
class QKeyEvent;
class QGraphicsDropShadowEffect;

class TextWidget : public QTextEdit
{
    Q_OBJECT
public:
    explicit TextWidget(QWidget* parent = nullptr);

    void adjustSize();
    void setFont(const QFont& f);

protected:
    bool event(QEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
    void showEvent(QShowEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;

signals:
    void textUpdated(const QString& s);
    void editingFinished();

public slots:
    void setTextColor(const QColor& c);
    void setAlignment(Qt::AlignmentFlag alignment);
    void setShadow(bool shadow);

private slots:
    void emitTextUpdated();

private:
    QSize m_baseSize;
    QSize m_minSize;
    QGraphicsDropShadowEffect* m_shadowEffect = nullptr;
    QColor m_textColor = Qt::black;
};
