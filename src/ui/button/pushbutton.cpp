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
        if (m_size_.x > 0.0f && m_size_.y > 0.0f) {
            ImGui::SetNextItemWidth(m_size_.x);
        }

        if (ImGui::Button(m_label_.c_str(), m_size_)) {
            if (m_callback_) {
                m_callback_();
            }

            OnClickedEvent(this);
            return true;
        }
        return false;
    }

    void PushButton::SetSize(const ImVec2 &size)
    {
        m_size_ = size;
    }

    void PushButton::SetColor(const ImVec4 &color)
    {
        if (m_need_pop_color_) {
            ImGui::PopStyleColor(3);
        }
        ImGui::PushStyleColor(ImGuiCol_Button, color);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.x * 1.2f, color.y * 1.2f, color.z * 1.2f, color.w));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(color.x * 0.8f, color.y * 0.8f, color.z * 0.8f, color.w));
        m_need_pop_color_ = true;
    }

    void PushButton::OnClickedEvent(WidgetObject* widget)
    {

    }

    void PushButton::SetOnClick(std::function<void()> callback)
    {
        m_callback_ = std::move(callback);
    }

} // namespace UI