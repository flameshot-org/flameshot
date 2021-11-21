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
#ifndef COLOR_SELECTOR_HPP
#define COLOR_SELECTOR_HPP

#include "color_preview.hpp"
#include "color_wheel.hpp"

namespace color_widgets {

/**
 * Color preview that opens a color dialog
 */
class QCP_EXPORT ColorSelector : public ColorPreview
{
    Q_OBJECT
    Q_ENUMS(UpdateMode)
    Q_PROPERTY(UpdateMode updateMode READ updateMode WRITE setUpdateMode NOTIFY updateModeChanged)
    Q_PROPERTY(Qt::WindowModality dialogModality READ dialogModality WRITE setDialogModality NOTIFY dialogModalityChanged)
    Q_PROPERTY(ColorWheel::ShapeEnum wheelShape READ wheelShape WRITE setWheelShape NOTIFY wheelShapeChanged)
    Q_PROPERTY(ColorWheel::ColorSpaceEnum colorSpace READ colorSpace WRITE setColorSpace NOTIFY colorSpaceChanged)
    Q_PROPERTY(bool wheelRotating READ wheelRotating WRITE setWheelRotating NOTIFY wheelRotatingChanged)

public:
    enum UpdateMode {
        Confirm, ///< Update color only after the dialog has been accepted
        Continuous ///< Update color as it's being modified in the dialog
    };

    explicit ColorSelector(QWidget *parent = 0);
    ~ColorSelector();

    void setUpdateMode(UpdateMode m);
    UpdateMode updateMode() const;

    Qt::WindowModality dialogModality() const;
    void setDialogModality(Qt::WindowModality m);

    ColorWheel::ShapeEnum wheelShape() const;
    ColorWheel::ColorSpaceEnum colorSpace() const;
    bool wheelRotating() const;

Q_SIGNALS:
    void wheelShapeChanged(ColorWheel::ShapeEnum shape);
    void colorSpaceChanged(ColorWheel::ColorSpaceEnum space);
    void wheelRotatingChanged(bool rotating);
    void updateModeChanged(UpdateMode);
    void dialogModalityChanged(Qt::WindowModality);
    /// Emitted when a color is selected by the user
    void colorSelected(const QColor& c);

public Q_SLOTS:
    void showDialog();
    void setWheelShape(ColorWheel::ShapeEnum shape);
    void setColorSpace(ColorWheel::ColorSpaceEnum space);
    void setWheelRotating(bool rotating);

private Q_SLOTS:
    void accept_dialog();
    void reject_dialog();
    void update_old_color(const QColor &c);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent * event);

private:
    /// Connect/Disconnect colorChanged based on UpdateMode
    void connect_dialog();

    /// Disconnect from dialog update
    void disconnect_dialog();

    class Private;
    Private * const p;
};

} // namespace color_widgets

#endif // COLOR_SELECTOR_HPP
