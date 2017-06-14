#ifndef GENENERALCONF_H
#define GENENERALCONF_H

#include <QFrame>

class QVBoxLayout;

class GeneneralConf : public QFrame {
    Q_OBJECT
public:
    GeneneralConf(QWidget *parent = 0);

private slots:
   void showHelpChanged(bool checked);
   void showDesktopNotificationChanged(bool checked);

private:
    QVBoxLayout *m_layout;

    void initShowHelp();
    void initShowDesktopNotification();
};

#endif // GENENERALCONF_H
