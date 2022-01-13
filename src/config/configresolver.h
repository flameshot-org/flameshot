#pragma once

#include <QDialog>

class QGridLayout;

class ConfigResolver : public QDialog
{
public:
    ConfigResolver(QWidget* parent = nullptr);

    QGridLayout* layout();

private:
    void populate();
    void resetLayout();
};
