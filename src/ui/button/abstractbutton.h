#pragma once

namespace UI {
    class AbstractButton
    {
    public:
        AbstractButton() = default;
        virtual ~AbstractButton() = default;

        virtual void OnClickedEvent() = 0;

        

    };    
}

