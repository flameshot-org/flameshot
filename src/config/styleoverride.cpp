//
// Created by jeremy on 9/24/20.
//
#include "styleoverride.h"

int StyleOverride::styleHint(StyleHint hint,
                             const QStyleOption* option,
                             const QWidget* widget,
                             QStyleHintReturn* returnData) const
{
    if (hint == SH_ToolTip_WakeUpDelay) {
        return 600;
    } else {
        return baseStyle()->styleHint(hint, option, widget, returnData);
    }
}
