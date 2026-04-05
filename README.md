# metaphysics_view

基于 OpenGL 的 3D 渲染器（C++17）：模型加载（Assimp）、场景与材质、ImGui 界面。详细功能与架构见 [PROJECT_SUMMARY.md](PROJECT_SUMMARY.md)、[ARCHITECTURE.md](ARCHITECTURE.md)。

## 依赖说明

| 组件 | 说明 |
|------|------|
| **CMake** | ≥ 3.12 |
| **C++ 编译器** | Windows：Visual Studio 2019+ 或 Build Tools（含 MSVC）；也可用 MinGW 等（需自行用对应 CMake 生成器） |
| **OpenGL** | 系统驱动即可 |
| **GLFW / GLM / GLEW** | 仓库内已带：`framework/ui/glfw3`、`framework/ui/glm`、`framework/glew-2.2.0`；若本机已安装，CMake 会优先用系统包 |
| **Assimp** | 源码在 `metaphysics_deps/assimp`，由 CMake 一并编译（首次较慢） |
| **imgui** | 源码在 `framework/ui/imgui` |

## Windows 上编译（推荐）

在仓库根目录打开 **PowerShell**：

```powershell
.\build.ps1
```

- **作用**：创建或使用 `build/` 目录，必要时执行 `cmake ..`，再执行 `cmake --build .`（会编 GLFW、Assimp 和主程序）。
- **完全清理重编**：`.\build.ps1 -Clean`

若出现「在此系统上禁止运行脚本」，是 PowerShell **执行策略** 限制。可执行：`Set-ExecutionPolicy -Scope CurrentUser RemoteSigned`，或单次：`powershell -ExecutionPolicy Bypass -File .\build.ps1`。详见 [BUILD_GUIDE.md](BUILD_GUIDE.md)。

**可执行文件位置**（取决于 CMake 生成器）：

- Visual Studio 默认多为：`build\bin\Debug\metaphysics.exe` 或 `build\bin\Release\metaphysics.exe`
- 单配置生成器（如 Ninja）：常为 `build\bin\metaphysics.exe`

建议在**可执行文件所在目录**运行，以便加载同目录下的 `glew32.dll` 等：

```powershell
cd build\bin\Debug   # 若你的 exe 在 Debug 下
.\metaphysics.exe
```

## 手动 CMake（可选）

与脚本等价的大致步骤：

```powershell
mkdir build -Force; cd build
cmake ..
cmake --build .
```

指定生成器示例：`cmake -G "Visual Studio 17 2022" -A x64 ..` 或 `cmake -G "Ninja" ..`（需已安装对应工具）。

## 其他文档

- [QUICKSTART.md](QUICKSTART.md) — 运行与操作说明
- [metaphysics_deps/README.md](metaphysics_deps/README.md) — 第三方依赖目录说明
## VS Code Workflow

If you prefer VS Code over the full Visual Studio IDE, use the workspace tasks already included in `.vscode/`.

Recommended extensions:

- `ms-vscode.cpptools`
- `ms-vscode.cmake-tools`

Available VS Code tasks:

- `CMake: Configure (VSCode)`
- `CMake: Build (VSCode)`
- `Run: metaphysics (VSCode)`
- `CMake: Clean (VSCode)`

Notes:

- VS Code builds use `build_vscode/`
- If `ninja` is installed, the configure task prefers `Ninja`
- Otherwise it falls back to the system default CMake generator
- You still need a C++ toolchain, but you do not need the full Visual Studio IDE

Available debug profiles:

- `Debug metaphysics (MSVC)`
- `Debug metaphysics (Ninja/Single Config)`
