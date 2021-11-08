/**
 * \file
 *
 * \author Mattia Basaglia
 *
 * \copyright Copyright (C) 2013-2020 Mattia Basaglia
 * \copyright Copyright (C) 2014 Calle Laakkonen
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
#include "QtColorWidgets/color_dialog.hpp"
#include "ui_color_dialog.h"

#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QPushButton>
#include <QScreen>

#include "QtColorWidgets/color_utils.hpp"

#include <QDebug>
namespace color_widgets {

class ColorDialog::Private
{
public:
    Ui_ColorDialog ui;
    ButtonMode button_mode;
    bool pick_from_screen;
    bool alpha_enabled;
    QColor color;

    Private() : pick_from_screen(false), alpha_enabled(true)
    {}

#ifdef Q_OS_ANDROID
    void screen_rotation()
    {
        auto scr = QApplication::primaryScreen();
        if ( scr->size().height() > scr->size().width() )
            ui.layout_main->setDirection(QBoxLayout::TopToBottom);
        else
            ui.layout_main->setDirection(QBoxLayout::LeftToRight);
    }
#endif
};

ColorDialog::ColorDialog(QWidget *parent, Qt::WindowFlags f) :
    QDialog(parent, f), p(new Private)
{
    p->ui.setupUi(this);

    setAcceptDrops(true);

#ifdef Q_OS_ANDROID
    connect(
        QGuiApplication::primaryScreen(),
        &QScreen::primaryOrientationChanged,
        this,
        [this]{p->screen_rotation();}
    );
    p->ui.wheel->setWheelWidth(50);
    p->screen_rotation();
#else
    // Add "pick color" button
    QPushButton *pickButton = p->ui.buttonBox->addButton(tr("Pick"), QDialogButtonBox::ActionRole);
    pickButton->setIcon(QIcon::fromTheme(QStringLiteral("color-picker")));
#endif

    setButtonMode(OkApplyCancel);

    connect(p->ui.wheel, &ColorWheel::colorSpaceChanged, this, &ColorDialog::colorSpaceChanged);
    connect(p->ui.wheel, &ColorWheel::selectorShapeChanged, this, &ColorDialog::wheelShapeChanged);
    connect(p->ui.wheel, &ColorWheel::rotatingSelectorChanged, this, &ColorDialog::wheelRotatingChanged);

}

ColorDialog::~ColorDialog()
{
    delete p;
}

QSize ColorDialog::sizeHint() const
{
    return QSize(400,0);
}

QColor ColorDialog::color() const
{
    QColor col = p->color;
    if ( !p->alpha_enabled )
        col.setAlpha(255);
    return col;
}

void ColorDialog::setColor(const QColor &c)
{
    p->ui.preview->setComparisonColor(c);
    p->ui.edit_hex->setModified(false);
    setColorInternal(c);
}

void ColorDialog::showColor(const QColor &c)
{
    setColor(c);
    show();
}

void ColorDialog::setPreviewDisplayMode(ColorPreview::DisplayMode mode)
{
    p->ui.preview->setDisplayMode(mode);
}

ColorPreview::DisplayMode ColorDialog::previewDisplayMode() const
{
    return p->ui.preview->displayMode();
}

void ColorDialog::setAlphaEnabled(bool a)
{
    if ( a != p->alpha_enabled )
    {
        p->alpha_enabled = a;

        p->ui.edit_hex->setShowAlpha(a);
        p->ui.line_alpha->setVisible(a);
        p->ui.label_alpha->setVisible(a);
        p->ui.slide_alpha->setVisible(a);
        p->ui.spin_alpha->setVisible(a);

        Q_EMIT alphaEnabledChanged(a);
    }
}

bool ColorDialog::alphaEnabled() const
{
    return p->alpha_enabled;
}

void ColorDialog::setButtonMode(ButtonMode mode)
{
    p->button_mode = mode;
    QDialogButtonBox::StandardButtons btns;
    switch(mode) {
        case OkCancel: btns = QDialogButtonBox::Ok | QDialogButtonBox::Cancel; break;
        case OkApplyCancel: btns = QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply | QDialogButtonBox::Reset; break;
        case Close: btns = QDialogButtonBox::Close;
    }
    p->ui.buttonBox->setStandardButtons(btns);
}

ColorDialog::ButtonMode ColorDialog::buttonMode() const
{
    return p->button_mode;
}

void ColorDialog::setColorInternal(const QColor &col)
{
    /**
     * \note Unlike setColor, this is used to update the current color which
     * migth differ from the final selected color
     */
    p->ui.wheel->setColor(col);

    p->color = col;

    bool blocked = signalsBlocked();
    blockSignals(true);
    Q_FOREACH(QWidget* w, findChildren<QWidget*>())
        w->blockSignals(true);


    p->ui.slide_red->setValue(col.red());
    p->ui.spin_red->setValue(p->ui.slide_red->value());
    p->ui.slide_red->setFirstColor(QColor(0,col.green(),col.blue()));
    p->ui.slide_red->setLastColor(QColor(255,col.green(),col.blue()));

    p->ui.slide_green->setValue(col.green());
    p->ui.spin_green->setValue(p->ui.slide_green->value());
    p->ui.slide_green->setFirstColor(QColor(col.red(),0,col.blue()));
    p->ui.slide_green->setLastColor(QColor(col.red(),255,col.blue()));

    p->ui.slide_blue->setValue(col.blue());
    p->ui.spin_blue->setValue(p->ui.slide_blue->value());
    p->ui.slide_blue->setFirstColor(QColor(col.red(),col.green(),0));
    p->ui.slide_blue->setLastColor(QColor(col.red(),col.green(),255));

    p->ui.slide_hue->setValue(qRound(p->ui.wheel->hue()*360.0));
    p->ui.slide_hue->setColorSaturation(p->ui.wheel->saturation());
    p->ui.slide_hue->setColorValue(p->ui.wheel->value());
    p->ui.spin_hue->setValue(p->ui.slide_hue->value());

    p->ui.slide_saturation->setValue(qRound(p->ui.wheel->saturation()*255.0));
    p->ui.spin_saturation->setValue(p->ui.slide_saturation->value());
    p->ui.slide_saturation->setFirstColor(QColor::fromHsvF(p->ui.wheel->hue(),0,p->ui.wheel->value()));
    p->ui.slide_saturation->setLastColor(QColor::fromHsvF(p->ui.wheel->hue(),1,p->ui.wheel->value()));

    p->ui.slide_value->setValue(qRound(p->ui.wheel->value()*255.0));
    p->ui.spin_value->setValue(p->ui.slide_value->value());
    p->ui.slide_value->setFirstColor(QColor::fromHsvF(p->ui.wheel->hue(), p->ui.wheel->saturation(),0));
    p->ui.slide_value->setLastColor(QColor::fromHsvF(p->ui.wheel->hue(), p->ui.wheel->saturation(),1));


    QColor apha_color = col;
    apha_color.setAlpha(0);
    p->ui.slide_alpha->setFirstColor(apha_color);
    apha_color.setAlpha(255);
    p->ui.slide_alpha->setLastColor(apha_color);
    p->ui.spin_alpha->setValue(col.alpha());
    p->ui.slide_alpha->setValue(col.alpha());

    if ( !p->ui.edit_hex->isModified() )
        p->ui.edit_hex->setColor(col);

    p->ui.preview->setColor(col);

    blockSignals(blocked);
    Q_FOREACH(QWidget* w, findChildren<QWidget*>())
        w->blockSignals(false);

    Q_EMIT colorChanged(col);
}

