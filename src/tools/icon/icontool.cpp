// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "icontool.h"
#include "tools/icon/iconconfig.h"

#include <QFont>
#include <QPainter>
#include <algorithm>
#include <cmath>

namespace {
constexpr int ICON_BASE_PIXEL_SIZE = 22;
constexpr int ICON_SIZE_STEP = 4;
constexpr int ICON_MIN_PIXEL_SIZE = 18;
constexpr int ICON_PADDING = 6;
}

IconTool::IconTool(QObject* parent)
  : AbstractTwoPointTool(parent)
  , m_symbol(QStringLiteral("✓"))
  , m_valid(false)
{}

IconTool::~IconTool()
{
    closeConfig();
}

void IconTool::closeConfig()
{
    if (!m_confW.isNull()) {
        m_confW->hide();
        delete m_confW;
        m_confW = nullptr;
    }
}

QIcon IconTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "image.svg");
}

QString IconTool::name() const
{
    return tr("Icon");
}

QString IconTool::description() const
{
    return tr("Add an icon to your capture");
}

QString IconTool::info()
{
    m_tempString = QStringLiteral("%1 - %2").arg(name(), m_symbol);
    return m_tempString;
}

CaptureTool::Type IconTool::type() const
{
    return CaptureTool::TYPE_ICON;
}

bool IconTool::isValid() const
{
    return m_valid && !m_symbol.isEmpty();
}

QRect IconTool::mousePreviewRect(const CaptureContext& context) const
{
    const int pixelSize = ICON_BASE_PIXEL_SIZE + context.toolSize * ICON_SIZE_STEP;
    QRect rect(0, 0, pixelSize + ICON_PADDING * 2, pixelSize + ICON_PADDING * 2);
    rect.moveCenter(context.mousePos);
    return rect;
}

QRect IconTool::boundingRect() const
{
    if (!isValid()) {
        return {};
    }
    return iconRect();
}

QWidget* IconTool::configurationWidget()
{
    closeConfig();
    m_confW = new IconConfig();
    connect(m_confW,
            &IconConfig::symbolChanged,
            this,
            &IconTool::updateSymbol);
    m_confW->setSymbol(m_symbol);
    return m_confW;
}

void IconTool::copyParams(const IconTool* from, IconTool* to)
{
    AbstractTwoPointTool::copyParams(from, to);
    to->m_symbol = from->m_symbol;
    to->m_valid = from->m_valid;
}

CaptureTool* IconTool::copy(QObject* parent)
{
    auto* tool = new IconTool(parent);
    if (m_confW != nullptr) {
        connect(m_confW,
                &IconConfig::symbolChanged,
                tool,
                &IconTool::updateSymbol);
    }
    copyParams(this, tool);
    return tool;
}

void IconTool::process(QPainter& painter, const QPixmap& pixmap)
{
    Q_UNUSED(pixmap)
    if (!isValid()) {
        return;
    }
    drawIcon(painter, iconRect());
}

void IconTool::paintMousePreview(QPainter& painter,
                                 const CaptureContext& context)
{
    const int pixelSize = ICON_BASE_PIXEL_SIZE + context.toolSize * ICON_SIZE_STEP;
    QRect rect(0, 0, pixelSize + ICON_PADDING * 2, pixelSize + ICON_PADDING * 2);
    rect.moveCenter(context.mousePos);
    drawIcon(painter, rect, 0.35);
}

void IconTool::drawStart(const CaptureContext& context)
{
    AbstractTwoPointTool::drawStart(context);
    m_valid = true;
}

void IconTool::pressed(CaptureContext& context)
{
    Q_UNUSED(context)
}

void IconTool::updateSymbol(const QString& symbol)
{
    m_symbol = symbol;
}

int IconTool::iconPixelSize() const
{
    const QPoint delta = points().second - points().first;
    const int draggedSize = static_cast<int>(std::lround(
      std::sqrt(delta.x() * delta.x() + delta.y() * delta.y()) * 2.0));
    const int configuredSize = ICON_BASE_PIXEL_SIZE + size() * ICON_SIZE_STEP;
    return std::max({ ICON_MIN_PIXEL_SIZE, configuredSize, draggedSize });
}

QRect IconTool::iconRect() const
{
    const int pixelSize = iconPixelSize();
    QRect rect(0, 0, pixelSize + ICON_PADDING * 2, pixelSize + ICON_PADDING * 2);
    rect.moveCenter(points().first);
    return rect;
}

void IconTool::drawIcon(QPainter& painter, const QRect& rect, qreal opacity)
{
    auto origPen = painter.pen();
    auto origFont = painter.font();
    auto origOpacity = painter.opacity();

    QFont font = origFont;
    font.setPixelSize(std::max(ICON_MIN_PIXEL_SIZE, rect.height() - ICON_PADDING * 2));
    font.setBold(true);

    painter.setOpacity(opacity);
    painter.setFont(font);
    painter.setPen(color());
    painter.drawText(rect, Qt::AlignCenter, m_symbol);

    painter.setOpacity(origOpacity);
    painter.setFont(origFont);
    painter.setPen(origPen);
}
