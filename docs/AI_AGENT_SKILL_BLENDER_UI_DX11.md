# AI Agent Skill: Blender UI + DX11 Migration

## 1) Blender UI design analysis (for this project)

- Workspace-first layout: Blender puts the viewport at center and treats all other areas as contextual tools.
- Region segmentation: Top bar (global commands), left outliner, right properties, bottom status/timeline.
- Dense but readable controls: small paddings, compact headers, neutral dark palette, high-contrast active states.
- Mode-driven editing: content of right panel changes with selected object and current workflow.
- Non-blocking workflow: file loading, hierarchy, and material edits stay docked and can coexist.

## 2) Mapping to current project

- `Scene Hierarchy` => Blender Outliner.
- `Properties` + `Render Settings` => Blender Properties editor tabs.
- `Viewport` panel => main 3D work area (camera + interaction hints).
- `Status` => live selected object and screen stats.
- `Load Model` => non-modal import workflow.

## 3) DX11 migration target

- Keep existing data model (`Scene/Entity/Model/Material`) unchanged.
- Keep existing interaction model (WASDQE, mouse rotate, picking).
- Replace render backend per platform/runtime with same output:
  - solid rendering
  - wireframe/solid+wireframe
  - grid + axis
  - runtime light/material tuning via UI

## 4) Current implementation in this repo

- Added common renderer contracts:
  - `src/core/renderer/RenderSettings.h`
  - `src/core/renderer/ISceneRenderer.h`
- Added DX11 renderer:
  - `src/platform/directx/DirectX11Renderer.h/.cpp`
- Upgraded app flow to runtime backend selection:
  - set `METAPHYSICS_RENDERER=dx11` on Windows to use DirectX11
  - default remains OpenGL
- Upgraded ImGui layer to multi-backend + Blender-like styling/layout:
  - OpenGL path: `imgui_impl_opengl3`
  - DX11 path: `imgui_impl_dx11`

## 5) Next iterative tasks (recommended)

1. Add true “viewport texture panel” (render scene to offscreen target and display in ImGui viewport region).
2. Add property tabs matching Blender semantics (Object/Material/Scene/Render).
3. Persist workspace layout and panel visibility to config file.
4. Add command palette and search for fast object/action access.
