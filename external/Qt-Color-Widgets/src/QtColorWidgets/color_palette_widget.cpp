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
#include "QtColorWidgets/color_palette_widget.hpp"
#include "ui_color_palette_widget.h"
#include "QtColorWidgets/color_dialog.hpp"
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QImageReader>

namespace color_widgets {

class ColorPaletteWidget::Private : public Ui::ColorPaletteWidget
{
public:
    ColorPaletteModel* model = nullptr;
    bool read_only = false;
    QColor default_color;

    bool hasSelectedPalette()
    {
        return model && palette_list->currentIndex() != -1;
    }

    const ColorPalette& selectedPalette()
    {
        return model->palette(palette_list->currentIndex());
    }

    void addPalette(ColorPalette& palette)
    {
        bool save = false;
        // Save palettes in the savePath
        /// \todo This currently breaks opening the right directory
        /// ie: the one containing the original file.
        if ( !palette.fileName().isEmpty() )
        {
            QFileInfo file(palette.fileName());
            if ( file.dir().canonicalPath() != QDir(model->savePath()).canonicalPath() )
            {
                palette.setFileName(QString());
                save = true;
            }
        }
        model->addPalette(palette, save);
        palette_list->setCurrentIndex(model->count()-1);
    }

    bool openImage(const QString& file)
    {
        QImage image(file);
        if ( !image.isNull() )
        {
            ColorPalette palette;
            palette.loadImage(image);
            palette.setName(QFileInfo(file).baseName());
            palette.setFileName(file+".gpl");
            addPalette(palette);
            return true;
        }
        return false;
    }

    bool openGpl(const QString& file)
    {
        int existing = model->indexFromFile(file);
        if ( existing != -1 )
        {
            palette_list->setCurrentIndex(existing);
            return true;
        }

        ColorPalette palette;
        if ( palette.load(file) )
        {
            addPalette(palette);
            return true;
        }

        return false;
    }

