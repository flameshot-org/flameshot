// Copyright 2017 Alejandro Sirgo Rica
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

#ifndef BUTTON_H
#define BUTTON_H

#include <QPushButton>
#include <QMap>
#include <QVector>

class QWidget;
class QPropertyAnimation;
class CaptureTool;

class CaptureButton : public QPushButton {
    Q_OBJECT
    Q_ENUMS(ButtonType)

public:
    // Don't forget to add the new types to CaptureButton::iterableButtonTypes
    // in the .cpp and the order value in the private array buttonTypeOrder
    enum ButtonType {
        TYPE_PENCIL,
        TYPE_LINE,
        TYPE_ARROW,
        TYPE_SELECTION,
        TYPE_RECTANGLE,
        TYPE_CIRCLE,
        TYPE_MARKER,
        TYPE_SELECTIONINDICATOR,
        TYPE_MOVESELECTION,
        TYPE_UNDO,
        TYPE_COPY,
        TYPE_SAVE,
        TYPE_EXIT,
        TYPE_IMAGEUPLOADER,
    };

    CaptureButton() = delete;
    explicit CaptureButton(const ButtonType, QWidget *parent = nullptr);

    static size_t buttonBaseSize();
    static bool iconIsWhiteByColor(const QColor &);
    static QString globalStyleSheet();
    static QVector<CaptureButton::ButtonType> getIterableButtonTypes();
    static int getPriorityByButton(CaptureButton::ButtonType);

    QString name() const;
    QString description() const;
    QIcon icon() const;
    QString styleSheet() const;
    ButtonType buttonType() const;
    CaptureTool* tool() const;

    void setColor(const QColor &c);
    void animatedShow();

protected:
    virtual void mousePressEvent(QMouseEvent *);
    static QVector<ButtonType> iterableButtonTypes;

    CaptureTool *m_tool;

signals:
    void pressedButton(CaptureButton *);

private:
    CaptureButton(QWidget *parent = 0);
    ButtonType m_buttonType;

    QPropertyAnimation *m_emergeAnimation;

    static QColor m_mainColor;

    void initButton();

};

#endif // BUTTON_H
