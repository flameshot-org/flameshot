// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
//
// This file is part of Flameshot.
//
//     Flameshot is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Flameshot is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Flameshot.  If not, see <http://www.gnu.org/licenses/>.

#include "pixelatetool.h"
#include <QApplication>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QImage>
#include <QPainter>
#include <cassert>

PixelateTool::PixelateTool(QObject* parent)
  : AbstractTwoPointTool(parent)
{}

QIcon
PixelateTool::icon(const QColor& background, bool inEditor) const
{
  Q_UNUSED(inEditor);
  return QIcon(iconPath(background) + "blur.svg");
}
QString
PixelateTool::name() const
{
  return tr("Pixelate");
}

QString
PixelateTool::nameID()
{
  return QLatin1String("");
}

QString
PixelateTool::description() const
{
  return tr("Set Pixelate as the paint tool");
}

CaptureTool*
PixelateTool::copy(QObject* parent)
{
  return new PixelateTool(parent);
}

void
write_block(QImage& image,
            int x_start,
            int y_start,
            int pixel_size,
            QRgb block_color)
{
  assert(x_start + pixel_size < image.width());
  assert(y_start + pixel_size < image.height());

  for (auto x = x_start; x < x_start + pixel_size; x++) {
    for (auto y = y_start; y < y_start + pixel_size; y++) {
      image.setPixel(x, y, block_color);
    }
  }
}

QRgb
calculate_block_averge(QImage& image, int x_start, int y_start, int pixel_size)
{
  assert(x_start + pixel_size < image.width());
  assert(y_start + pixel_size < image.height());

  int red_count = 0;
  int blue_count = 0;
  int green_count = 0;
  int pixel_count = 0;
  for (auto x = x_start; x < x_start + pixel_size; x++) {
    for (auto y = y_start; y < y_start + pixel_size; y++) {
      auto pixel = image.pixel(x, y);

      red_count += qRed(pixel);
      green_count += qGreen(pixel);
      blue_count += qBlue(pixel);
      pixel_count++;
    }
  }
  return (qRgb(red_count / pixel_count,
               green_count / pixel_count,
               blue_count / pixel_count));
}
void
PixelateTool::process(QPainter& painter, const QPixmap& pixmap, bool recordUndo)
{
  if (recordUndo) {
    updateBackup(pixmap);
  }
  QPoint& p0 = m_points.first;
  QPoint& p1 = m_points.second;
  auto pixelRatio = pixmap.devicePixelRatio();

  QRect selection = QRect(p0, p1).normalized();
  QRect selectionScaled = QRect(p0 * pixelRatio, p1 * pixelRatio).normalized();

  QPixmap* source = new QPixmap(pixmap.copy(selectionScaled));

  QImage original_image{ source->toImage() };
  QImage imageResult{ source->toImage() };
  unsigned int pixel_size = m_thickness;
  if (pixel_size < 1) {
    pixel_size = 1;
  }

  const unsigned int width = source->width();
  const unsigned int height = source->height();

  // Don't start pixelating until the region is at least as big as the pixel
  if ((width > pixel_size) && (height > pixel_size)) {
    for (unsigned int x = 0; x < (width - pixel_size); x += pixel_size) {
      for (unsigned int y = 0; y < (height - pixel_size); y += pixel_size) {
        auto block_color =
          calculate_block_averge(original_image, x, y, pixel_size);
        write_block(imageResult, x, y, pixel_size, block_color);
      }
    }
  }
  QPixmap result{ QPixmap::fromImage(imageResult) };

  QGraphicsScene scene;
  scene.addPixmap(result);

  scene.render(&painter, selection, QRectF());
}

void
PixelateTool::paintMousePreview(QPainter& painter,
                                const CaptureContext& context)
{
  Q_UNUSED(context);
  Q_UNUSED(painter);
}

void
PixelateTool::drawStart(const CaptureContext& context)
{
  m_thickness = context.thickness;
  m_points.first = context.mousePos;
  m_points.second = context.mousePos;
}

void
PixelateTool::pressed(const CaptureContext& context)
{
  Q_UNUSED(context);
}