void ColorDialog::set_hsv()
{
    if ( !signalsBlocked() )
    {
        QColor col = QColor::fromHsv(
            p->ui.slide_hue->value(),
            p->ui.slide_saturation->value(),
            p->ui.slide_value->value(),
            p->ui.slide_alpha->value()
        );
        p->ui.wheel->setColor(col);
        setColorInternal(col);
    }
}

void ColorDialog::set_alpha()
{
    if ( !signalsBlocked() )
    {
        QColor col = p->color;
        col.setAlpha(p->ui.slide_alpha->value());
        setColorInternal(col);
    }
}

void ColorDialog::set_rgb()
{
    if ( !signalsBlocked() )
    {
        QColor col(
                p->ui.slide_red->value(),
                p->ui.slide_green->value(),
                p->ui.slide_blue->value(),
                p->ui.slide_alpha->value()
            );
        if (col.saturation() == 0)
            col = QColor::fromHsv(p->ui.slide_hue->value(), 0, col.value());
        p->ui.wheel->setColor(col);
        setColorInternal(col);
    }
}

void ColorDialog::on_edit_hex_colorChanged(const QColor& color)
{
    setColorInternal(color);
}

void ColorDialog::on_edit_hex_colorEditingFinished(const QColor& color)
{
    p->ui.edit_hex->setModified(false);
    setColorInternal(color);
}

