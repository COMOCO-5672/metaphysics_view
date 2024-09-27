cmake_minimum_required(VERSION 3.10)
project(YourProjectName)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ogre源码路径
set(OGRE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/metaphysics_deps/ogre)

# 添加Ogre的CMake模块路径
set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  "${OGRE_SOURCE_DIR}/CMake"
  "${OGRE_SOURCE_DIR}/CMake/Utils"
  "${OGRE_SOURCE_DIR}/CMake/Packages"
)

# 包含Ogre的CMake配置
include(OgreConfigTargets)

# 设置Ogre的构建选项
set(OGRE_BUILD_COMPONENT_PAGING TRUE)
set(OGRE_BUILD_COMPONENT_TERRAIN TRUE)
set(OGRE_BUILD_COMPONENT_VOLUME TRUE)
set(OGRE_BUILD_PLUGIN_BSP TRUE)
set(OGRE_BUILD_PLUGIN_PCZ TRUE)
set(OGRE_BUILD_PLUGIN_PFX TRUE)
set(OGRE_BUILD_RENDERSYSTEM_GL TRUE)
set(OGRE_BUILD_RENDERSYSTEM_GL3PLUS TRUE)
set(OGRE_BUILD_RENDERSYSTEM_GLES2 TRUE)
set(OGRE_BUILD_SAMPLES FALSE)

# 添加Ogre子目录
add_subdirectory(${OGRE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/ogre)

# 你的项目源文件
set(SOURCE_FILES
    src/platform/windows/main.cpp
    # 添加其他源文件
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# 链接Ogre库
target_link_libraries(${PROJECT_NAME}
    OgreMain
    OgreBites
    OgreRTShaderSystem
    # 添加其他需要的Ogre组件
)

# 包含Ogre头文件
target_include_directories(${PROJECT_NAME} PRIVATE
    ${OGRE_SOURCE_DIR}/OgreMain/include
    ${OGRE_SOURCE_DIR}/Components/Bites/include
    ${OGRE_SOURCE_DIR}/Components/RTShaderSystem/include
    # 添加其他需要的Ogre头文件路径
)
