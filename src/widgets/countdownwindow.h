#include <QWidget>

#pragma once

class QLabel;
class QTimer;

class CountdownWindow : public QWidget
{
    Q_OBJECT
public:
    explicit CountdownWindow(int seconds, QWidget* parent = nullptr);
    void startCountdown();

signals:
    void countdownFinished();

private:
    void updateDisplay();

    QLabel* m_label;
    QTimer* m_timer;
    int m_remainingSeconds;
};