void ColorDialog::on_buttonBox_clicked(QAbstractButton *btn)
{
    QDialogButtonBox::ButtonRole role = p->ui.buttonBox->buttonRole(btn);

    switch(role) {
    case QDialogButtonBox::AcceptRole:
    case QDialogButtonBox::ApplyRole:
        // Explicitly select the color
        p->ui.preview->setComparisonColor(color());
        Q_EMIT colorSelected(color());
        break;

    case QDialogButtonBox::ActionRole:
        // Currently, the only action button is the "pick color" button
        grabMouse(Qt::CrossCursor);
        p->pick_from_screen = true;
        break;

    case QDialogButtonBox::ResetRole:
        // Restore old color
        setColorInternal(p->ui.preview->comparisonColor());
        break;

    default: break;
    }
}

void ColorDialog::dragEnterEvent(QDragEnterEvent *event)
{
    if ( event->mimeData()->hasColor() ||
         ( event->mimeData()->hasText() && QColor(event->mimeData()->text()).isValid() ) )
        event->acceptProposedAction();
}


void ColorDialog::dropEvent(QDropEvent *event)
{
    if ( event->mimeData()->hasColor() )
    {
        setColorInternal(event->mimeData()->colorData().value<QColor>());
        event->accept();
    }
    else if ( event->mimeData()->hasText() )
    {
        QColor col(event->mimeData()->text());
        if ( col.isValid() )
        {
            setColorInternal(col);
            event->accept();
        }
    }
}

void ColorDialog::mouseReleaseEvent(QMouseEvent *event)
{
    if (p->pick_from_screen)
    {
        setColorInternal(utils::get_screen_color(event->globalPos()));
        p->pick_from_screen = false;
        releaseMouse();
    }
}

void ColorDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (p->pick_from_screen)
    {
        setColorInternal(utils::get_screen_color(event->globalPos()));
    }
}

void ColorDialog::keyReleaseEvent(QKeyEvent *ev)
{
    QDialog::keyReleaseEvent(ev);

#ifdef Q_OS_ANDROID
    if ( ev->key() == Qt::Key_Back )
    {
        reject();
        ev->accept();
    }
#endif
}

void ColorDialog::setWheelShape(ColorWheel::ShapeEnum shape)
{
    p->ui.wheel->setSelectorShape(shape);
}

ColorWheel::ShapeEnum ColorDialog::wheelShape() const
{
    return p->ui.wheel->selectorShape();
}

void ColorDialog::setColorSpace(ColorWheel::ColorSpaceEnum space)
{
    p->ui.wheel->setColorSpace(space);
}

ColorWheel::ColorSpaceEnum ColorDialog::colorSpace() const
{
    return p->ui.wheel->colorSpace();
}

void ColorDialog::setWheelRotating(bool rotating)
{
    p->ui.wheel->setRotatingSelector(rotating);
}

bool ColorDialog::wheelRotating() const
{
    return p->ui.wheel->rotatingSelector();
}

int ColorDialog::exec()
{
#if defined(Q_OS_ANDROID) && !defined(Q_OS_ANDROID_FAKE)
    showMaximized();
    setFocus();
#endif
    return QDialog::exec();
}

} // namespace color_widgets
