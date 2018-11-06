// Copyright(c) 2017-2018 Alejandro Sirgo Rica & Contributors
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

#include <QWidget>
#include <QMessageBox>
#include <QInputDialog>
#include <QDesktopServices>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QVariant>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QMap>
#include <QList>
#include <QPair>

#include "src/utils/imgurconfighandler.h"

class ImgurConf : public QWidget
{
    Q_OBJECT
public:
    explicit ImgurConf(QWidget *parent = nullptr);

signals:
    void settingsChanged();

public slots:
    void authorize(bool force = false);
    void deauthorize();
    void refreshToken();

private slots:
    void handleReply(QNetworkReply *reply);

private:
    ImgurConfigHandler config;
    QNetworkAccessManager *m_networkManager;
    QVBoxLayout *m_widgetLayout;
    QLabel *m_userField;
    QLineEdit *m_albumField;
    QCheckBox *m_anonymousUpload;
    QLineEdit *m_clientIdField;
    QLineEdit *m_clientSecretField;
    QPushButton *m_logInButton;
    QPushButton *m_logOutButton;
    QPushButton *m_saveButton;

    void initWidgets();
    void updateImgurSettings();
    void updateComponents();
    void extractToken(QString &token_url);
};
