---
name: metaphysics-3d-dev
description: Work on the Metaphysics 3D renderer project. Use when Codex needs to modify or extend this repository's 3D model viewer/editor, including Blender-inspired UI decisions, ImGui layout changes, OpenGL or DirectX11 renderer work, model loading with Assimp, camera/scene interaction, or project-specific architecture and collaboration rules.
---

# Metaphysics 3D Dev

Use this skill when editing the `metaphysics_view` repository.

## Project intent

Treat the project as a lightweight 3D model viewer/editor with:

- Scene/entity management
- Camera navigation and picking
- Runtime material and render settings editing
- Blender-inspired editor workflow
- Dual renderer direction: `OpenGL` first, `DirectX11` on Windows

Do not treat it as a generic game engine unless the user explicitly asks for that shift.

## Read first

Read only the files needed for the task. Start from these anchors:

- App flow: `src/core/Application.h`, `src/core/Application.cpp`
- UI state and editor UI: `src/ui/imgui_layer/ImGuiLayer.h`, `src/ui/imgui_layer/ImGuiLayer.cpp`
- Renderer abstraction: `src/core/renderer/RenderAPI.h`, `src/core/renderer/RenderSettings.h`, `src/core/renderer/ISceneRenderer.h`
- OpenGL backend: `src/platform/opengl/OpenGLRenderer.h`, `src/platform/opengl/OpenGLRenderer.cpp`
- DirectX11 backend: `src/platform/directx/DirectX11Renderer.h`, `src/platform/directx/DirectX11Renderer.cpp`
- Scene/model domain: `src/core/scene/Scene.h`, `src/core/model/Model.h`, `src/core/material/Material.h`, `src/core/camera/Camera.h`
- Build wiring: `CMakeLists.txt`

## Working rules for this repo

- Preserve the separation between core domain code, renderer backends, and UI layer.
- Prefer extending `ISceneRenderer` contracts instead of adding backend-specific logic into `Application`.
- Keep `RenderSettings` and shared data structures renderer-agnostic.
- Avoid binding new UI behavior directly to OpenGL-only types or headers.
- If adding rendering features, keep feature parity in mind between `OpenGLRenderer` and `DirectX11Renderer`, or clearly mark the gap.
- Reuse the existing `AppState` flow for editor state unless there is a strong reason to split it.

## UI direction

The user dislikes default ImGui styling and prefers Blender-like editor UX. When changing UI:

- Favor Blender-style layout logic: central viewport, left hierarchy/outliner, right properties, top menu, bottom status/info.
- Favor dense, compact controls over oversized demo-style spacing.
- Use a neutral dark palette with clear active/hover hierarchy.
- Keep tooling panels practical and editor-like, not game HUD-like.
- Avoid flashy styling that weakens readability.
- If adding new panels, place them in the editor workflow and docking model instead of opening loose floating windows by default.

## Renderer direction

Support these modes:

- Default path: `OpenGL`
- Windows alternate path: `DirectX11`

Current backend selection is runtime-driven through `METAPHYSICS_RENDERER=dx11` on Windows. Preserve or improve that behavior rather than introducing scattered compile-time toggles unless the user asks for them.

When touching renderer code:

- Keep camera, picking, and scene traversal behavior consistent across backends.
- Keep CPU-side mesh data valid for picking and backend upload.
- Do not move mesh ownership into renderer-only structures unless lifetime handling is explicit.
- Be careful with viewport resize, swap chain recreation, and ImGui backend initialization order.

## Model and scene expectations

- Model import uses `Assimp`.
- Entity selection and deletion already exist.
- Picking is ray-based and should keep working after rendering changes.
- Material editing is live and should remain reflected in rendering.

If a change may break selection, transform editing, or model loading, verify those paths explicitly in code review and testing notes.

## Build and validation

Use the smallest realistic validation for the change:

- For UI-only edits, ensure the affected files compile conceptually and check backend init paths.
- For renderer edits, try to build with CMake if the environment allows it.
- If local build is blocked by machine-specific permissions or toolchain issues, say so clearly and list the exact blocker.

Known local risk in this repo: Visual Studio/MSBuild may fail with `Access denied` on generated `*.tlog` or `unsuccessfulbuild` files inside temporary build directories. Do not hide this. Report it as an environment blocker.

## Communication expectations

The user wants momentum and low-friction collaboration. When responding:

- Be direct and supportive.
- State assumptions after making them.
- Prefer concrete implementation over abstract proposals.
- When there are multiple viable paths with real tradeoffs, present a short option list and recommend one.
- When creating project docs, make them reusable by future agents, not just human-readable summaries.

## Good default tasks for this skill

- Refine Blender-like editor layout or styling
- Add or improve property panels and render settings
- Extend OpenGL and DirectX11 rendering behavior
- Keep feature parity between backends
- Improve model loading workflow
- Add project-specific docs that reduce future ambiguity

## Avoid

- Large architecture rewrites without preserving current project intent
- Introducing unrelated frameworks for UI or rendering unless requested
- Mixing renderer-specific state into core domain types without a clear abstraction
- Reverting unrelated local changes in the worktree
