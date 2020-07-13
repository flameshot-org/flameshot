#pragma once

#include <QWidget>
//#include <QPointer>

class QVBoxLayout;
class QLineEdit;
class QCheckBox;
class FileNameHandler;
class QPushButton;

class FilePathConfiguration : public QWidget {
    Q_OBJECT
public:
    explicit FilePathConfiguration(QWidget *parent = nullptr);

private:
    QVBoxLayout *m_layout;
    QCheckBox *m_screenshotPathFixed;
    QLineEdit *m_screenshotPathFixedDefault;
    QPushButton *m_screenshotPathFixedBrowse;
    QPushButton *m_screenshotPathFixedClear;

    void initLayout();
    void initWidgets();

public slots:

private slots:
    void sreenshotPathFixed();
    void screenshotPathFixedSet();
    void screenshotPathFixedClear();
};
