# 构建指南

## 方法1：使用PowerShell脚本（推荐，自动配置）

```powershell
# 正常构建
.\build.ps1

# 清理并重新构建
.\build.ps1 -Clean
```

## 方法2：使用Visual Studio（如果你安装了VS）

### Step 1: 配置CMake生成Visual Studio项目

```powershell
cd build
cmake -G "Visual Studio 17 2022" ..
# 或者使用 VS 2019: cmake -G "Visual Studio 16 2019" ..
# 或者使用 VS 2017: cmake -G "Visual Studio 15 2017" ..
```

### Step 2: 打开解决方案

```powershell
# 在VS中打开
start metaphysics.sln
```

或直接在Visual Studio中打开 `build/metaphysics.sln`

### Step 3: 在VS中构建

- 选择 Release 或 Debug 配置
- 按 F7 或 点击 "生成" → "生成解决方案"

## 方法3：使用MinGW + Ninja（当前配置）

```powershell
cd build
cmake ..
cmake --build .
```

**注意：** 当前CMakeLists.txt已经配置为禁用Assimp的严格警告，所以MinGW编译应该可以正常工作。

## 方法4：使用MinGW Makefiles

```powershell
cd build
cmake -G "MinGW Makefiles" ..
cmake --build .
```

## 常见问题

### Q: 编译Assimp时出现警告错误？

**A:** 已在CMakeLists.txt中添加了 `ASSIMP_WARNINGS_AS_ERRORS OFF`，应该已解决。如果仍有问题，尝试：

```powershell
.\build.ps1 -Clean
```

### Q: 找不到依赖库（GLFW, GLEW, GLM）？

**A:** 确保这些库已通过vcpkg或其他方式安装，并且在系统PATH中可找到。

使用vcpkg安装：
```powershell
vcpkg install glfw3:x64-windows glew:x64-windows glm:x64-windows
```

### Q: 想使用Visual Studio而不是MinGW？

**A:** 删除build目录并使用方法2重新配置：

```powershell
Remove-Item -Recurse -Force build
mkdir build
cd build
cmake -G "Visual Studio 17 2022" ..
start metaphysics.sln
```

### Q: Assimp编译太慢？

**A:** 首次编译Assimp需要几分钟，这是正常的。后续编译会快很多。

使用多线程加速：
```powershell
cmake --build . -- -j8  # 使用8个线程
```

## 推荐的构建方式

如果你有Visual Studio：
1. 使用方法2（VS项目）- 最方便，可以调试

如果只有MinGW：
1. 使用方法1（PowerShell脚本）- 最简单
2. 或方法4（MinGW Makefiles）- 更稳定

## 构建输出

成功构建后，可执行文件位于：
```
build/bin/metaphysics.exe
```

运行：
```powershell
cd build/bin
.\metaphysics.exe
```

## 测试模型

项目自带一个测试立方体模型：
```
models/cube.obj
```

在程序中按 `Ctrl+O`，输入路径：
```
../../models/cube.obj
```

即可加载测试模型。
