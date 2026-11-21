// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "pixelatetool.h"
#include "utils/confighandler.h"

#include <QApplication>
#include <QGraphicsBlurEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QImage>
#include <QPainter>
#include <array>
#include <random>

PixelateTool::PixelateTool(QObject* parent)
  : AbstractTwoPointTool(parent)
{}

QIcon PixelateTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "pixelate.svg");
}

QString PixelateTool::name() const
{
    return tr("Pixelate");
}

CaptureTool::Type PixelateTool::type() const
{
    return CaptureTool::TYPE_PIXELATE;
}

QString PixelateTool::description() const
{
    return tr("Set Pixelate as the paint tool.");
}

QRect PixelateTool::boundingRect() const
{
    return QRect(points().first, points().second).normalized();
}

CaptureTool* PixelateTool::copy(QObject* parent)
{
    auto* tool = new PixelateTool(parent);
    copyParams(this, tool);
    tool->m_cachedImage = m_cachedImage;
    tool->m_cachedSelectionScaled = m_cachedSelectionScaled;
    tool->m_cachedSize = m_cachedSize;
    tool->m_cachedInsecure = m_cachedInsecure;
    tool->m_cachedPixmapKey = m_cachedPixmapKey;
    return tool;
}

/**
 * Since pixelation does not protect the contents of the pixelated area
 * (see e.g. https://github.com/bishopfox/unredacter),
 * _pseudo-pixelation_ is used:
 *
 * Only colors from the fringe of the selected area are used to generate
 * a pixelation-like effect. The interior of the selected area is not used
 * as an input at all and hence can not be recovered.
 *
 */
