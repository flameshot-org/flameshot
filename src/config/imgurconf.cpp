#include "imgurconf.h"

ImgurConf::ImgurConf(QWidget *parent) : QWidget(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &ImgurConf::handleReply);

    initWidgets();
    updateComponents();
}

void ImgurConf::authorize(bool force)
{
    if (config.isAuthorized() && !force) {
        return;
    }

    QString clientId = config.getSetting(QStringLiteral("Api/client_id"), "").toString();
    QString clientSecret = config.getSetting(QStringLiteral("Api/client_secret"), "").toString();

    if (clientId.isEmpty() || clientSecret.isEmpty()) {
        return;
    }

    QUrlQuery urlQuery;
    urlQuery.addQueryItem(QStringLiteral("client_id"), clientId);
    urlQuery.addQueryItem(QStringLiteral("response_type"), QStringLiteral("token"));

    QUrl url(QStringLiteral("https://api.imgur.com/oauth2/authorize"));
    url.setQuery(urlQuery);

    // Authorization URL
    QDesktopServices::openUrl(url);

    // User clicked OK/Yes in the message box
    bool accept;

    // Authorization response
    QString token_url = QInputDialog::getText(this, tr("Imgur token"), tr("Token URL"),
        QLineEdit::Normal, QString(), &accept);

    if (accept && token_url.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Token URL can't be empty."));
        return;
    }

    // Generate Imgur token
    extractToken(token_url);
}

void ImgurConf::deauthorize()
{
    if (!config.isAuthorized()) {
        return;
    }

    int button = QMessageBox::warning(
        this,
        tr("Delete Imgur data"),
        tr("This will delete all Imgur data including API credentials and authentication token. You won't be able to recover said data so please be certain.<br><br>Are you sure you want to delete all Imgur data?"),
        QMessageBox::Yes|QMessageBox::No,
        QMessageBox::No
    );

    if (button != QMessageBox::Yes) {
        return;
    }

    m_clientIdField->setText(QLatin1String(""));
    m_clientSecretField->setText(QLatin1String(""));
    m_albumField->setText(QLatin1String(""));
    m_anonymousUpload->setChecked(false);

    updateImgurSettings();
    emit settingsChanged();
}

void ImgurConf::refreshToken()
{
    QMap<QString, QVariant> token = config.getToken();
    QString clientId = config.getSetting(QStringLiteral("Api/client_id")).toString();
    QString clientSecret = config.getSetting(QStringLiteral("Api/client_secret")).toString();

    QUrl url(QStringLiteral("https://api.imgur.com/oauth2/token"));

    QUrlQuery urlQuery;
    urlQuery.addQueryItem(QStringLiteral("refresh_token"), token.value(QStringLiteral("refresh_token")).toString());
    urlQuery.addQueryItem(QStringLiteral("client_id"), clientId);
    urlQuery.addQueryItem(QStringLiteral("client_secret"), clientSecret);
    urlQuery.addQueryItem(QStringLiteral("grant_type"), QStringLiteral("refresh_token"));

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/application/x-www-form-urlencoded");

    m_networkManager->post(request, urlQuery.toString().toUtf8());
}

void ImgurConf::handleReply(QNetworkReply *reply)
{
    QMap<QString, QVariant> token = config.getToken();

    if (reply->error() != QNetworkReply::NoError) {
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        // Remove invalid token
        if (status == 400) {
            //token.clear();
        }

        QMessageBox::warning(this, tr("Error"), reply->errorString());
    }

    QJsonDocument response = QJsonDocument::fromJson(reply->readAll());
    QJsonObject json = response.object();

    // Update token values
    for (const QString &key : json.keys()) {
        if (!token.contains(key)) {
            continue;
        }

        token.insert(key, json.value(key).toVariant());
    }

    // Save new token
    config.setToken(token);

    emit settingsChanged();
}

