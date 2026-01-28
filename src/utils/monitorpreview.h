// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Jeremy Borgman & Contributors

#pragma once

#include <QWidget>

class QScreen;
class QPixmap;
class QLabel;
class QEnterEvent;

class MonitorPreview : public QWidget
{
    Q_OBJECT
public:
    MonitorPreview(int monitorIndex,
                   QScreen* screen,
                   const QPixmap& thumbnail,
                   QWidget* parent = nullptr);

    int monitorIndex() const { return m_monitorIndex; }

signals:
    void monitorSelected(int index);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    int m_monitorIndex;
    QColor m_uiColor;
    QColor m_contrastColor;
    QLabel* m_textLabel;
};
