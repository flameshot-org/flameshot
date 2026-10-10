// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2024 Flameshot Contributors

#pragma once

#ifdef ENABLE_QR_DECODER

#include <QLabel>
#include <QPushButton>
#include <QRect>
#include <QString>
#include <QTimer>
#include <QUrl>
#include <QWidget>

/**
 * @brief Floating panel widget that displays QR code scan results.
 *
 * QrResultWidget is a child of CaptureWidget. It appears automatically
 * next to the selection area whenever a QR/barcode is detected.
 * It parses the content type (URL, WiFi, vCard, etc.) and shows the
 * appropriate action buttons.
 *
 * The widget is never shown without a valid QR result; call showResult()
 * to display it and hideResult() (or the dismiss button) to close it.
 */
class QrResultWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QrResultWidget(QWidget* parent = nullptr);

    // Content type enum — determines layout / buttons shown
    enum class QrContentType
    {
        URL,
        TEXT,
        WIFI,
        VCARD,
        EMAIL,
        PHONE,
        SMS,
        GEO,
        UNKNOWN
    };

public slots:
    /**
     * @brief Show the result panel next to the given selection area.
     * @param content  Decoded QR text content.
     * @param selectionGeometry  Selection rect in parent widget coordinates.
     */
    void showResult(const QString& content, const QRect& selectionGeometry);

    /** @brief Hide and reset the result panel. */
    void hideResult();

signals:
    void dismissed();
    void copyRequested(const QString& text);
    void openUrlRequested(const QUrl& url);

private slots:
    void onCopyClicked();
    void onOpenClicked();
    void onCloseClicked();

private:
    // --- widgets ---
    QLabel*       m_typeLabel;
    QLabel*       m_contentLabel;
    QPushButton*  m_copyButton;
    QPushButton*  m_openButton;    // only shown for URL / EMAIL / PHONE
    QPushButton*  m_closeButton;

    // --- state ---
    QString          m_rawContent;
    QrContentType    m_contentType;
    QTimer*          m_autoHideTimer;

    // --- helpers ---
    QrContentType detectContentType(const QString& content) const;
    QString       formatContent(const QString& content, QrContentType type) const;
    QString       contentTypeLabel(QrContentType type) const;
    void          positionRelativeToSelection(const QRect& selection);
    void          applyStyleSheet();
    void          updateButtons();
};

#endif // ENABLE_QR_DECODER
