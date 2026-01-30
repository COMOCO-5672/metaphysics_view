# 🔧 依赖库安装指南

## 📋 项目依赖

这个项目需要以下依赖库：

- ✅ **OpenGL** - Windows自带
- ✅ **GLFW** - 已包含在项目中 (`framework/ui/glfw3/`)
- ✅ **Assimp** - 已包含在项目中 (`metaphysics_deps/assimp/`)
- ✅ **ImGui** - 已包含在项目中 (`framework/ui/imgui/`)
- ❌ **GLEW** - 需要安装
- ❌ **GLM** - 需要安装

## 🚀 快速安装（推荐）

### 方法1：使用vcpkg（最简单）

```powershell
# 1. 如果还没有vcpkg，先安装它
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# 2. 安装GLEW和GLM
.\vcpkg install glew:x64-windows glm:x64-windows

# 3. 集成到系统
.\vcpkg integrate install

# 4. 回到项目目录，使用vcpkg的CMake工具链构建
cd d:\github\metaphysics_view\build
cmake -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE="<vcpkg路径>/scripts/buildsystems/vcpkg.cmake" ..
```

### 方法2：手动下载安装

#### 安装 GLEW

1. **下载 GLEW**
   - 访问：http://glew.sourceforge.net/
   - 下载 Windows 二进制版本（例如 glew-2.1.0-win32.zip）

2. **解压到合适位置**
   ```
   例如：C:\libs\glew-2.1.0\
   ```

3. **设置环境变量（可选）**
   ```powershell
   # 临时设置（仅当前会话）
   $env:GLEW_DIR = "C:\libs\glew-2.1.0"
   
   # 或永久设置（系统环境变量）
   [System.Environment]::SetEnvironmentVariable('GLEW_DIR', 'C:\libs\glew-2.1.0', 'User')
   ```

4. **在CMake中指定路径**
   ```powershell
   cmake -G "Visual Studio 17 2022" -DGLEW_DIR="C:/libs/glew-2.1.0" ..
   ```

#### 安装 GLM

1. **下载 GLM**
   - 访问：https://github.com/g-truc/glm/releases
   - 下载最新版本（例如 glm-0.9.9.8.zip）

2. **解压到合适位置**
   ```
   例如：C:\libs\glm\
   ```
   
   解压后应该有这样的结构：
   ```
   C:\libs\glm\
   └── glm\
       └── glm.hpp
   ```

3. **在CMake中指定路径**
   ```powershell
   cmake -G "Visual Studio 17 2022" -DGLM_DIR="C:/libs/glm" ..
   ```

### 方法3：使用MSYS2/MinGW（如果你用MinGW）

```bash
# 在MSYS2终端中
pacman -S mingw-w64-x86_64-glew mingw-w64-x86_64-glm
```

## 🎯 完整的构建流程

### 使用vcpkg（推荐）

```powershell
# 1. 安装依赖
vcpkg install glew:x64-windows glm:x64-windows

# 2. 清理旧的构建
cd d:\github\metaphysics_view
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue

# 3. 创建构建目录
mkdir build
cd build

# 4. 配置CMake（使用vcpkg工具链）
cmake -G "Visual Studio 17 2022" `
      -DCMAKE_TOOLCHAIN_FILE="C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake" `
      ..

# 5. 构建
cmake --build . --config Release

# 6. 运行
cd bin\Release
.\metaphysics.exe
```

### 手动安装依赖

```powershell
# 1. 下载并解压GLEW和GLM到 C:\libs\

# 2. 清理旧的构建
cd d:\github\metaphysics_view
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue

# 3. 创建构建目录
mkdir build
cd build

# 4. 配置CMake（指定库路径）
cmake -G "Visual Studio 17 2022" `
      -DGLEW_DIR="C:/libs/glew-2.1.0" `
      -DGLM_DIR="C:/libs/glm" `
      ..

# 5. 构建
cmake --build . --config Release
```

## 🔍 验证安装

运行CMake配置后，应该看到类似的输出：

```
-- Found OpenGL: opengl32
-- Using local GLFW from framework/ui/glfw3
-- Found GLEW: C:/libs/glew-2.1.0/lib/Release/x64/glew32.lib
-- Found GLM at: C:/libs/glm
```

## ❓ 常见问题

### Q1: CMake找不到GLEW

**A:** 有几种解决方案：

1. **使用vcpkg**（最简单）
   ```powershell
   vcpkg install glew:x64-windows
   vcpkg integrate install
   ```

2. **手动指定路径**
   ```powershell
   cmake -DGLEW_DIR="C:/path/to/glew" ..
   ```

3. **设置环境变量**
   ```powershell
   $env:GLEW_DIR = "C:/path/to/glew"
   ```

### Q2: CMake找不到GLM

**A:** GLM是header-only库，只需要头文件：

1. **使用vcpkg**
   ```powershell
   vcpkg install glm:x64-windows
   ```

2. **手动下载**
   - 从 GitHub 下载 GLM
   - 解压到任意目录
   - 设置 `GLM_DIR` 指向包含 `glm/glm.hpp` 的目录

### Q3: 已经安装但CMake还是找不到

**A:** 尝试完全清理重建：

```powershell
cd d:\github\metaphysics_view
Remove-Item -Recurse -Force build
mkdir build
cd build

# 使用vcpkg工具链
cmake -G "Visual Studio 17 2022" `
      -DCMAKE_TOOLCHAIN_FILE="<vcpkg路径>/scripts/buildsystems/vcpkg.cmake" `
      ..
```

### Q4: 使用Visual Studio还是MinGW？

**A:** 两者都可以，但有区别：

- **Visual Studio**（推荐）
  - 优点：可视化IDE，调试方便，与Windows集成好
  - 缺点：需要安装VS
  - 适合：日常开发、调试

- **MinGW**
  - 优点：轻量级，命令行构建
  - 缺点：某些Windows特性支持不如VS
  - 适合：自动化构建、CI/CD

### Q5: 运行时找不到DLL

**A:** 如果使用动态链接的GLEW：

1. **找到glew32.dll**（通常在 `glew安装目录/bin/Release/x64/`）

2. **复制到可执行文件目录**
   ```powershell
   copy "C:\libs\glew-2.1.0\bin\Release\x64\glew32.dll" "build\bin\Release\"
   ```

3. **或添加到PATH**
   ```powershell
   $env:PATH += ";C:\libs\glew-2.1.0\bin\Release\x64"
   ```

## 📦 推荐的目录结构

如果选择手动安装，建议这样组织：

```
C:\libs\
├── glew-2.1.0\
│   ├── include\
│   │   └── GL\
│   │       └── glew.h
│   ├── lib\
│   │   └── Release\
│   │       └── x64\
│   │           └── glew32.lib
│   └── bin\
│       └── Release\
│           └── x64\
│               └── glew32.dll
└── glm\
    └── glm\
        └── glm.hpp
```

然后设置环境变量：
```powershell
$env:GLEW_DIR = "C:\libs\glew-2.1.0"
$env:GLM_DIR = "C:\libs\glm"
```

## ✅ 总结

**最简单的方法（强烈推荐）：**

```powershell
# 1. 安装vcpkg（一次性）
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install

# 2. 安装依赖
.\vcpkg install glew:x64-windows glm:x64-windows

# 3. 构建项目
cd d:\github\metaphysics_view\build
cmake -G "Visual Studio 17 2022" `
      -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake" `
      ..
cmake --build . --config Release
```

完成后你就可以运行 `build\bin\Release\metaphysics.exe` 了！🎉
