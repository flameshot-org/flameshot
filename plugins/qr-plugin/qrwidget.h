// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#pragma once

#include <QPixmap>
#include <QString>
#include <QWidget>

class QLabel;
class QPlainTextEdit;
class QPushButton;
class QTimer;

class QrWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QrWidget(const QString& scannedContent,
                      const QString& symbolType,
                      const QPixmap& capturePixmap,
                      QWidget* parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void openUrl();
    void copyText();
    void copyCaptureImage();
    void showToast(const QString& msg);

private:
    void setupUi();
    void applyTheme();

    QString m_content;
    QString m_symbolType;
    QPixmap m_capturePixmap;

    QLabel* m_previewLabel{ nullptr };
    QLabel* m_typeBadge{ nullptr };
    QLabel* m_toastLabel{ nullptr };
    QPlainTextEdit* m_textEdit{ nullptr };
    QPushButton* m_openBtn{ nullptr };
    QTimer* m_toastTimer{ nullptr };

    bool m_isDragging{ false };
    QPoint m_dragPos;
};