void ImgurConf::initWidgets()
{
    m_widgetLayout = new QVBoxLayout(this);

    QGroupBox *imgurApi = new QGroupBox(tr("Imgur API"), this);
    QFormLayout *imgurApiLayout = new QFormLayout(imgurApi);
    m_clientIdField = new QLineEdit(imgurApi);
    m_clientSecretField = new QLineEdit(imgurApi);
    m_clientSecretField->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    imgurApiLayout->addRow(tr("Client ID"), m_clientIdField);
    imgurApiLayout->addRow(tr("Client Secret"), m_clientSecretField);
    imgurApiLayout->itemAt(1, QFormLayout::FieldRole)->widget();
    imgurApi->setLayout(imgurApiLayout);
    m_widgetLayout->addWidget(imgurApi);

    QGroupBox *imgurAccount = new QGroupBox(tr("Imgur Account"), this);
    QFormLayout *imgurAccountLayout = new QFormLayout(imgurAccount);
    m_userField = new QLabel(tr("N/A"), imgurAccount);
    m_userField->setStyleSheet(QStringLiteral("font-weight: bold"));
    m_albumField = new QLineEdit(imgurAccount);
    m_anonymousUpload = new QCheckBox(tr("Anonymous upload"), imgurAccount);
    m_anonymousUpload->setToolTip(tr("This will use your Imgur client ID but it won't be uploaded to your account."));
    imgurAccountLayout->addRow(tr("User"), m_userField);
    imgurAccountLayout->addRow(tr("Album"), m_albumField);
    imgurAccountLayout->addWidget(m_anonymousUpload);
    imgurAccount->setLayout(imgurAccountLayout);
    m_widgetLayout->addWidget(imgurAccount);

    m_widgetLayout->addStretch();
    QGroupBox *actionButtons = new QGroupBox(QStringLiteral("Actions"), this);
    QHBoxLayout *buttonLayout = new QHBoxLayout(actionButtons);
    m_logInButton = new QPushButton(tr("Login"), actionButtons);
    m_logOutButton = new QPushButton(tr("Logout"), actionButtons);
    m_saveButton = new QPushButton(tr("Save"), actionButtons);
    buttonLayout->addWidget(m_logInButton);
    buttonLayout->addWidget(m_logOutButton);
    buttonLayout->addWidget(m_saveButton);
    actionButtons->setLayout(buttonLayout);
    m_widgetLayout->addWidget(actionButtons);

    connect(m_logInButton, &QPushButton::clicked, this, &ImgurConf::authorize);
    connect(m_logOutButton, &QPushButton::clicked, this, &ImgurConf::deauthorize);
    connect(m_saveButton, &QPushButton::clicked, this, &ImgurConf::updateImgurSettings);
    connect(this, &ImgurConf::settingsChanged, this, &ImgurConf::updateComponents);
}

void ImgurConf::updateImgurSettings()
{
    // Check for API changes
    if (m_clientIdField->text() != config.getSetting(QStringLiteral("Api/client_id")).toString() ||
        m_clientSecretField->text() != config.getSetting(QStringLiteral("Api/client_secret")).toString()) {
        // Purge old token
        QMap<QString, QVariant> token = {};
        config.setToken(token);
    }

    config.setApiCredentials(m_clientIdField->text(), m_clientSecretField->text());
    config.setSetting(QStringLiteral("album"), m_albumField->text());
    config.setSetting(QStringLiteral("anonymous_upload"), config.isAuthorized() && m_anonymousUpload->isChecked());

    emit settingsChanged();
}

void ImgurConf::updateComponents()
{
    QMap<QString, QVariant> token = config.getToken();

    m_clientIdField->setText(config.getSetting(QStringLiteral("Api/client_id")).toString());
    m_clientSecretField->setText(config.getSetting(QStringLiteral("Api/client_secret")).toString());
    m_userField->setText(token.value(QStringLiteral("account_username")).toString());
    m_albumField->setText(config.getSetting(QStringLiteral("album")).toString());

    // Anonymous upload if user authorized the application and checked
    // the option or if the application hasn't been authorized.
    if ((config.isAuthorized() && config.getSetting(QStringLiteral("anonymous_upload")).toBool()) ||
        !config.isAuthorized()) {
        m_anonymousUpload->setChecked(true);
    }

    m_logInButton->setEnabled(false);
    m_logOutButton->setEnabled(true);

    if (token.isEmpty()) {
        m_logInButton->setEnabled(true);
        m_logOutButton->setEnabled(false);
    }
}

void ImgurConf::extractToken(QString &token_url)
{
    if (token_url.isEmpty() || token_url.isNull()) {
        return;
    }

    // Remove unneeded data
    token_url.remove(QRegularExpression(QStringLiteral("^.+#")));

    QUrlQuery urlQuery(token_url);

    QMap<QString, QVariant> token;

    for (const QPair<QString, QString> &pair : urlQuery.queryItems()) {
        // Filter token
        if (pair.first != QLatin1String("access_token") &&
            pair.first != QLatin1String("expires_in") &&
            pair.first != QLatin1String("token_type") &&
            pair.first != QLatin1String("refresh_token") &&
            pair.first != QLatin1String("account_username") &&
            pair.first != QLatin1String("account_id")) {
            continue;
        }

        // Generate token
        token.insert(pair.first, pair.second);
    }

    config.setToken(token);

    emit settingsChanged();
}
