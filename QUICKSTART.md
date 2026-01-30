# Metaphysics 3D渲染器 - 快速开始指南

## 🎯 项目概述

这是一个完整的、架构清晰的3D渲染器项目，具有以下特点：

- ✅ **API与UI分离**：Core层完全独立于渲染API和UI框架
- ✅ **易于扩展**：可以轻松切换到DirectX或Qt
- ✅ **功能完整**：模型加载、场景管理、材质编辑、拾取系统
- ✅ **代码规范**：清晰的命名空间和模块划分

## 📂 项目结构

```
metaphysics_view/
├── src/
│   ├── core/              # 核心API（平台无关）
│   │   ├── Application.cpp/h
│   │   ├── renderer/      # 渲染抽象
│   │   ├── camera/        # 相机系统
│   │   ├── material/      # 材质系统
│   │   ├── model/         # 模型加载
│   │   └── scene/         # 场景管理
│   ├── platform/
│   │   ├── opengl/        # OpenGL实现
│   │   └── windows/       # 平台入口
│   └── ui/
│       └── imgui_layer/   # ImGui UI层
├── models/                # 测试模型
│   └── cube.obj
├── CMakeLists.txt
└── ARCHITECTURE.md        # 架构文档
```

## 🚀 快速开始

### 1. 构建项目

```bash
cd d:\github\metaphysics_view
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
cmake --build .
```

或使用已有的构建目录：

```bash
cd d:\github\metaphysics_view\build
cmake --build .
```

### 2. 运行程序

```bash
cd d:\github\metaphysics_view\build\bin
.\metaphysics.exe
```

### 3. 加载模型

启动后，按照以下步骤加载3D模型：

1. 按 `Ctrl+O` 或点击 `File → Load Model`
2. 输入模型路径（测试用）：
   ```
   ../../models/cube.obj
   ```
   或使用绝对路径：
   ```
   D:/github/metaphysics_view/models/cube.obj
   ```
3. 点击 "Load" 按钮
4. 模型加载完成！

### 4. 操作模型

**相机控制：**
- `W/A/S/D` - 前后左右移动
- `Q/E` - 上下移动
- 按住鼠标左键拖拽 - 旋转视角
- 滚轮 - 缩放

**选择和编辑：**
1. 左键单击模型可选中（显示橙色线框）
2. 在右侧 **Properties** 面板编辑：
   - Transform（位置、旋转、缩放）
   - Material（环境光、漫反射、镜面反射、光泽度）
3. 实时查看效果！

**场景管理：**
- 左侧 **Scene Hierarchy** 面板显示所有实体
- 右键实体可以删除或复制
- 按 `Delete` 键删除选中的实体

## 🎨 测试材质编辑

加载cube.obj后，尝试：

1. 选中立方体
2. 在Properties面板展开 "Model → Mesh 0 → Material"
3. 修改 **Diffuse** 颜色（主要颜色）：
   - 红色：R=1.0, G=0.0, B=0.0
   - 绿色：R=0.0, G=1.0, B=0.0
   - 蓝色：R=0.0, G=0.0, B=1.0
4. 调整 **Shininess** 滑块（1-256）查看光泽变化

## 📝 支持的模型格式

通过Assimp库，支持以下格式：

- ✅ **OBJ** - Wavefront Object（最常用）
- ✅ **FBX** - Autodesk FBX
- ✅ **GLTF/GLB** - GL Transmission Format
- ✅ **DAE** - COLLADA
- ✅ **3DS** - 3D Studio
- ✅ **BLEND** - Blender
- ✅ **PLY** - Polygon File Format
- 以及更多...

## 🔧 常见问题

### Q: 模型加载失败？

**A:** 检查以下几点：
1. 路径是否正确（使用正斜杠 `/` 或双反斜杠 `\\`）
2. 文件是否存在
3. 查看控制台错误信息

### Q: 看不到模型？

**A:** 可能的原因：
1. 模型太大或太小 - 调整Scale
2. 模型在相机后面 - 移动相机或调整Position
3. 材质颜色太暗 - 调整Diffuse颜色

### Q: 如何重置相机？

**A:** 重启程序，或手动调整相机参数：
- 初始位置：(5, 5, 5)
- 初始朝向：看向原点

## 🏗️ 扩展开发

### 添加DirectX支持

1. 创建 `src/platform/directx/` 目录
2. 实现 `DirectXRenderer.h/cpp`（接口与OpenGLRenderer相同）
3. 在 `Application.cpp` 中切换渲染器：

```cpp
// 将来可以这样切换
#ifdef USE_DIRECTX
    m_Renderer = std::make_unique<DirectXRenderer>();
#else
    m_Renderer = std::make_unique<OpenGLRenderer>();
#endif
```

### 切换到Qt UI

1. 创建 `src/ui/qt_layer/` 目录
2. 实现Qt主窗口和控件
3. 复用Core API（Scene, Camera, Model等）
4. 调用Renderer的渲染函数

**核心优势**：Core层代码完全不需要修改！

## 📚 代码示例

### 程序化创建场景

在 `Application::Init()` 中添加：

```cpp
// 创建多个立方体
for (int i = 0; i < 5; i++) {
    auto model = std::make_shared<Model>();
    if (model->LoadFromFile("models/cube.obj")) {
        auto entity = m_AppState.currentScene->CreateEntity(
            "Cube " + std::to_string(i), model
        );
        entity->SetPosition(glm::vec3(i * 3.0f, 0.0f, 0.0f));
        entity->SetScale(glm::vec3(0.5f));
        
        // 随机颜色
        auto mesh = model->GetMeshes()[0];
        float r = (float)rand() / RAND_MAX;
        float g = (float)rand() / RAND_MAX;
        float b = (float)rand() / RAND_MAX;
        mesh->material->SetDiffuse(glm::vec3(r, g, b));
    }
}
```

## 🎯 下一步

1. ✅ 熟悉基本操作
2. ✅ 加载自己的3D模型
3. ✅ 实验材质编辑
4. 🔲 添加更多光源
5. 🔲 实现纹理支持
6. 🔲 添加阴影
7. 🔲 实现DirectX渲染器
8. 🔲 创建Qt UI版本

## 💡 提示

- 所有修改实时生效，无需重新编译
- 使用Scene Hierarchy快速管理多个物体
- Properties面板支持精确数值输入
- 右键菜单提供快捷操作

## 📞 获取帮助

遇到问题？
1. 查看控制台输出
2. 阅读 `ARCHITECTURE.md` 了解架构
3. 检查代码注释

---

**开始创造你的3D世界吧！🌟**
