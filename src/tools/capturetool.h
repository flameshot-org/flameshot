// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "src/tools/capturecontext.h"
#include "src/utils/colorutils.h"
#include "src/utils/pathinfo.h"
#include <QIcon>
#include <QPainter>
#include <qgraphicseffect.h>
#include <qgraphicsitem.h>
#include <qgraphicsscene.h>
#include <qpainterpath.h>
#include <qpixmapcache.h>

class CaptureTool : public QObject
{
    Q_OBJECT

public:
    // IMPORTANT:
    //   Add new entries to the BOTTOM so existing user configurations don't get
    //   messed up.
    // ALSO NOTE:
    //   When adding new types, don't forget to update:
    //   - CaptureToolButton::iterableButtonTypes
    //   - CaptureToolButton::buttonTypeOrder
    enum Type
    {
        NONE = -1,
        TYPE_PENCIL = 0,
        TYPE_DRAWER = 1,
        TYPE_ARROW = 2,
        TYPE_SELECTION = 3,
        TYPE_RECTANGLE = 4,
        TYPE_CIRCLE = 5,
        TYPE_MARKER = 6,
        TYPE_MOVESELECTION = 8,
        TYPE_UNDO = 9,
        TYPE_COPY = 10,
        TYPE_SAVE = 11,
        TYPE_EXIT = 12,
        TYPE_IMAGEUPLOADER = 13,
        TYPE_OPEN_APP = 14,
        TYPE_PIXELATE = 15,
        TYPE_REDO = 16,
        TYPE_PIN = 17,
        TYPE_TEXT = 18,
        TYPE_CIRCLECOUNT = 19,
        TYPE_SIZEINCREASE = 20,
        TYPE_SIZEDECREASE = 21,
        TYPE_INVERT = 22,
        TYPE_ACCEPT = 23,
        TYPE_CANCEL = 24,
    };
    Q_ENUM(Type);

    // Request actions on the main widget
    enum Request
    {
        // Call close() in the editor.
        REQ_CLOSE_GUI,
        // Call hide() in the editor.
        REQ_HIDE_GUI,
        // Undo the last active modification in the stack.
        REQ_UNDO_MODIFICATION,
        // Redo the next modification in the stack.
        REQ_REDO_MODIFICATION,
        // Open the color picker under the mouse.
        REQ_SHOW_COLOR_PICKER,
        // Notify is the screenshot has been saved.
        REQ_CAPTURE_DONE_OK,
        // Notify to redraw screenshot with tools without object selection.
        REQ_CLEAR_SELECTION,
        // Instance this->widget()'s widget inside the editor under the mouse.
        REQ_ADD_CHILD_WIDGET,
        // Instance this->widget()'s widget which handles its own lifetime.
        REQ_ADD_EXTERNAL_WIDGETS,
        // increase tool size for all tools
        REQ_INCREASE_TOOL_SIZE,
        // decrease tool size for all tools
        REQ_DECREASE_TOOL_SIZE
    };

    explicit CaptureTool(QObject* parent = nullptr)
      : QObject(parent)
      , m_count(0)
      , m_editMode(false)
    {}

    // TODO unused
    virtual void setCapture(const QPixmap& pixmap){};

    // Returns false when the tool is in an inconsistent state and shouldn't
    // be included in the tool undo/redo stack.
    virtual bool isValid() const = 0;
    // Close the capture after the process() call if the tool was activated
    // from a button press. TODO remove this function
    virtual bool closeOnButtonPressed() const = 0;
    // If the tool keeps active after the selection.
    virtual bool isSelectable() const = 0;
    // Enable mouse preview.
    virtual bool showMousePreview() const = 0;
    virtual QRect mousePreviewRect(const CaptureContext& context) const
    {
        return {};
    };
    virtual QRect boundingRect() const = 0;

