# 🔧 快速修复指南

## 问题：Assimp编译错误（MinGW + Ninja）

你遇到的错误是因为Assimp在编译时启用了 `-Werror`（将警告视为错误）。

## ✅ 解决方案（已修复）

我已经在 `CMakeLists.txt` 中添加了以下配置：

```cmake
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
```

这会禁用Assimp的严格警告检查。

## 🚀 现在请执行以下步骤：

### 方案A：重新配置（推荐）

```powershell
# 1. 清除旧的CMake缓存
cd d:\github\metaphysics_view\build
Remove-Item CMakeCache.txt
Remove-Item -Recurse CMakeFiles

# 2. 重新配置
cmake ..

# 3. 构建（这会需要几分钟来编译Assimp）
cmake --build .
```

### 方案B：完全清理重建

```powershell
# 1. 删除整个build目录
cd d:\github\metaphysics_view
Remove-Item -Recurse -Force build

# 2. 重新创建并构建
mkdir build
cd build
cmake ..
cmake --build .
```

### 方案C：使用Visual Studio（如果你想避免MinGW问题）

```powershell
# 1. 清理
cd d:\github\metaphysics_view
Remove-Item -Recurse -Force build

# 2. 使用VS生成器
mkdir build
cd build
cmake -G "Visual Studio 17 2022" ..

# 3. 在VS中打开并构建
start metaphysics.sln
```

然后在Visual Studio中：
- 选择 Release 模式
- 右键点击 metaphysics 项目 → 生成

## ⚡ 使用自动化脚本（最简单）

```powershell
cd d:\github\metaphysics_view
.\build.ps1 -Clean
```

这个脚本会自动：
1. 清理旧的构建文件
2. 重新配置CMake
3. 构建整个项目

## 📊 预期构建时间

- **首次构建**：约5-10分钟（需要编译Assimp，大约250个文件）
- **后续构建**：几秒钟到1分钟

## ✅ 构建成功后

可执行文件在：
```
build\bin\metaphysics.exe
```

运行：
```powershell
cd build\bin
.\metaphysics.exe
```

## 🎯 测试加载模型

1. 运行程序
2. 按 `Ctrl+O`
3. 输入：`../../models/cube.obj`
4. 点击 Load

## ❓ 如果还有问题

### 问题1：仍然有编译错误

尝试禁用特定的Assimp导入器：

在 CMakeLists.txt 中添加（在 `add_subdirectory(${ASSIMP_DIR} ...)` 之前）：

```cmake
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_FBX_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)
```

这只会编译常用的格式，大大加快编译速度并减少错误。

### 问题2：找不到依赖库

确保安装了所有依赖：

```powershell
# 使用vcpkg
vcpkg install glfw3:x64-windows glew:x64-windows glm:x64-windows

# 或使用MSYS2
pacman -S mingw-w64-x86_64-glfw mingw-w64-x86_64-glew mingw-w64-x86_64-glm
```

### 问题3：想要更快的构建

使用多线程：

```powershell
cmake --build . -- -j8  # 使用8个CPU核心
```

## 📝 当前状态

✅ CMakeLists.txt 已更新，禁用了Assimp的严格警告
✅ 构建脚本已改进
✅ 文档已完善

现在重新配置并构建应该可以成功！
