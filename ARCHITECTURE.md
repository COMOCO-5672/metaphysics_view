# Metaphysics 3D Renderer

一个清晰、可扩展的3D渲染器，采用API与UI分离的架构设计。

## 🏗️ 架构设计

```
src/
├── core/                   # 核心API层（与渲染API无关）
│   ├── Application.cpp/h   # 应用程序主类
│   ├── renderer/          
│   │   └── RenderAPI.h    # 渲染API抽象接口
│   ├── camera/            
│   │   └── Camera.cpp/h   # 相机系统
│   ├── material/          
│   │   └── Material.cpp/h # 材质系统
│   ├── model/             
│   │   └── Model.cpp/h    # 模型加载（Assimp）
│   └── scene/             
│       └── Scene.cpp/h    # 场景管理（实体、变换）
│
├── platform/              # 平台相关实现
│   ├── opengl/           # OpenGL实现
│   │   ├── Shader.cpp/h  # 着色器封装
│   │   └── OpenGLRenderer.cpp/h  # OpenGL渲染器
│   └── windows/          # Windows入口
│       └── main.cpp
│
└── ui/                   # UI层
    └── imgui_layer/      # ImGui UI实现
        └── ImGuiLayer.cpp/h
```

## ✨ 核心特性

### 已实现功能

1. **模型加载**
   - 支持多种3D格式（OBJ, FBX, GLTF, DAE等）
   - 使用Assimp库加载
   - 自动计算法线和包围盒

2. **场景管理**
   - 实体系统（Entity-Component模式）
   - Transform操作（平移、旋转、缩放）
   - 场景层级管理

3. **相机系统**
   - 自由相机控制
   - WASD移动，QE上下
   - 鼠标拖拽旋转视角
   - 滚轮缩放

4. **渲染系统**
   - Phong光照模型
   - 材质系统（环境光、漫反射、镜面反射）
   - 选中实体高亮（线框显示）

5. **交互功能**
   - 点击拾取3D物体（射线检测）
   - 实时调整材质颜色
   - 实体属性编辑

6. **ImGui UI**
   - 场景层级面板
   - 属性编辑器
   - 模型加载对话框
   - Docking支持

## 🎮 操作说明

### 相机控制
- **W/A/S/D**: 前后左右移动
- **Q/E**: 上下移动
- **鼠标左键拖拽**: 旋转视角
- **鼠标滚轮**: 缩放
- **左键单击**: 选择物体

### 快捷键
- **Ctrl+O**: 打开模型加载对话框
- **Delete**: 删除选中的实体
- **ESC**: 退出程序

### UI面板
- **Scene Hierarchy**: 显示场景中所有实体
- **Properties**: 编辑选中实体的属性（位置、旋转、缩放、材质等）
- **File → Load Model**: 加载3D模型

## 🔧 构建与运行

### 依赖库
- OpenGL 3.3+
- GLFW 3.x
- GLEW
- GLM
- Assimp
- ImGui（已包含在framework中）

### Windows构建（MinGW）

```bash
# 1. 创建构建目录
mkdir build
cd build

# 2. 配置CMake
cmake -G "MinGW Makefiles" ..

# 3. 编译
cmake --build .

# 4. 运行
./bin/metaphysics.exe
```

### Visual Studio构建

```bash
# 1. 创建构建目录
mkdir msvc-build
cd msvc-build

# 2. 配置CMake
cmake ..

# 3. 在Visual Studio中打开 metaphysics.sln
# 4. 编译并运行
```

## 📦 加载模型示例

1. 按 `Ctrl+O` 打开模型加载对话框
2. 输入模型路径，例如：
   - `models/cube.obj`
   - `D:/models/character.fbx`
   - `C:/projects/assets/scene.gltf`
3. 点击 "Load" 按钮
4. 模型将自动加载并显示在场景中

## 🏛️ 架构优势

### API与UI分离

```
Core API (平台无关)
    ↓
Platform Layer (OpenGL/DirectX)
    ↓
UI Layer (ImGui/Qt)
```

- **Core层**: 纯C++，不依赖任何渲染API
- **Platform层**: 实现具体的渲染API（当前是OpenGL，未来可扩展DirectX）
- **UI层**: 独立的UI实现（当前是ImGui，可替换为Qt）

### 扩展到DirectX

要添加DirectX支持，只需：

1. 创建 `src/platform/directx/` 目录
2. 实现 `DirectXRenderer` 类（继承/实现与 `OpenGLRenderer` 相同的接口）
3. 在 `Application` 中切换渲染器

**不需要修改任何Core层代码！**

### 扩展到Qt UI

要切换到Qt UI，只需：

1. 创建 `src/ui/qt_layer/` 目录
2. 实现Qt窗口和控件
3. 使用相同的 `Scene`、`Camera`、`Model` 等Core API

**不需要修改任何渲染代码！**

## 🎨 材质编辑

选中物体后，在Properties面板可以编辑：

- **Ambient**: 环境光颜色
- **Diffuse**: 漫反射颜色（物体主要颜色）
- **Specular**: 镜面反射颜色（高光颜色）
- **Shininess**: 光泽度（1-256）

实时预览效果！

## 🔍 拾取系统

点击3D视图中的物体即可选中，选中后：
- 显示橙色线框高亮
- Properties面板显示详细信息
- 可以编辑Transform和材质

拾取使用射线检测算法（Möller–Trumbore）：
- 将屏幕坐标转换为射线
- 遍历所有三角形
- 返回最近的相交物体

## 📝 代码示例

### 加载模型

```cpp
auto model = std::make_shared<Model>();
if (model->LoadFromFile("path/to/model.obj")) {
    auto entity = scene->CreateEntity("MyModel", model);
    entity->SetPosition(glm::vec3(0, 0, 0));
}
```

### 修改材质

```cpp
auto mesh = model->GetMeshes()[0];
mesh->material->SetDiffuse(glm::vec3(1.0f, 0.0f, 0.0f)); // 红色
mesh->material->SetShininess(128.0f); // 高光泽
```

### 相机控制

```cpp
camera->ProcessKeyboard(CameraMovement::FORWARD, deltaTime);
camera->ProcessMouseMovement(xoffset, yoffset);
camera->ProcessMouseScroll(yoffset);
```

## 🚀 未来扩展

已为以下功能预留架构：

- [ ] DirectX 11/12 渲染后端
- [ ] Qt UI层
- [ ] PBR材质
- [ ] 纹理支持
- [ ] 阴影系统
- [ ] 多光源
- [ ] 场景序列化/保存
- [ ] 骨骼动画
- [ ] 粒子系统

## 📄 License

MIT License

## 👨‍💻 作者

Metaphysics Team

---

**Enjoy 3D Rendering! 🎉**
