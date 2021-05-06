// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "clickablelabel.h"

ClickableLabel::ClickableLabel(QWidget* parent)
  : QLabel(parent)
{}

ClickableLabel::ClickableLabel(QString s, QWidget* parent)
  : QLabel(parent)
{
    setText(s);
}

void ClickableLabel::mousePressEvent(QMouseEvent*)
{
    emit clicked();
}
