//
// Created by yuriypuchkov on 09.12.2020.
//

#ifndef FLAMESHOT_UPDATENOTIFICATIONWIDGET_H
#define FLAMESHOT_UPDATENOTIFICATIONWIDGET_H

#include <QPointer>
#include <QWidget>

class QVBoxLayout;
class QPropertyAnimation;
class QScrollArea;
class QPushButton;
class QLabel;

class UpdateNotificationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UpdateNotificationWidget(QWidget* parent,
                                      const QString& appLatestVersion,
                                      QString appLatestUrl);
    void setAppLatestVersion(const QString& latestVersion);

    void hide();
    void show();

public slots:
    void ignoreButton();
    void laterButton();
    void updateButton();

private:
    void initInternalPanel();

    // class members
    QString m_appLatestVersion;
    QString m_appLatestUrl;
    QVBoxLayout* m_layout;
    QLabel* m_notification;
    QScrollArea* m_internalPanel;
    QPropertyAnimation* m_showAnimation;
    QPropertyAnimation* m_hideAnimation;
};

#endif // FLAMESHOT_UPDATENOTIFICATIONWIDGET_H
