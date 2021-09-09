#include "overlaymessage.h"
#include "colorutils.h"
#include "confighandler.h"
#include "qguiappcurrentscreen.h"

#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QPen>
#include <QScreen>
#include <QWidget>

OverlayMessage::OverlayMessage(QWidget* parent)
  : QWidget(parent)
{
    // NOTE: do not call the static functions from the constructor
    m_instance = this;
    m_messageStack.push(QString()); // Default message is empty
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_AlwaysStackOnTop);
    QWidget::hide();
}

void OverlayMessage::init(QWidget* parent)
{
    new OverlayMessage(parent);
}

void OverlayMessage::push(const QString& msg)
{
    m_instance->m_messageStack.push(msg);
    setVisibility(true);
}

void OverlayMessage::pop()
{
    if (m_instance->m_messageStack.size() > 1) {
        m_instance->m_messageStack.pop();
    }

    if (m_instance->m_messageStack.size() == 1) {
        // Only empty message left (don't show it)
        m_instance->QWidget::hide();
    } else {
        // Still visible, resize for new message
        m_instance->updateGeometry();
    }
}

void OverlayMessage::setVisibility(bool visible)
{
    m_instance->updateGeometry();
    m_instance->setVisible(visible);
}

OverlayMessage* OverlayMessage::instance()
{
    return m_instance;
}

void OverlayMessage::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    QRectF bRect = boundingRect();
    bRect.moveTo(0, 0);

    QColor rectColor(ConfigHandler().uiMainColorValue());
    rectColor.setAlpha(180);
    QColor textColor(
      (ColorUtils::colorIsDark(rectColor) ? Qt::white : Qt::black));

    painter.setBrush(QBrush(rectColor, Qt::SolidPattern));
    painter.setPen(QPen(textColor));

    painter.drawRect(bRect);
    painter.drawText(bRect, Qt::AlignCenter, m_messageStack.top());
}

void OverlayMessage::showEvent(QShowEvent*)
{
    update();
}

QRectF OverlayMessage::boundingRect() const
{
    // We draw the white contrasting background for the text, using the
    // same text and options to get the boundingRect that the text will
    // have.
    QRectF bRect = QApplication::fontMetrics().boundingRect(
      m_instance->parentWidget()->geometry(),
      Qt::AlignCenter,
      m_messageStack.top());

    // These four calls provide padding for the rect
    const int margin = QApplication::fontMetrics().height() / 2;
    bRect.setWidth(bRect.width() + margin);
    bRect.setHeight(bRect.height() + margin);
    bRect.setX(bRect.x() - margin);
    bRect.setY(bRect.y() - margin);
    return bRect;
}

void OverlayMessage::updateGeometry()
{
    m_instance->setGeometry(m_instance->boundingRect().toRect());
    QWidget::updateGeometry();
}

OverlayMessage* OverlayMessage::m_instance = nullptr;
