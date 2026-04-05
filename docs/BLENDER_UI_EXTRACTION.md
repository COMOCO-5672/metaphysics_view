# Blender UI extraction note

## Why the previous implementation was not the real Blender UI framework

The earlier work only copied Blender-like layout and styling ideas into `ImGuiLayer`.
It did not extract Blender's actual UI framework.

That distinction matters because Blender's real editor UI depends on a large internal stack:

- window manager and event system
- screen / area / region layout model
- operators and context dispatch
- RNA-driven property panels
- GPU drawing and text rendering infrastructure

That stack is not a small standalone library that can be copied into this repository.
It is deeply integrated into Blender itself and also carries GPL implications.

## What was added now

A new independent UI module was created under `src/ui/blender_ui/`:

- `BlenderUI.h`
- `BlenderUI.cpp`

This module is not tied to ImGui.
It defines a Blender-like editor UI framework for this project:

- workspace model
- region layout model
- properties panel model
- viewport state model
- command/event model
- draw command list
- hit testing
- reusable layout snapshot for a future renderer

## What this module is for

This is the correct starting point if the goal is:

- stop depending on ImGui for the final editor UI
- keep a Blender-like editor structure
- render the UI later with DirectX or OpenGL using project-owned drawing code

## What it does not do yet

It does not render text, buttons, tabs, icons, or panels on screen by itself yet.
It now produces renderer-agnostic draw commands and hit-test results, but it still needs a real
2D backend to display and interact on screen.

That next step still requires:

1. a 2D UI renderer
2. text and font atlas rendering
3. hit testing and input routing
4. widget drawing for panels, tabs, labels, buttons, and property rows
5. integration into `Application` as the active editor UI

## Suggested next implementation order

1. Create a backend-agnostic 2D UI draw list
2. Add DirectX/OpenGL primitive rendering for rectangles, borders, and text
3. Add input hit testing using the layout rectangles from `BlenderUI`
4. Replace `ImGuiLayer` as the active UI path once basic panels are interactive
