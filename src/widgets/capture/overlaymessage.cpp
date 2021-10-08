#include "overlaymessage.h"
#include "colorutils.h"
#include "confighandler.h"
#include "qguiappcurrentscreen.h"

#include <QApplication>
#include <QDebug>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QScreen>

OverlayMessage::OverlayMessage(QWidget* parent, const QRect& targetArea)
  : QLabel(parent)
  , m_targetArea(targetArea)
{
    // NOTE: do not call the static functions from the constructor
    m_instance = this;
    m_messageStack.push(QString()); // Default message is empty
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_AlwaysStackOnTop);
    setAlignment(Qt::AlignCenter);
    setTextFormat(Qt::RichText);
    setMargin(QApplication::fontMetrics().height() / 2);
    QWidget::hide();
}

void OverlayMessage::init(QWidget* parent, const QRect& targetArea)
{
    new OverlayMessage(parent, targetArea);
}

/**
 * @brief Push a message to the message stack.
 * @param msg Message text formatted as rich text
 */
void OverlayMessage::push(const QString& msg)
{
    m_instance->m_messageStack.push(msg);
    m_instance->setText(m_instance->m_messageStack.top());
    setVisibility(true);
}

void OverlayMessage::pop()
{
    if (m_instance->m_messageStack.size() > 1) {
        m_instance->m_messageStack.pop();
    }

    m_instance->setText(m_instance->m_messageStack.top());
    setVisibility(m_instance->m_messageStack.size() > 1);
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

void OverlayMessage::pushKeyMap(const QList<QPair<QString, QString>>& map)
{
    push(compileKeyMap(map));
}

/**
 * @brief Compile a message from a set of shortcuts and descriptions.
 * @param map List of (shortcut, description) pairs
 */
QString OverlayMessage::compileKeyMap(const QList<QPair<QString, QString>>& map)
{
    QString str = QStringLiteral("<table>");
    for (const auto& pair : map) {
        str += QStringLiteral("<tr>"
                              "<td align=\"right\"><b>%1 </b></td>"
                              "<td align=\"left\">&nbsp;&nbsp;%2</td>"
                              "</tr>")
                 .arg(pair.first)
                 .arg(pair.second);
    }
    str += QStringLiteral("</table>");
    return str;
}

void OverlayMessage::paintEvent(QPaintEvent* e)
{
    QPainter painter(this);

    QColor rectColor(ConfigHandler().uiColor());
    rectColor.setAlpha(180);
    QColor textColor(
      (ColorUtils::colorIsDark(rectColor) ? Qt::white : Qt::black));

    painter.setBrush(QBrush(rectColor, Qt::SolidPattern));
    painter.setPen(QPen(textColor));
    float margin = painter.pen().widthF();
    painter.drawRect(rect() - QMarginsF(margin, margin, margin, margin));

    return QLabel::paintEvent(e);
}

QRect OverlayMessage::boundingRect() const
{
    QRect geom = QRect(QPoint(), sizeHint());
    geom.moveCenter(m_targetArea.center());
    return geom;
}

void OverlayMessage::updateGeometry()
{
    setGeometry(boundingRect());
    QLabel::updateGeometry();
}

OverlayMessage* OverlayMessage::m_instance = nullptr;