    bool openPalette(const QString& file, int type)
    {
        if ( type == 1 )
            return openImage(file);
        return openGpl(file);
    }
};


ColorPaletteWidget::ColorPaletteWidget(QWidget* parent)
    : QWidget(parent), p(new Private)
{
    p->setupUi(this);

    // Connext Swatch signals
    connect(p->swatch, &Swatch::colorSizeChanged, this, &ColorPaletteWidget::colorSizeChanged);
    connect(p->swatch, &Swatch::colorSizePolicyChanged, this, &ColorPaletteWidget::colorSizePolicyChanged);
    connect(p->swatch, &Swatch::forcedRowsChanged, this, &ColorPaletteWidget::forcedRowsChanged);
    connect(p->swatch, &Swatch::forcedColumnsChanged, this, &ColorPaletteWidget::forcedColumnsChanged);
    connect(p->swatch, &Swatch::colorSelected,
            this, (void (ColorPaletteWidget::*)(const QColor&)) &ColorPaletteWidget::currentColorChanged);
    connect(p->swatch, &Swatch::selectedChanged,
            this, (void (ColorPaletteWidget::*)(int)) &ColorPaletteWidget::currentColorChanged);
    connect(p->swatch, &Swatch::borderChanged, this, &ColorPaletteWidget::borderChanged);
    connect(p->swatch, &Swatch::paletteChanged, this, &ColorPaletteWidget::currentPaletteChanged);

    connect(&p->swatch->palette(), &ColorPalette::dirtyChanged, p->button_palette_save, &QWidget::setEnabled);
    connect(&p->swatch->palette(), &ColorPalette::dirtyChanged, p->button_palette_revert, &QWidget::setEnabled);

    connect(p->palette_list, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged,
            this, &ColorPaletteWidget::currentRowChanged);

    // Buttons changing the colors in the current palette
    connect(p->button_color_add, &QAbstractButton::clicked, [this](){
        if ( !p->read_only && p->hasSelectedPalette() )
        {
            if ( p->default_color.isValid() )
            {
                p->swatch->palette().appendColor(p->default_color);
                p->swatch->setSelected(p->swatch->palette().count()-1);
            }
            else
            {
                ColorDialog dialog(this);
                dialog.setAlphaEnabled(false);
                if ( p->swatch->selected() != -1 )
                    dialog.setColor(p->swatch->selectedColor());
                if ( dialog.exec() )
                {
                    p->swatch->palette().appendColor(dialog.color());
                    p->swatch->setSelected(p->swatch->palette().count()-1);
                }
            }
        }
    });
    connect(p->button_color_remove, &QAbstractButton::clicked, p->swatch, &Swatch::removeSelected);

    // Buttons modifying the current palette file
    connect(p->button_palette_delete, &QAbstractButton::clicked, [this]() {
        if ( !p->read_only && p->hasSelectedPalette() )
            p->model->removePalette(p->palette_list->currentIndex());
    });
    connect(p->button_palette_save, &QAbstractButton::clicked, [this](){
        if ( !p->read_only && p->hasSelectedPalette() && p->swatch->palette().dirty() )
            if ( p->model->updatePalette( p->palette_list->currentIndex(), p->swatch->palette() ) )
            {
                p->swatch->palette().setDirty(false);
            }
            /// \todo else ask for a file name (?)
    });
    connect(p->button_palette_revert, &QAbstractButton::clicked, [this](){
        if ( p->hasSelectedPalette() )
        {
             p->swatch->setPalette(p->selectedPalette());
        }
    });

    // Buttons creating new palettes
    connect(p->button_palette_duplicate, &QAbstractButton::clicked, [this](){
        if ( p->hasSelectedPalette() )
        {
            ColorPalette new_palette = p->selectedPalette();
            new_palette.setFileName(QString());
            bool ok = false;
            QString name = QInputDialog::getText(this, tr("New Palette"),
                tr("Name"), QLineEdit::Normal, new_palette.name(), &ok);
            if ( ok )
            {
                new_palette.setName(name);
                p->model->addPalette(new_palette);
                p->palette_list->setCurrentIndex(p->model->count()-1);
            }
        }
    });
    /// \todo Show a dialog that asks for the number of columns (?)
    connect(p->button_palette_new, &QAbstractButton::clicked, [this](){
        if ( p->model )
        {
            bool ok = false;
            QString name = QInputDialog::getText(this, tr("New Palette"),
                tr("Name"), QLineEdit::Normal, QString(), &ok);
            if ( ok )
            {
                ColorPalette new_palette(name);
                p->model->addPalette(new_palette);
                p->palette_list->setCurrentIndex(p->model->count()-1);
            }
        }
    });

    QString image_formats;
    Q_FOREACH(QByteArray ba, QImageReader::supportedImageFormats())
        image_formats += " *."+QString(ba);

    connect(p->button_palette_open, &QAbstractButton::clicked, [this, image_formats](){
        if ( p->model )
        {
            QString default_dir;
            if ( p->hasSelectedPalette() )
            {
                const ColorPalette& palette = p->selectedPalette();
                if ( !palette.fileName().isEmpty() )
                    default_dir = QFileInfo(palette.fileName()).dir().path();
            }

            QStringList file_formats = QStringList()
                << tr("GIMP Palettes (*.gpl)")
                << tr("Palette Image (%1)").arg(image_formats)
                << tr("All Files (*)");
            QFileDialog open_dialog(this, tr("Open Palette"), default_dir);
            open_dialog.setFileMode(QFileDialog::ExistingFile);
            open_dialog.setAcceptMode(QFileDialog::AcceptOpen);
            open_dialog.setNameFilters(file_formats);

            if ( !open_dialog.exec() )
                return;

            int type = file_formats.indexOf(open_dialog.selectedNameFilter());
            QString file_name = open_dialog.selectedFiles()[0];

            if ( !p->openPalette( file_name, type ) )
            {
                QMessageBox::warning(this, tr("Open Palette"),
                    tr("Failed to load the palette file\n%1").arg(file_name));
            }
        }
    });
}

ColorPaletteWidget::~ColorPaletteWidget() = default;

ColorPaletteModel* ColorPaletteWidget::model() const
{
    return p->model;
}

const ColorPalette& ColorPaletteWidget::currentPalette() const
{
    return p->swatch->palette();
}

QSize ColorPaletteWidget::colorSize() const
{
    return p->swatch->colorSize();
}

Swatch::ColorSizePolicy ColorPaletteWidget::colorSizePolicy() const
{
    return p->swatch->colorSizePolicy();
}

QPen ColorPaletteWidget::border() const
{
    return p->swatch->border();
}

int ColorPaletteWidget::forcedRows() const
{
    return p->swatch->forcedRows();
}
int ColorPaletteWidget::forcedColumns() const
{
    return p->swatch->forcedColumns();
}

bool ColorPaletteWidget::readOnly() const
{
    return p->read_only;
}

QColor ColorPaletteWidget::currentColor() const
{
    return p->swatch->selectedColor();
}

void ColorPaletteWidget::setModel(ColorPaletteModel* model)
{
    if ( model == p->model )
        return;

    if ( p->model )
    {
        disconnect(model, &QAbstractItemModel::dataChanged, this, nullptr);
    }

    p->model = model;
    p->swatch->setPalette(ColorPalette());
    p->palette_list->setModel(model);

    if ( model )
    {
        connect(model, &QAbstractItemModel::dataChanged, this, [this](const QModelIndex &index){
            if ( index.row() == p->palette_list->currentIndex() )
                p->swatch->setPalette(p->model->palette(index.row()));
        });
    }
}

void ColorPaletteWidget::setColorSize(const QSize& colorSize)
{
    p->swatch->setColorSize(colorSize);
}

void ColorPaletteWidget::setColorSizePolicy(Swatch::ColorSizePolicy colorSizePolicy)
{
    p->swatch->setColorSizePolicy(colorSizePolicy);
}

void ColorPaletteWidget::setBorder(const QPen& border)
{
    p->swatch->setBorder(border);
}

void ColorPaletteWidget::setForcedRows(int forcedRows)
{
    p->swatch->setForcedRows(forcedRows);
}

void ColorPaletteWidget::setForcedColumns(int forcedColumns)
{
    p->swatch->setForcedColumns(forcedColumns);
}

void ColorPaletteWidget::setReadOnly(bool readOnly)
{
    if ( readOnly == p->read_only )
        return;

    p->swatch->setReadOnly(readOnly);
    p->group_edit_list->setVisible(!readOnly);
    p->group_edit_palette->setVisible(!readOnly);
    Q_EMIT readOnlyChanged(p->read_only = readOnly);
}

bool ColorPaletteWidget::setCurrentColor(const QColor& color)
{
    QColor rgb = color.toRgb();
    const auto& palette = p->swatch->palette();
    for ( int i = 0; i < palette.count(); i++ )
    {
        if ( palette.colorAt(i).toRgb() == rgb )
        {
            p->swatch->setSelected(i);
            return true;
        }
    }

    p->swatch->clearSelection();
    return false;
}

bool ColorPaletteWidget::setCurrentColor(const QString& name)
{
    const auto& palette = p->swatch->palette();
    for ( int i = 0; i < palette.count(); i++ )
    {
        if ( palette.nameAt(i) == name )
        {
            p->swatch->setSelected(i);
            return true;
        }
    }

    p->swatch->clearSelection();
    return false;
}

bool ColorPaletteWidget::setCurrentColor(int index)
{
    const auto& palette = p->swatch->palette();
    if ( index >= 0 && index < palette.count() )
    {
        p->swatch->setSelected(index);
        return true;
    }

    p->swatch->clearSelection();
    return false;
}


void ColorPaletteWidget::on_palette_list_currentIndexChanged(int index)
{
    if ( !p->model )
        p->swatch->setPalette(ColorPalette());
    else
        p->swatch->setPalette(p->model->palette(index));

    p->swatch->palette().setDirty(false);
}

void ColorPaletteWidget::on_swatch_doubleClicked(int index)
{
    if ( !p->read_only )
    {
        ColorDialog dialog(this);
        dialog.setAlphaEnabled(false);
        dialog.setColor(p->swatch->palette().colorAt(index));
        if ( dialog.exec() )
            p->swatch->palette().setColorAt(index, dialog.color());
    }
}

int ColorPaletteWidget::currentRow() const
{
    return p->palette_list->currentIndex();
}

void ColorPaletteWidget::setCurrentRow(int row)
{
    p->palette_list->setCurrentIndex(row);
}

void ColorPaletteWidget::clearCurrentColor()
{
    p->swatch->clearSelection();
}

QColor ColorPaletteWidget::defaultColor() const
{
    return p->default_color;
}


void ColorPaletteWidget::setDefaultColor(const QColor& color)
{
    Q_EMIT(p->default_color = color);
}

} // namespace color_widgets
