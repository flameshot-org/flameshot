#pragma once

#include <QRect>

namespace layoututils
{
bool adjustRectInsideAnother(const QRect &parent, QRect &inner);;
}