    // The icon of the tool.
    // inEditor is true when the icon is requested inside the editor
    // and false otherwise.
    virtual QIcon icon(const QColor& background, bool inEditor) const = 0;
    // Name displayed for the tool, this could be translated with tr()
    virtual QString name() const = 0;
    // Codename for the tool, this shouldn't change as it is used as ID
    // for the tool in the internals of Flameshot
    virtual CaptureTool::Type type() const = 0;
    // Short description of the tool.
    virtual QString description() const = 0;
    // Short tool item info
    virtual QString info() { return name(); };

    // if the type is TYPE_WIDGET the widget is loaded in the main widget.
    // If the type is TYPE_EXTERNAL_WIDGET it is created outside as an
    // individual widget.
    virtual QWidget* widget() { return nullptr; }
    // When the tool is selected this method is called and the widget is added
    // to the configuration panel inside the main widget.
    virtual QWidget* configurationWidget() { return nullptr; }
    // Return a copy of the tool
    virtual CaptureTool* copy(QObject* parent = nullptr) = 0;

    virtual void setEditMode(bool b) { m_editMode = b; };
    virtual bool editMode() { return m_editMode; };

    // return true if object was change after editMode
    virtual bool isChanged() { return true; };

    // Counter for all object types (currently is used for the CircleCounter
    // only)
    virtual void setCount(int count) { m_count = count; };
    virtual int count() const { return m_count; };

    virtual void setDropShadowEnabled(bool enabled)
    {
        m_dropShadowEnabled = enabled;
    }
    virtual bool dropShadowEnabled() const { return m_dropShadowEnabled; }

    virtual void process(QPainter& painter, const QPixmap& pixmap) = 0;

    // Called every time the tool has to draw
    void doProcess(QPainter& painter, const QPixmap& pixmap, bool skipCache)
    {
        if (dropShadowEnabled()) {
            QPixmap renderedPixmap;
            if (skipCache || !m_pixmapCacheKey.isValid()) {
                QPixmapCache::remove(m_pixmapCacheKey);
            }
            if (!QPixmapCache::find(m_pixmapCacheKey, &renderedPixmap)) {
                renderedPixmap = renderShapeWithShadow(pixmap.size());
                m_pixmapCacheKey = QPixmapCache::insert(renderedPixmap);
            }
            painter.drawPixmap(m_pixmapCachePosition, renderedPixmap);
        } else {
            process(painter, pixmap);
        }
    };

    virtual void drawSearchArea(QPainter& painter, const QPixmap& pixmap)
    {
        process(painter, pixmap);
    };
    virtual void drawObjectSelection(QPainter& painter)
    {
        drawObjectSelectionRect(painter, boundingRect());
    };
    // When the tool is selected, this is called when the mouse moves
    virtual void paintMousePreview(QPainter& painter,
                                   const CaptureContext& context) = 0;

    // Move tool objects
    virtual void move(const QPoint& pos) { Q_UNUSED(pos) };
    virtual const QPoint* pos() { return nullptr; };

signals:
    void requestAction(Request r);

protected:
    void copyParams(const CaptureTool* from, CaptureTool* to)
    {
        to->m_count = from->m_count;
        to->m_dropShadowEnabled = from->m_dropShadowEnabled;
        to->m_pixmapCacheKey = from->m_pixmapCacheKey;
        to->m_pixmapCachePosition = from->m_pixmapCachePosition;
    }

    QString iconPath(const QColor& c) const
    {
        return ColorUtils::colorIsDark(c) ? PathInfo::whiteIconPath()
                                          : PathInfo::blackIconPath();
    }

    void drawObjectSelectionRect(QPainter& painter, QRect rect)
    {
        QPen orig_pen = painter.pen();
        painter.setPen(QPen(Qt::black, 3));
        painter.drawRect(rect);
        painter.setPen(QPen(Qt::white, 1, Qt::DotLine));
        painter.drawRect(rect);
        painter.setPen(orig_pen);
    }
    virtual void drawDropShadow(QPainter& painter, const QPixmap& pixmap) = 0;

