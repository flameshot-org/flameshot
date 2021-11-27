// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QScrollArea>
#include <QWidget>

class QVBoxLayout;
class QHBoxLayout;
class QRadioButton;
class QLineEdit;

class CloudConf : public QWidget
{
    Q_OBJECT
public:
    explicit CloudConf(QWidget* parent = nullptr);

public slots:
    void updateComponents();

private slots:
    void cloudUploadSetToImgur(bool checked);
    void cloudUploadSetToDroplr(bool checked);
    void setDroplrUsername(const QString& username);
    void setDroplrPassword(const QString& password);

private:
    const QString chooseFolder(const QString currentPath = "");

    void initCredentialHolder();

    void _updateComponents(bool allowEmptySavePath);

    // class members
    QVBoxLayout* m_layout;
    QHBoxLayout* cloudGroupLayout;
    QRadioButton* m_imgurUploadButton;
    QRadioButton* m_droplrUploadButton;
    QLineEdit* m_droplrUsernameEditor;
    QLineEdit* m_droplrPasswordEditor;
};
