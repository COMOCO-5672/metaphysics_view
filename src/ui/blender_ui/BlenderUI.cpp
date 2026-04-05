#include "BlenderUI.h"

namespace Metaphysics {

BlenderUI::BlenderUI()
{
    ResetDefaultPanels();
    RebuildLayout();
}

void BlenderUI::SetViewportSize(int width, int height)
{
    m_ViewportWidth = width;
    m_ViewportHeight = height;
    RebuildLayout();
}

void BlenderUI::SetTheme(const BlenderTheme& theme)
{
    m_Theme = theme;
    RebuildLayout();
}

BlenderUIState& BlenderUI::GetState()
{
    return m_State;
}

const BlenderUIState& BlenderUI::GetState() const
{
    return m_State;
}

const BlenderTheme& BlenderUI::GetTheme() const
{
    return m_Theme;
}

const BlenderLayoutSnapshot& BlenderUI::GetLayout() const
{
    return m_Layout;
}

BlenderUIModel BlenderUI::BuildModel(const std::shared_ptr<Scene>& scene,
                                     const std::shared_ptr<Entity>& selectedEntity,
                                     const RenderSettings& renderSettings) const
{
    BlenderUIModel model;
    model.renderSettings = renderSettings;
    model.renderMode = renderSettings.renderMode;

    if (scene) {
        model.sceneName = scene->GetName();
        model.entityCount = scene->GetEntities().size();
    }

    if (selectedEntity) {
        model.selectedEntityName = selectedEntity->GetName();
        auto selectedModel = selectedEntity->GetModel();
        if (selectedModel) {
            model.modelPath = selectedModel->GetPath();
            model.meshCount = selectedModel->GetMeshes().size();
        }
    }

    return model;
}

void BlenderUI::RebuildLayout()
{
    const float width = static_cast<float>(m_ViewportWidth);
    const float height = static_cast<float>(m_ViewportHeight);
    const float topBarHeight = m_Theme.titleBarHeight;
    const float workspaceTabsHeight = m_Theme.headerHeight;
    const float statusBarHeight = m_Theme.statusBarHeight;
    const float toolbarWidth = m_Theme.toolbarWidth;
    const float outlinerWidth = m_Theme.outlinerWidth;
    const float propertiesWidth = m_Theme.propertiesWidth;
    const float viewportHeaderHeight = m_Theme.headerHeight;

    const float contentTop = topBarHeight + workspaceTabsHeight;
    const float contentHeight = height - contentTop - statusBarHeight;
    const float centerLeft = toolbarWidth + outlinerWidth;
    const float centerWidth = width - centerLeft - propertiesWidth;

    m_Layout.topBar = {0.0f, 0.0f, width, topBarHeight};
    m_Layout.workspaceTabs = {0.0f, topBarHeight, width, workspaceTabsHeight};
    m_Layout.toolbar = {0.0f, contentTop, toolbarWidth, contentHeight};
    m_Layout.outliner = {toolbarWidth, contentTop, outlinerWidth, contentHeight};
    m_Layout.viewportHeader = {centerLeft, contentTop, centerWidth, viewportHeaderHeight};
    m_Layout.viewport = {centerLeft, contentTop + viewportHeaderHeight, centerWidth,
                         contentHeight - viewportHeaderHeight};
    m_Layout.properties = {centerLeft + centerWidth, contentTop, propertiesWidth, contentHeight};
    m_Layout.statusBar = {0.0f, height - statusBarHeight, width, statusBarHeight};
}

void BlenderUI::BuildDrawList(const BlenderUIModel& model, BlenderDrawList& drawList) const
{
    drawList.Clear();

    AddFilledRect(drawList, {0.0f, 0.0f, static_cast<float>(m_ViewportWidth),
                  static_cast<float>(m_ViewportHeight)}, m_Theme.appBackground);

    AddFilledRect(drawList, m_Layout.topBar, m_Theme.regionAltBackground);
    AddFilledRect(drawList, m_Layout.workspaceTabs, m_Theme.regionBackground);
    AddFilledRect(drawList, m_Layout.toolbar, m_Theme.regionBackground);
    AddFilledRect(drawList, m_Layout.outliner, m_Theme.regionBackground);
    AddFilledRect(drawList, m_Layout.viewportHeader, m_Theme.regionAltBackground);
    AddFilledRect(drawList, m_Layout.viewport, m_Theme.appBackground);
    AddFilledRect(drawList, m_Layout.properties, m_Theme.regionBackground);
    AddFilledRect(drawList, m_Layout.statusBar, m_Theme.regionAltBackground);

    AddStrokeRect(drawList, m_Layout.topBar, m_Theme.border);
    AddStrokeRect(drawList, m_Layout.workspaceTabs, m_Theme.border);
    AddStrokeRect(drawList, m_Layout.toolbar, m_Theme.border);
    AddStrokeRect(drawList, m_Layout.outliner, m_Theme.border);
    AddStrokeRect(drawList, m_Layout.viewportHeader, m_Theme.border);
    AddStrokeRect(drawList, m_Layout.viewport, m_Theme.border);
    AddStrokeRect(drawList, m_Layout.properties, m_Theme.border);
    AddStrokeRect(drawList, m_Layout.statusBar, m_Theme.border);

    AddText(drawList, 12.0f, 7.0f, m_Theme.textPrimary, "Metaphysics");
    AddText(drawList, 16.0f, m_Layout.workspaceTabs.y + 7.0f, m_Theme.textPrimary, "Layout");
    AddText(drawList, 92.0f, m_Layout.workspaceTabs.y + 7.0f, m_Theme.textMuted, "Modeling");
    AddText(drawList, 188.0f, m_Layout.workspaceTabs.y + 7.0f, m_Theme.textMuted, "Shading");
    AddText(drawList, 276.0f, m_Layout.workspaceTabs.y + 7.0f, m_Theme.textMuted, "Rendering");

    AddText(drawList, m_Layout.outliner.x + 12.0f, m_Layout.outliner.y + 12.0f,
            m_Theme.textPrimary, "Outliner");
    AddText(drawList, m_Layout.outliner.x + 12.0f, m_Layout.outliner.y + 36.0f,
            m_Theme.textMuted, model.sceneName.empty() ? "Scene" : model.sceneName);
    AddText(drawList, m_Layout.outliner.x + 12.0f, m_Layout.outliner.y + 58.0f,
            m_Theme.textMuted, "Entities: " + std::to_string(model.entityCount));

    AddText(drawList, m_Layout.viewportHeader.x + 12.0f, m_Layout.viewportHeader.y + 8.0f,
            m_Theme.textPrimary, "3D Viewport");
    AddText(drawList, m_Layout.viewportHeader.x + 140.0f, m_Layout.viewportHeader.y + 8.0f,
            m_Theme.textMuted, m_State.viewport.usePerspective ? "Perspective" : "Orthographic");
    AddText(drawList, m_Layout.viewportHeader.x + 260.0f, m_Layout.viewportHeader.y + 8.0f,
            m_Theme.textMuted, m_State.viewport.wireframeShading ? "Wireframe" : "Solid");

    AddText(drawList, m_Layout.viewport.x + 24.0f, m_Layout.viewport.y + 24.0f,
            m_Theme.textPrimary, "Independent Blender-like UI");
    AddText(drawList, m_Layout.viewport.x + 24.0f, m_Layout.viewport.y + 48.0f,
            m_Theme.textMuted, "This region is intended for project-owned DX/OpenGL UI rendering.");
    AddText(drawList, m_Layout.viewport.x + 24.0f, m_Layout.viewport.y + 72.0f,
            m_Theme.textMuted, std::string("Selected: ")
            + (model.selectedEntityName.empty() ? std::string("None") : model.selectedEntityName));

    AddText(drawList, m_Layout.properties.x + 12.0f, m_Layout.properties.y + 12.0f,
            m_Theme.textPrimary, "Properties");
    AddText(drawList, m_Layout.properties.x + 12.0f, m_Layout.properties.y + 38.0f,
            m_Theme.textMuted, "Model");
    AddText(drawList, m_Layout.properties.x + 88.0f, m_Layout.properties.y + 38.0f,
            m_Theme.textMuted, "Material");
    AddText(drawList, m_Layout.properties.x + 176.0f, m_Layout.properties.y + 38.0f,
            m_Theme.textMuted, "Render");
    AddText(drawList, m_Layout.properties.x + 12.0f, m_Layout.properties.y + 76.0f,
            m_Theme.textMuted, "Meshes: " + std::to_string(model.meshCount));
    AddText(drawList, m_Layout.properties.x + 12.0f, m_Layout.properties.y + 98.0f,
            m_Theme.textMuted, std::string("Grid: ")
            + (model.renderSettings.showGrid ? std::string("On") : std::string("Off")));

    AddText(drawList, m_Layout.statusBar.x + 10.0f, m_Layout.statusBar.y + 5.0f,
            m_Theme.textMuted,
            "Scene objects: " + std::to_string(model.entityCount) + " | Resolution: "
            + std::to_string(m_ViewportWidth) + "x" + std::to_string(m_ViewportHeight));
}

BlenderHitResult BlenderUI::HitTest(const BlenderInputState& input) const
{
    BlenderHitResult result;

    if (Contains(m_Layout.topBar, input.mouseX, input.mouseY)) {
        result.region = BlenderRegion::TopBar;
        result.hit = true;
        return result;
    }

    if (Contains(m_Layout.workspaceTabs, input.mouseX, input.mouseY)) {
        result.region = BlenderRegion::WorkspaceTabs;
        result.hit = true;
        if (input.leftReleased) {
            if (input.mouseX < 84.0f) {
                result.command = BlenderCommand::SetWorkspaceLayout;
            } else if (input.mouseX < 176.0f) {
                result.command = BlenderCommand::SetWorkspaceModeling;
            } else if (input.mouseX < 264.0f) {
                result.command = BlenderCommand::SetWorkspaceShading;
            } else {
                result.command = BlenderCommand::SetWorkspaceRendering;
            }
        }
        return result;
    }

    if (Contains(m_Layout.toolbar, input.mouseX, input.mouseY)) {
        result.region = BlenderRegion::Toolbar;
        result.hit = true;
        if (input.leftReleased) {
            const float localY = input.mouseY - m_Layout.toolbar.y;
            if (localY < 56.0f) {
                result.command = BlenderCommand::OpenModel;
            } else if (localY < 112.0f) {
                result.command = BlenderCommand::ToggleWireframe;
            } else if (localY < 168.0f) {
                result.command = BlenderCommand::ToggleGrid;
            } else {
                result.command = BlenderCommand::FocusSelection;
            }
        }
        return result;
    }

    if (Contains(m_Layout.outliner, input.mouseX, input.mouseY)) {
        result.region = BlenderRegion::Outliner;
        result.hit = true;
        return result;
    }

    if (Contains(m_Layout.viewportHeader, input.mouseX, input.mouseY)) {
        result.region = BlenderRegion::ViewportHeader;
        result.hit = true;
        return result;
    }

    if (Contains(m_Layout.viewport, input.mouseX, input.mouseY)) {
        result.region = BlenderRegion::Viewport;
        result.hit = true;
        return result;
    }

    if (Contains(m_Layout.properties, input.mouseX, input.mouseY)) {
        result.region = BlenderRegion::Properties;
        result.hit = true;
        return result;
    }

    if (Contains(m_Layout.statusBar, input.mouseX, input.mouseY)) {
        result.region = BlenderRegion::StatusBar;
        result.hit = true;
        return result;
    }

    return result;
}

void BlenderUI::OnCommand(const BlenderCommandEvent& event)
{
    switch (event.command) {
    case BlenderCommand::ToggleWireframe:
        m_State.viewport.wireframeShading = !m_State.viewport.wireframeShading;
        break;
    case BlenderCommand::ToggleGrid:
        m_State.viewport.showGrid = !m_State.viewport.showGrid;
        break;
    case BlenderCommand::SetWorkspaceLayout:
        m_State.activeWorkspace = BlenderWorkspace::Layout;
        ApplyWorkspaceDefaults();
        break;
    case BlenderCommand::SetWorkspaceModeling:
        m_State.activeWorkspace = BlenderWorkspace::Modeling;
        ApplyWorkspaceDefaults();
        break;
    case BlenderCommand::SetWorkspaceShading:
        m_State.activeWorkspace = BlenderWorkspace::Shading;
        ApplyWorkspaceDefaults();
        break;
    case BlenderCommand::SetWorkspaceRendering:
        m_State.activeWorkspace = BlenderWorkspace::Rendering;
        ApplyWorkspaceDefaults();
        break;
    default:
        break;
    }
}

void BlenderUI::ResetDefaultPanels()
{
    m_State.propertiesPanels = {
        {BlenderPanelKind::Scene, "Scene", true},
        {BlenderPanelKind::Object, "Object", true},
        {BlenderPanelKind::Material, "Material", true},
        {BlenderPanelKind::Render, "Render", true},
        {BlenderPanelKind::View, "View", true},
    };
}

void BlenderUI::ApplyWorkspaceDefaults()
{
    switch (m_State.activeWorkspace) {
    case BlenderWorkspace::Layout:
        m_State.viewport.showOverlays = true;
        m_State.viewport.wireframeShading = false;
        break;
    case BlenderWorkspace::Modeling:
        m_State.viewport.showOverlays = true;
        m_State.viewport.wireframeShading = true;
        break;
    case BlenderWorkspace::Shading:
        m_State.viewport.showOverlays = false;
        m_State.viewport.wireframeShading = false;
        break;
    case BlenderWorkspace::Rendering:
        m_State.viewport.showStats = false;
        m_State.viewport.showOverlays = false;
        break;
    }
}

bool BlenderUI::Contains(const BlenderRect& rect, float x, float y)
{
    return x >= rect.x && x <= (rect.x + rect.width) && y >= rect.y && y <= (rect.y + rect.height);
}

void BlenderUI::AddFilledRect(BlenderDrawList& drawList, const BlenderRect& rect, const BlenderColor& color)
{
    drawList.commands.push_back({BlenderDrawCommandType::FillRect, rect, color, "", 1.0f, 13.0f});
}

void BlenderUI::AddStrokeRect(BlenderDrawList& drawList, const BlenderRect& rect, const BlenderColor& color,
                              float thickness)
{
    drawList.commands.push_back({BlenderDrawCommandType::StrokeRect, rect, color, "", thickness, 13.0f});
}

void BlenderUI::AddText(BlenderDrawList& drawList, float x, float y, const BlenderColor& color,
                        const std::string& text, float fontSize)
{
    drawList.commands.push_back({BlenderDrawCommandType::Text, {x, y, 0.0f, 0.0f}, color, text, 1.0f, fontSize});
}

} // namespace Metaphysics