    QPixmap renderShapeWithShadow(QSize pixmapSize)
    {
        int offset = 3;
        QColor shadowColor = QColor(0, 0, 0, 128);
        qreal blurRadius = 10.0;
        QPixmap shapePixmap(pixmapSize);
        shapePixmap.fill(Qt::transparent);

        // Draw the shape into an offscreen pixmap
        QPainter shapePainter(&shapePixmap);
        shapePainter.setRenderHint(QPainter::Antialiasing);
        process(shapePainter, shapePixmap);
        shapePainter.end();

        // Create a pixmap item and apply the drop shadow effect
        QGraphicsPixmapItem* item = new QGraphicsPixmapItem(shapePixmap);
        QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect;
        shadowEffect->setBlurRadius(blurRadius);
        shadowEffect->setOffset(offset, offset);
        shadowEffect->setColor(shadowColor);
        item->setGraphicsEffect(shadowEffect);

        // Render the item with shadow onto a new pixmap via QGraphicsScene
        QGraphicsScene scene;
        scene.addItem(item);
        QImage finalRender(pixmapSize, QImage::Format_ARGB32_Premultiplied);
        finalRender.fill(Qt::transparent);

        QPainter scenePainter(&finalRender);
        scene.render(&scenePainter);

        // Trim the image render, noting the position bounds
        m_pixmapCachePosition = findContentBounds(finalRender);
        if (m_pixmapCachePosition.isEmpty()) {
            return { QPixmap() };
        }
        return QPixmap::fromImage(finalRender.copy(m_pixmapCachePosition));
    }

    QRect findContentBounds(const QImage& image) {
        const int width = image.width();
        const int height = image.height();
        if (width == 0 || height == 0) return QRect();

        int top = 0, bottom = height - 1, left = 0, right = width - 1;

        auto hasAlphaInRow = [&](int y) -> bool {
            const QRgb* line = reinterpret_cast<const QRgb*>(image.scanLine(y));
            for (int x = 0; x < width; ++x)
                if (qAlpha(line[x]) != 0) return true;
            return false;
        };

        auto hasAlphaInColumn = [&](int x) -> bool {
            for (int y = top; y <= bottom; ++y) {
                const QRgb* line = reinterpret_cast<const QRgb*>(image.scanLine(y));
                if (qAlpha(line[x]) != 0) return true;
            }
            return false;
        };

        while (top <= bottom && !hasAlphaInRow(top))    ++top;
        while (bottom >= top && !hasAlphaInRow(bottom)) --bottom;
        while (left <= right && !hasAlphaInColumn(left)) ++left;
        while (right >= left && !hasAlphaInColumn(right)) --right;

        if (right < left || bottom < top)
            return QRect(); // Fully transparent

        return QRect(left, top, right - left + 1, bottom - top + 1);
    }

public slots:
    // On mouse release.
    virtual void drawEnd(const QPoint& p) = 0;
    // Mouse pressed and moving, called once a pixel.
    virtual void drawMove(const QPoint& p) = 0;
    // Called when drawMove is needed with an adjustment;
    // should be overridden in case an adjustment is applicable.
    virtual void drawMoveWithAdjustment(const QPoint& p) { drawMove(p); }
    // Called when the tool is activated.
    virtual void drawStart(const CaptureContext& context) = 0;
    // Called right after pressing the button which activates the tool.
    virtual void pressed(CaptureContext& context) = 0;
    // Called when the color is changed in the editor.
    virtual void onColorChanged(const QColor& c) = 0;
    // Called when the drop shadow is changed in the editor.
    virtual void onDropShadowChanged(bool enabled)
    {
        m_dropShadowEnabled = enabled;
    }
    // Called when the size the tool size is changed by the user.
    virtual void onSizeChanged(int size) = 0;
    virtual int size() const { return -1; };

private:
    unsigned int m_count;
    bool m_editMode;
    bool m_dropShadowEnabled = false;
    QPixmapCache::Key m_pixmapCacheKey;
    QRect m_pixmapCachePosition;
};
