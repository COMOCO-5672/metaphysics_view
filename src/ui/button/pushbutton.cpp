#include "pushbutton.h"

namespace UI
{
    PushButton::PushButton(std::string &label) : m_label_(label) {}

    PushButton::~PushButton()
    {
        if (m_need_pop_color_) {
            ImGui::PopStyleColor();
        }
    }

    bool PushButton::Render()
    {
        return ImGui::Button(m_label_.c_str());
    }

    void PushButton::SetSize(const ImVec2 &size)
    {
        ImGui::SetItemDefaultFocus();
    }

    void PushButton::SetColor(const ImVec4 &color)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, color);
        m_need_pop_color_ = true;
    }

} // namespace UI