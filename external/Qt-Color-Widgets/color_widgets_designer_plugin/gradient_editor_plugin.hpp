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
#ifndef COLOR_WIDGETS_GRADIENT_EDITOR_PLUGIN_HPP
#define COLOR_WIDGETS_GRADIENT_EDITOR_PLUGIN_HPP

#include <QObject>
#include <QtUiPlugin/QDesignerCustomWidgetInterface>

class GradientEditor_Plugin : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    explicit GradientEditor_Plugin(QObject *parent = nullptr);

    void initialize(QDesignerFormEditorInterface *core) Q_DECL_OVERRIDE;
    bool isInitialized() const Q_DECL_OVERRIDE;

    QWidget *createWidget(QWidget *parent) Q_DECL_OVERRIDE;

    QString name() const Q_DECL_OVERRIDE;
    QString group() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    QString toolTip() const Q_DECL_OVERRIDE;
    QString whatsThis() const Q_DECL_OVERRIDE;
    bool isContainer() const Q_DECL_OVERRIDE;

    QString domXml() const Q_DECL_OVERRIDE;

    QString includeFile() const Q_DECL_OVERRIDE;

private:
    bool initialized;
};


#endif // COLOR_WIDGETS_GRADIENT_EDITOR_PLUGIN_HPP

