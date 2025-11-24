#include "countdownwindow.h"

#include "src/utils/confighandler.h"
#include <QApplication>
#include <QLabel>
#include <QScreen>
#include <QTimer>
#include <QVBoxLayout>

CountdownWindow::CountdownWindow(int seconds, QWidget* parent)
  : QWidget(parent,
            Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
              Qt::WindowTransparentForInput)
  , m_remainingSeconds(seconds)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_TransparentForMouseEvents);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(15, 15, 15, 15);

    m_label = new QLabel(this);
    m_label->setAlignment(Qt::AlignCenter);

    QColor uiColor = ConfigHandler().uiColor();
    QColor textColor = uiColor.lightness() > 127 ? Qt::black : Qt::white;

    // Make background semi-transparent (75% opacity)
    uiColor.setAlphaF(0.75);

    m_label->setStyleSheet(QString("QLabel {"
                                   "    background-color: rgba(%1, %2, %3, %4);"
                                   "    color: %5;"
                                   "    border-radius: 8px;"
                                   "    padding: 15px 20px;"
                                   "    font-size: 14pt;"
                                   "    font-weight: bold;"
                                   "}")
                             .arg(uiColor.red())
                             .arg(uiColor.green())
                             .arg(uiColor.blue())
                             .arg(int(uiColor.alphaF() * 255))
                             .arg(textColor.name()));

    layout->addWidget(m_label);

    m_timer = new QTimer(this);
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, &CountdownWindow::updateDisplay);

    // Position window at top-right (about 5% from edges)
    updateDisplay(); // Update text first so we know the size
    adjustSize();    // Resize to fit content

    QScreen* screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();

        // Calculate position: 5% from right edge, 5% from top edge
        int xOffset = screenGeometry.width() * 0.05;
        int yOffset = screenGeometry.height() * 0.05;

        int x = screenGeometry.right() - width() - xOffset;
        int y = screenGeometry.top() + yOffset;

        move(x, y);
    }
}

void CountdownWindow::startCountdown()
{
    show();
    m_timer->start();
}

void CountdownWindow::updateDisplay()
{
    if (m_remainingSeconds > 0) {
        QString suffix =
          m_remainingSeconds == 1 ? tr(" second") : tr(" seconds");
        m_label->setText(
          tr("Screenshot in\n%1%2").arg(m_remainingSeconds).arg(suffix));
        m_remainingSeconds--;
    } else {
        m_timer->stop();
        close();
        emit countdownFinished();
    }
}
