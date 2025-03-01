#pragma once

#include "widgetobject.h"

namespace UI {
    class AbstractButton : public WidgetObject
    {
    public:
        AbstractButton() = default;
        virtual ~AbstractButton() = default;

        virtual void OnClickedEvent(WidgetObject* widget) = 0;
    };    
}