void PixelateTool::process(QPainter& painter, const QPixmap& pixmap)
{
    if (pixmap.isNull()) {
        return;
    }

    const bool useInsecurePixelate = ConfigHandler().insecurePixelate();

    QRect selection = boundingRect().intersected(pixmap.rect());
    if (selection.isEmpty()) {
        return;
    }
    const auto pixelRatio = pixmap.devicePixelRatio();
    const QRect selectionScaled(selection.topLeft() * pixelRatio,
                                selection.bottomRight() * pixelRatio);
    if (selectionScaled.isEmpty()) {
        return;
    }

    const qint64 pixKey = pixmap.cacheKey();
    if (!m_cachedImage.isNull() && m_cachedSelectionScaled == selectionScaled &&
        m_cachedSize == size() && m_cachedInsecure == useInsecurePixelate &&
        m_cachedPixmapKey == pixKey) {
        painter.drawImage(selection, m_cachedImage);
        return;
    }

    const int width =
      static_cast<int>(selection.width() * (0.5 / qMax(1, size() + 1)));
    const int height =
      static_cast<int>(selection.height() * (0.5 / qMax(1, size() + 1)));
    const QSize effectSize(qMax(width, 1), qMax(height, 1));

    QImage result;
    if (useInsecurePixelate) {
        if (size() <= 1) {
            result = QImage(selectionScaled.size(),
                            QImage::Format_ARGB32_Premultiplied);
            result.setDevicePixelRatio(pixelRatio);
            result.fill(Qt::transparent);
            QPainter imgPainter(&result);
            auto* blur = new QGraphicsBlurEffect();
            blur->setBlurRadius(10);
            auto* item = new QGraphicsPixmapItem(pixmap.copy(selectionScaled));
            item->setGraphicsEffect(blur);

            QGraphicsScene scene;
            scene.addItem(item);

            const QRectF targetRect(QPointF(0, 0), selection.size());
            scene.render(&imgPainter, targetRect, QRectF());
            blur->setBlurRadius(12);
            // multiple repeat for make blur effect stronger
            scene.render(&imgPainter, targetRect, QRectF());
        } else {
            auto pixmapPixelated = pixmap.copy(selectionScaled);
            pixmapPixelated = pixmapPixelated.scaled(
              effectSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            pixmapPixelated =
              pixmapPixelated.scaled(selection.width(), selection.height());
            result = pixmapPixelated.toImage();
        }
    } else {
        // the PRNG is only used for visual effects and NOT part of the security
        // boundary
        std::mt19937 prng(42);

        // noise for the sampling process to avoid only sampling from a small
        // subset of the fringe
        std::normal_distribution<float> sampling_noise(0, 5 * size() + 1);

        // additional noise that will be added on top of the effect to avoid
        // generating a monochromatic box when the fringe is monochromatic
        std::normal_distribution<float> noise(0, 0.1f);

        QPoint const offset_top(0, selectionScaled.topLeft().y() == 0 ? 0 : -1);
        QPoint const offset_bottom(0,
                                   selectionScaled.bottomLeft().y() ==
                                       pixmap.rect().bottomLeft().y()
                                     ? 0
                                     : 1);
        QPoint const offset_left(selectionScaled.topLeft().x() == 0 ? 0 : -1,
                                 0);
        QPoint const offset_right(
          selectionScaled.topRight().x() == pixmap.rect().topRight().x() ? 0
                                                                         : 1,
          0);

        // only values from the fringe will be used to compute the
        // pseudo-pixelation
        std::array<QImage, 4> fringe = {
            // top fringe
            pixmap
              .copy(QRect(selectionScaled.topLeft() + offset_top,
                          selectionScaled.topRight() + offset_top))
              .toImage()
              .convertToFormat(QImage::Format_RGB32),
            // bottom fringe
            pixmap
              .copy(QRect(selectionScaled.bottomLeft() + offset_bottom,
                          selectionScaled.bottomRight() + offset_bottom))
              .toImage()
              .convertToFormat(QImage::Format_RGB32),
            // left fringe
            pixmap
              .copy(QRect(selectionScaled.topLeft() + offset_left,
                          selectionScaled.bottomLeft() + offset_left))
              .toImage()
              .convertToFormat(QImage::Format_RGB32),
            // right fringe
            pixmap
              .copy(QRect(selectionScaled.topRight() + offset_right,
                          selectionScaled.bottomRight() + offset_right))
              .toImage()
              .convertToFormat(QImage::Format_RGB32)
        };

        for (const QImage& f : fringe) {
            if (f.isNull() || f.width() <= 0 || f.height() <= 0) {
                return;
            }
        }

        const int effW = effectSize.width();
        const int effH = effectSize.height();
        QImage pixelated(effW, effH, QImage::Format_RGB32);

        const float invW = 1.0f / static_cast<float>(effW);
        const float invH = 1.0f / static_cast<float>(effH);
        constexpr float inv255 = 1.0f / 255.0f;

        std::array<std::array<float, 3>, 4> samples;

        for (int y = 0; y < effH; ++y) {
            QRgb* outRow = reinterpret_cast<QRgb*>(pixelated.scanLine(y));
            const float vertical = y * invH;
            const int wyRatio =
              (width > 0 && height > 0) ? (qMin(y, height - y) / height) : 0;

            for (int x = 0; x < effW; ++x) {
                const float n = noise(prng);
                const float horizontal = x * invW;

                for (int i = 0; i < 4; ++i) {
                    const int fx = std::clamp(
                      static_cast<int>(horizontal * fringe[i].width() +
                                       sampling_noise(prng)),
                      0,
                      fringe[i].width() - 1);
                    const int fy = std::clamp(
                      static_cast<int>(vertical * fringe[i].height() +
                                       sampling_noise(prng)),
                      0,
                      fringe[i].height() - 1);
                    const QRgb* fRow = reinterpret_cast<const QRgb*>(
                      fringe[i].constScanLine(fy));
                    const QRgb c = fRow[fx];
                    samples[i][0] = qRed(c) * inv255;
                    samples[i][1] = qGreen(c) * inv255;
                    samples[i][2] = qBlue(c) * inv255;
                }

                const int wxRatio =
                  (width > 0) ? (qMin(x, width - x) / width) : 0;
                const float weight_h =
                  static_cast<float>(wxRatio - wyRatio) + 0.5f;
                const float weight_v = 1.0f - weight_h;

                std::array<int, 3> rgb = { 0, 0, 0 };
                for (int i = 0; i < 3; ++i) {
                    const float c =
                      weight_h * ((1.0f - horizontal) * samples[2][i] +
                                  horizontal * samples[3][i]) +
                      weight_v * ((1.0f - vertical) * samples[0][i] +
                                  vertical * samples[1][i]) +
                      n;
                    rgb[i] = std::clamp(static_cast<int>(255.0f * c), 0, 255);
                }
                outRow[x] = qRgb(rgb[0], rgb[1], rgb[2]);
            }
        }

        result = pixelated.scaled(selection.width(),
                                  selection.height(),
                                  Qt::IgnoreAspectRatio,
                                  Qt::FastTransformation);
    }

    m_cachedImage = result;
    m_cachedSelectionScaled = selectionScaled;
    m_cachedSize = size();
    m_cachedInsecure = useInsecurePixelate;
    m_cachedPixmapKey = pixKey;

    painter.drawImage(selection, m_cachedImage);
}

void PixelateTool::drawSearchArea(QPainter& painter, const QPixmap& pixmap)
{
    Q_UNUSED(pixmap)
    painter.fillRect(boundingRect(), QBrush(Qt::black));
}

void PixelateTool::paintMousePreview(QPainter& painter,
                                     const CaptureContext& context)
{
    Q_UNUSED(context)
    Q_UNUSED(painter)
}

void PixelateTool::pressed(CaptureContext& context)
{
    Q_UNUSED(context)
}
