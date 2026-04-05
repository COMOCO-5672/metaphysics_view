#pragma once

#include "../../core/camera/Camera.h"
#include "../../core/renderer/RenderSettings.h"
#include "../../core/scene/Scene.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace Metaphysics {

enum class BlenderWorkspace {
    Layout = 0,
    Modeling,
    Shading,
    Rendering
};

enum class BlenderRegion {
    TopBar = 0,
    WorkspaceTabs,
    Toolbar,
    Outliner,
    ViewportHeader,
    Viewport,
    Properties,
    StatusBar
};

enum class BlenderPanelKind {
    Scene = 0,
    Object,
    Material,
    Render,
    View
};

enum class BlenderCommand {
    None = 0,
    OpenModel,
    DeleteSelected,
    FocusSelection,
    ToggleWireframe,
    ToggleGrid,
    SetWorkspaceLayout,
    SetWorkspaceModeling,
    SetWorkspaceShading,
    SetWorkspaceRendering
};

struct BlenderColor {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
};

struct BlenderRect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

enum class BlenderDrawCommandType {
    FillRect = 0,
    StrokeRect,
    Text
};

struct BlenderDrawCommand {
    BlenderDrawCommandType type = BlenderDrawCommandType::FillRect;
    BlenderRect rect;
    BlenderColor color;
    std::string text;
    float thickness = 1.0f;
    float fontSize = 13.0f;
};

struct BlenderDrawList {
    std::vector<BlenderDrawCommand> commands;

    void Clear() { commands.clear(); }
};

struct BlenderInputState {
    float mouseX = 0.0f;
    float mouseY = 0.0f;
    bool leftPressed = false;
    bool leftReleased = false;
};

struct BlenderHitResult {
    BlenderRegion region = BlenderRegion::Viewport;
    BlenderCommand command = BlenderCommand::None;
    bool hit = false;
};

struct BlenderTheme {
    BlenderColor appBackground {0.113f, 0.118f, 0.125f, 1.0f};
    BlenderColor regionBackground {0.145f, 0.149f, 0.157f, 1.0f};
    BlenderColor regionAltBackground {0.173f, 0.180f, 0.192f, 1.0f};
    BlenderColor border {0.255f, 0.271f, 0.298f, 1.0f};
    BlenderColor textPrimary {0.870f, 0.886f, 0.902f, 1.0f};
    BlenderColor textMuted {0.620f, 0.647f, 0.682f, 1.0f};
    BlenderColor accent {0.369f, 0.561f, 0.925f, 1.0f};
    BlenderColor accentSoft {0.255f, 0.365f, 0.565f, 1.0f};
    float spacing = 8.0f;
    float titleBarHeight = 28.0f;
    float headerHeight = 30.0f;
    float statusBarHeight = 24.0f;
    float toolbarWidth = 56.0f;
    float outlinerWidth = 260.0f;
    float propertiesWidth = 320.0f;
};

struct BlenderPanel {
    BlenderPanelKind kind = BlenderPanelKind::Scene;
    std::string title;
    bool expanded = true;
};

struct BlenderViewportState {
    bool showGrid = true;
    bool showOverlays = true;
    bool showStats = true;
    bool usePerspective = true;
    bool wireframeShading = false;
};

struct BlenderUIState {
    BlenderWorkspace activeWorkspace = BlenderWorkspace::Layout;
    std::vector<BlenderPanel> propertiesPanels;
    BlenderViewportState viewport;
};

struct BlenderCommandEvent {
    BlenderCommand command = BlenderCommand::None;
};

struct BlenderLayoutSnapshot {
    BlenderRect topBar;
    BlenderRect workspaceTabs;
    BlenderRect toolbar;
    BlenderRect outliner;
    BlenderRect viewportHeader;
    BlenderRect viewport;
    BlenderRect properties;
    BlenderRect statusBar;
};

struct BlenderUIModel {
    std::string sceneName;
    std::string selectedEntityName;
    std::string modelPath;
    std::size_t entityCount = 0;
    std::size_t meshCount = 0;
    RenderMode renderMode = RenderMode::Solid;
    RenderSettings renderSettings;
};

class BlenderUI {
public:
    BlenderUI();

    void SetViewportSize(int width, int height);
    void SetTheme(const BlenderTheme& theme);

    BlenderUIState& GetState();
    const BlenderUIState& GetState() const;
    const BlenderTheme& GetTheme() const;
    const BlenderLayoutSnapshot& GetLayout() const;

    BlenderUIModel BuildModel(const std::shared_ptr<Scene>& scene,
                              const std::shared_ptr<Entity>& selectedEntity,
                              const RenderSettings& renderSettings) const;

    void RebuildLayout();
    void BuildDrawList(const BlenderUIModel& model, BlenderDrawList& drawList) const;
    BlenderHitResult HitTest(const BlenderInputState& input) const;
    void OnCommand(const BlenderCommandEvent& event);

private:
    static bool Contains(const BlenderRect& rect, float x, float y);
    static void AddFilledRect(BlenderDrawList& drawList, const BlenderRect& rect, const BlenderColor& color);
    static void AddStrokeRect(BlenderDrawList& drawList, const BlenderRect& rect, const BlenderColor& color,
                              float thickness = 1.0f);
    static void AddText(BlenderDrawList& drawList, float x, float y, const BlenderColor& color,
                        const std::string& text, float fontSize = 13.0f);

    void ResetDefaultPanels();
    void ApplyWorkspaceDefaults();

private:
    int m_ViewportWidth = 1280;
    int m_ViewportHeight = 720;
    BlenderTheme m_Theme;
    BlenderUIState m_State;
    BlenderLayoutSnapshot m_Layout;
};

} // namespace Metaphysics
