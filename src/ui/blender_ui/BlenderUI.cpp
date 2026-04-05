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

    auto drawButton = [this, &drawList](const BlenderRect& rect, bool active, const std::string& label) {
        AddFilledRect(drawList, rect, active ? m_Theme.accentSoft : m_Theme.regionAltBackground);
        AddStrokeRect(drawList, rect, active ? m_Theme.accent : m_Theme.border);
        AddText(drawList, rect.x + 10.0f, rect.y + 7.0f,
                active ? m_Theme.textPrimary : m_Theme.textMuted, label);
    };

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
    AddText(drawList, 112.0f, 7.0f, m_Theme.textMuted, "File");
    AddText(drawList, 156.0f, 7.0f, m_Theme.textMuted, "Edit");
    AddText(drawList, 200.0f, 7.0f, m_Theme.textMuted, "Render");

    drawButton({12.0f, m_Layout.workspaceTabs.y + 3.0f, 66.0f, 24.0f},
               m_State.activeWorkspace == BlenderWorkspace::Layout, "Layout");
    drawButton({84.0f, m_Layout.workspaceTabs.y + 3.0f, 88.0f, 24.0f},
               m_State.activeWorkspace == BlenderWorkspace::Modeling, "Modeling");
    drawButton({178.0f, m_Layout.workspaceTabs.y + 3.0f, 80.0f, 24.0f},
               m_State.activeWorkspace == BlenderWorkspace::Shading, "Shading");
    drawButton({264.0f, m_Layout.workspaceTabs.y + 3.0f, 96.0f, 24.0f},
               m_State.activeWorkspace == BlenderWorkspace::Rendering, "Rendering");

    const float toolX = m_Layout.toolbar.x + 8.0f;
    const float toolW = m_Layout.toolbar.width - 16.0f;
    drawButton({toolX, m_Layout.toolbar.y + 10.0f, toolW, 40.0f}, false, "Open");
    drawButton({toolX, m_Layout.toolbar.y + 62.0f, toolW, 40.0f},
               m_State.viewport.wireframeShading, "Wire");
    drawButton({toolX, m_Layout.toolbar.y + 114.0f, toolW, 40.0f},
               m_State.viewport.showGrid, "Grid");
    drawButton({toolX, m_Layout.toolbar.y + 166.0f, toolW, 40.0f}, false, "Focus");

    AddText(drawList, m_Layout.outliner.x + 12.0f, m_Layout.outliner.y + 12.0f,
            m_Theme.textPrimary, "Outliner");
    AddFilledRect(drawList, {m_Layout.outliner.x + 8.0f, m_Layout.outliner.y + 32.0f,
                  m_Layout.outliner.width - 16.0f, 30.0f}, m_Theme.regionAltBackground);
    AddStrokeRect(drawList, {m_Layout.outliner.x + 8.0f, m_Layout.outliner.y + 32.0f,
                    m_Layout.outliner.width - 16.0f, 30.0f}, m_Theme.border);
    AddText(drawList, m_Layout.outliner.x + 12.0f, m_Layout.outliner.y + 36.0f,
            m_Theme.textMuted, model.sceneName.empty() ? "Scene" : model.sceneName);
    AddText(drawList, m_Layout.outliner.x + 12.0f, m_Layout.outliner.y + 58.0f,
            m_Theme.textMuted, "Entities: " + std::to_string(model.entityCount));
    if (!model.selectedEntityName.empty()) {
        AddFilledRect(drawList, {m_Layout.outliner.x + 8.0f, m_Layout.outliner.y + 74.0f,
                      m_Layout.outliner.width - 16.0f, 26.0f}, m_Theme.accentSoft);
        AddStrokeRect(drawList, {m_Layout.outliner.x + 8.0f, m_Layout.outliner.y + 74.0f,
                        m_Layout.outliner.width - 16.0f, 26.0f}, m_Theme.accent);
        AddText(drawList, m_Layout.outliner.x + 14.0f, m_Layout.outliner.y + 80.0f,
                m_Theme.textPrimary, model.selectedEntityName);
    }

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
    AddFilledRect(drawList, {m_Layout.viewport.x + 24.0f, m_Layout.viewport.y + 116.0f, 168.0f, 28.0f},
                  m_Theme.regionBackground);
    AddStrokeRect(drawList, {m_Layout.viewport.x + 24.0f, m_Layout.viewport.y + 116.0f, 168.0f, 28.0f},
                  m_Theme.border);
    AddText(drawList, m_Layout.viewport.x + 34.0f, m_Layout.viewport.y + 123.0f,
            m_Theme.textMuted, "Click Select  Drag Orbit");
    AddFilledRect(drawList, {m_Layout.viewport.x + 24.0f, m_Layout.viewport.y + 152.0f, 156.0f, 28.0f},
                  m_Theme.regionBackground);
    AddStrokeRect(drawList, {m_Layout.viewport.x + 24.0f, m_Layout.viewport.y + 152.0f, 156.0f, 28.0f},
                  m_Theme.border);
    AddText(drawList, m_Layout.viewport.x + 34.0f, m_Layout.viewport.y + 159.0f,
            m_Theme.textMuted, "Scroll Dolly");

    AddText(drawList, m_Layout.properties.x + 12.0f, m_Layout.properties.y + 12.0f,
            m_Theme.textPrimary, "Properties");
    drawButton({m_Layout.properties.x + 12.0f, m_Layout.properties.y + 34.0f, 62.0f, 24.0f}, true, "Model");
    drawButton({m_Layout.properties.x + 80.0f, m_Layout.properties.y + 34.0f, 78.0f, 24.0f}, false, "Material");
    drawButton({m_Layout.properties.x + 164.0f, m_Layout.properties.y + 34.0f, 68.0f, 24.0f}, false, "Render");

    AddFilledRect(drawList, {m_Layout.properties.x + 12.0f, m_Layout.properties.y + 74.0f,
                  m_Layout.properties.width - 24.0f, 96.0f}, m_Theme.regionAltBackground);
    AddStrokeRect(drawList, {m_Layout.properties.x + 12.0f, m_Layout.properties.y + 74.0f,
                    m_Layout.properties.width - 24.0f, 96.0f}, m_Theme.border);
    AddText(drawList, m_Layout.properties.x + 22.0f, m_Layout.properties.y + 84.0f,
            m_Theme.textPrimary, "Selection");
    AddText(drawList, m_Layout.properties.x + 22.0f, m_Layout.properties.y + 108.0f,
            m_Theme.textMuted, "Name");
    AddText(drawList, m_Layout.properties.x + 120.0f, m_Layout.properties.y + 108.0f,
            m_Theme.textPrimary, model.selectedEntityName.empty() ? "None" : model.selectedEntityName);
    AddText(drawList, m_Layout.properties.x + 22.0f, m_Layout.properties.y + 130.0f,
            m_Theme.textMuted, "Meshes");
    AddText(drawList, m_Layout.properties.x + 120.0f, m_Layout.properties.y + 130.0f,
            m_Theme.textPrimary, std::to_string(model.meshCount));
    AddText(drawList, m_Layout.properties.x + 22.0f, m_Layout.properties.y + 152.0f,
            m_Theme.textMuted, "Shading");
    AddText(drawList, m_Layout.properties.x + 120.0f, m_Layout.properties.y + 152.0f,
            m_Theme.textPrimary, m_State.viewport.wireframeShading ? "Wireframe" : "Solid");

    AddFilledRect(drawList, {m_Layout.properties.x + 12.0f, m_Layout.properties.y + 182.0f,
                  m_Layout.properties.width - 24.0f, 84.0f}, m_Theme.regionAltBackground);
    AddStrokeRect(drawList, {m_Layout.properties.x + 12.0f, m_Layout.properties.y + 182.0f,
                    m_Layout.properties.width - 24.0f, 84.0f}, m_Theme.border);
    AddText(drawList, m_Layout.properties.x + 22.0f, m_Layout.properties.y + 192.0f,
            m_Theme.textPrimary, "Viewport");
    AddText(drawList, m_Layout.properties.x + 22.0f, m_Layout.properties.y + 216.0f,
            m_Theme.textMuted, "Grid");
    AddText(drawList, m_Layout.properties.x + 120.0f, m_Layout.properties.y + 216.0f,
            m_Theme.textPrimary, model.renderSettings.showGrid ? "Enabled" : "Disabled");
    AddText(drawList, m_Layout.properties.x + 22.0f, m_Layout.properties.y + 238.0f,
            m_Theme.textMuted, "Mode");
    AddText(drawList, m_Layout.properties.x + 120.0f, m_Layout.properties.y + 238.0f,
            m_Theme.textPrimary, model.renderMode == RenderMode::Wireframe ? "Wireframe" : "Solid");

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
