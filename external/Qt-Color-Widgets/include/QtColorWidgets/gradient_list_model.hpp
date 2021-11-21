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
#ifndef COLOR_WIDGETS_GRADIENT_LIST_MODEL_HPP
#define COLOR_WIDGETS_GRADIENT_LIST_MODEL_HPP

#include "colorwidgets_global.hpp"

#include <memory>
#include <QAbstractListModel>
#include <QLinearGradient>

namespace color_widgets {

class QCP_EXPORT GradientListModel : public QAbstractListModel
{
    Q_OBJECT

    /**
     * \brief Size of the icon used for the gradient previews
     */
    Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize NOTIFY iconSizeChanged)

    Q_PROPERTY(ItemEditMode editMode READ editMode WRITE setEditMode NOTIFY editModeChanged)

public:
    enum ItemEditMode
    {
        EditNone = 0,
        EditName,
        EditGradient,
    };

    Q_ENUM(ItemEditMode);

    GradientListModel(QObject *parent = nullptr);
    ~GradientListModel();
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex & index) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex & index, const QVariant & value, int role) Q_DECL_OVERRIDE;

    QSize iconSize() const;

    /**
     * \brief Number of gradients
     */
    int count() const;

    /**
     * \brief Remove all gradients
     */
    void clear();

    /**
     * \brief Returns a reference to the first gradient with the given name
     * \pre hasGradient(name)
     */
    const QLinearGradient& gradient(const QString& name) const;

    /**
     * \brief Returns a reference to the first gradient with the given name
     * \pre hasGradient(name)
     */
    QGradientStops gradientStops(const QString& name) const;

    /**
     * \brief Whether a gradient with the given name exists in the model
     */
    bool hasGradient(const QString& name) const;

    /**
     * \brief Get the gradient at the given index (row)
     * \pre 0 <= index < count()
     */
    const QLinearGradient& gradient(int index) const;

    /**
     * \brief Get the gradient stops at the given index (row)
     * \pre 0 <= index < count()
     */
    QGradientStops gradientStops(int index) const;

    /**
     * \brief Inserts or updates a gradient
     * \returns The index for the new gradient
     */
    int setGradient(const QString& name, const QGradient& gradient);

    int setGradient(const QString& name, const QGradientStops& gradient);

    /**
     * \brief Updates the gradient at \p index
     */
    bool setGradient(int index, const QGradient& gradient);

    bool setGradient(int index, const QGradientStops& gradient);

    /**
     * \brief Renames the gradient at \p index
     * \returns \b true on success
     */
    bool rename(int index, const QString& new_name);

    /**
     * \brief Renames a gradient
     * \returns \b true on success
     */
    bool rename(const QString& old_name, const QString& new_name);

    /**
     * \brief Remove a gradient from the model
     * \returns \b true if the gradient has been successfully removed
     */
    bool removeGradient(const QString& name);


    bool removeGradient(int index);

    /**
     * \brief The index of the gradient with the given name
     * \returns -1 if none is found
     */
    int indexFromName(const QString& name) const;

    /**
     * \brief Name of the gradient at index
     */
    QString nameFromIndex(int index) const;

    ItemEditMode editMode() const;

    /**
     * \brief Brush for a gradient
     * \pre 0 <= \p index < count()
     */
    QBrush gradientBrush(int index) const;

public Q_SLOTS:
    void setIconSize(const QSize& iconSize);
    void setEditMode(ItemEditMode mode);

Q_SIGNALS:
    void iconSizeChanged(const QSize& iconSize);
    void editModeChanged(ItemEditMode mode);

private:
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace color_widgets

#endif // COLOR_WIDGETS_GRADIENT_LIST_MODEL_HPP

