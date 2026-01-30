#pragma once
#include "imgui.h"
#include "abstractbutton.h"

#include <functional>
#include <string>

namespace UI
{
    class PushButton : public AbstractButton
    {
    public:
        explicit PushButton(std::string &label);
        ~PushButton() override;

        bool Render();

        void SetSize(const ImVec2 &size);

        void SetColor(const ImVec4 &color);

        void OnClickedEvent(WidgetObject *widget) override;

        void SetOnClick(std::function<void()> callback);

    private:
        std::string m_label_ { "" };
        bool m_need_pop_color_ = false;

        ImVec2 m_size_{ 0, 0 };
        std::function<void()> m_callback_;
    };
}; // namespace UI
