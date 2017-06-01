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

private:
    QVBoxLayout *m_layout;

    void initHelpShow();
};

#endif // GENENERALCONF_H
