// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QWidget>

class QVBoxLayout;
class QPushButton;
class QComboBox;

class TextConfig : public QWidget
{
    Q_OBJECT
public:
    explicit TextConfig(QWidget* parent = nullptr);

    void setFontFamily(const QString& fontFamily);
    void setUnderline(bool underline);
    void setStrikeOut(bool strikeout);
    void setWeight(int weight);
    void setItalic(bool italic);
    void setTextAlignment(Qt::AlignmentFlag alignment);

signals:
    void fontFamilyChanged(const QString& f);
    void fontUnderlineChanged(const bool underlined);
    void fontStrikeOutChanged(const bool dashed);
    void fontWeightChanged(const QFont::Weight w);
    void fontItalicChanged(const bool italic);
    void alignmentChanged(Qt::AlignmentFlag alignment);
public slots:

private slots:
    void weightButtonPressed(bool weight);

private:
    QVBoxLayout* m_layout;
    QComboBox* m_fontsCB;
    QPushButton* m_strikeOutButton;
    QPushButton* m_underlineButton;
    QPushButton* m_weightButton;
    QPushButton* m_italicButton;

    QPushButton* m_leftAlignButton;
    QPushButton* m_centerAlignButton;
    QPushButton* m_rightAlignButton;
};
