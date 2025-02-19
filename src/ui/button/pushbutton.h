#pragma once
#include "imgui.h"
#include "abstractbutton.h"
#include <string>

namespace UI
{
    class PushButton : public AbstractButton
    {
    public:
        explicit PushButton(std::string &label);
        ~PushButton();

        bool Render();

        void SetSize(const ImVec2 &size);

        void SetColor(const ImVec4 &color);

        void OnClickedEvent() override;

    private:
        std::string m_label_ { "" };
        bool m_need_pop_color_ = false;
    };
}; // namespace UI
