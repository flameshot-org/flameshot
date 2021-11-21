/**
 * \file
 *
 * \author Mattia Basaglia
 *
 * \copyright Copyright (C) 2013-2020 Mattia Basaglia
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef COLOR_WIDGETS_COLOR_LINE_EDIT_HPP
#define COLOR_WIDGETS_COLOR_LINE_EDIT_HPP

#include "colorwidgets_global.hpp"
#include <QLineEdit>
#include <QColor>

namespace color_widgets {

/**
 * \brief A line edit used to define a color name
 *
 * Supported string formats:
 *  * Short hex strings #f00
 *  * Long hex strings  #ff0000
 *  * Color names       red
 *  * Function-like     rgb(255,0,0)
 *
 * Additional string formats supported when showAlpha is true:
 *  * Long hex strings  #ff0000ff
 *  * Function like     rgba(255,0,0,255)
 */
class QCP_EXPORT ColorLineEdit : public QLineEdit
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    /**
     * \brief Whether the widget displays and edits the alpha channel
     */
    Q_PROPERTY(bool showAlpha READ showAlpha WRITE setShowAlpha NOTIFY showAlphaChanged)
    /**
     * \brief If \b true, the background of the widget is changed to show the color
     */
    Q_PROPERTY(bool previewColor READ previewColor WRITE setPreviewColor NOTIFY previewColorChanged)

public:
    explicit ColorLineEdit(QWidget* parent = nullptr);
    ~ColorLineEdit();

    QColor color() const;
    bool showAlpha() const;
    bool previewColor() const;

public Q_SLOTS:
    void setColor(const QColor& color);
    void setShowAlpha(bool showAlpha);
    void setPreviewColor(bool previewColor);

Q_SIGNALS:
    /**
     * \brief Emitted when the color is changed by any means
     */
    void colorChanged(const QColor& color);
    /**
     * \brief Emitted when the user is typing a color but has not finished yet
     */
    void colorEdited(const QColor& color);
    /**
     * \brief Emitted when the user finished to edit a string
     */
    void colorEditingFinished(const QColor& color);

    void showAlphaChanged(bool showAlpha);
    void previewColorChanged(bool previewColor);

protected:
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent * event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;

private:
    class Private;
    Private* p;
};

} // namespace color_widgets
#endif // COLOR_WIDGETS_COLOR_LINE_EDIT_HPP
