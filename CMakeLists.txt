cmake_minimum_required(VERSION 3.12)

project(metaphysics VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -mmacosx-version-min=10.15")
    set(PLATFORM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/macos/main.mm)
    enable_language(OBJCXX)
elseif(MSVC)
    set(PLATFORM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/windows/main.cpp)
else()
    set(PLATFORM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/windows/main.cpp)
endif()

# Core sources
set(CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Application.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Application.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/renderer/RenderAPI.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/renderer/RenderAPI.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/renderer/RenderSettings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/renderer/ISceneRenderer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/camera/Camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/camera/Camera.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/material/Material.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/material/Material.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/model/Model.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/model/Model.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/scene/Scene.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/scene/Scene.h
)

# OpenGL platform sources
set(OPENGL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/opengl/Shader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/opengl/Shader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/opengl/OpenGLRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/opengl/OpenGLRenderer.h
)

set(DIRECTX_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/directx/DirectX11Renderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/directx/DirectX11Renderer.h
)

# UI sources
set(UI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/imgui_layer/ImGuiLayer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/imgui_layer/ImGuiLayer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/blender_ui/BlenderUI.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/blender_ui/BlenderUI.h
)

set(SOURCES ${PLATFORM_SOURCES} ${CORE_SOURCES} ${UI_SOURCES})

set(IMGUI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/ui/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/ui/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/ui/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/ui/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/ui/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/ui/imgui/backends/imgui_impl_opengl3.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/ui/imgui/backends/imgui_impl_glfw.cpp
)

if(WIN32)
    list(APPEND IMGUI_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/framework/ui/imgui/backends/imgui_impl_dx11.cpp
    )
endif()
add_executable(${PROJECT_NAME} ${SOURCES} ${OPENGL_SOURCES} ${IMGUI_SOURCES})

if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE ${DIRECTX_SOURCES})
endif()

if(APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE TARGET_DARWIN)
elseif(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE TARGET_WINDOWS)
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8 /FS)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/ui/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/ui/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/ui/glfw3/include
)

if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Cocoa")
    if(CMAKE_GENERATOR MATCHES "Xcode")
        set_target_properties(${PROJECT_NAME} PROPERTIES 
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/macos/Info.plist
        )
    endif()
endif()

if(DEFINED ENV{glfw3_DIR_${ARCH_DIR}})
    set(glfw3_DIR $ENV{glfw3_DIR_${ARCH_DIR}})
endif()

find_package(OpenGL REQUIRED)

# 检查是否有系统安装的glfw3和GLEW，如果没有则使用本地版本
find_package(glfw3 QUIET)
find_package(GLEW QUIET)
find_package(glm CONFIG QUIET)

# 如果找不到glfw3，使用本地的GLFW
if(NOT glfw3_FOUND)
    message(STATUS "System glfw3 not found, using local GLFW from framework/ui/glfw3")
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/framework/ui/glfw3 ${CMAKE_BINARY_DIR}/glfw3)
    set(GLFW_LIBRARY glfw)
else()
    message(STATUS "Using system glfw3")
    set(GLFW_LIBRARY glfw)
endif()

# 如果找不到GLEW，查找本地目录或提供手动指定路径的选项
if(NOT GLEW_FOUND)
    message(STATUS "GLEW not found via find_package, trying local and common paths...")
    find_path(GLEW_INCLUDE_DIR GL/glew.h
        PATHS
        "${CMAKE_CURRENT_SOURCE_DIR}/framework/glew-2.2.0/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/framework/glew/include"
        "C:/Program Files/GLEW/include"
        "D:/libs/GLEW/include"
        ENV GLEW_DIR
        PATH_SUFFIXES include
    )
    find_library(GLEW_LIBRARY
        NAMES glew32 glew GLEW
        PATHS
        "${CMAKE_CURRENT_SOURCE_DIR}/framework/glew-2.2.0/lib/Release/x64"
        "${CMAKE_CURRENT_SOURCE_DIR}/framework/glew/lib/Release/x64"
        "C:/Program Files/GLEW/lib"
        "D:/libs/GLEW/lib"
        ENV GLEW_DIR
        PATH_SUFFIXES lib lib/Release/x64
    )
    if(GLEW_INCLUDE_DIR AND GLEW_LIBRARY)
        message(STATUS "Found GLEW: ${GLEW_LIBRARY}")
        add_library(GLEW::GLEW UNKNOWN IMPORTED)
        set_target_properties(GLEW::GLEW PROPERTIES
            IMPORTED_LOCATION "${GLEW_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${GLEW_INCLUDE_DIR}"
        )
        get_filename_component(GLEW_BIN_DIR "${GLEW_LIBRARY}" DIRECTORY)
        get_filename_component(GLEW_BIN_DIR "${GLEW_BIN_DIR}/../../../bin/Release/x64" ABSOLUTE)
        if(EXISTS "${GLEW_BIN_DIR}/glew32.dll")
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${GLEW_BIN_DIR}/glew32.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            )
        endif()
    else()
        message(FATAL_ERROR "GLEW not found! Please install GLEW or set GLEW_DIR.\n"
            "  vcpkg: vcpkg install glew:x64-windows\n"
            "  Manual: http://glew.sourceforge.net/")
    endif()
endif()

# GLM是header-only库，优先使用本地 framework/ui/glm 子模块
if(NOT glm_FOUND)
    message(STATUS "GLM not found via find_package, using local GLM from framework/ui/glm")
    find_path(GLM_INCLUDE_DIR glm/glm.hpp
        PATHS
        ${CMAKE_CURRENT_SOURCE_DIR}/framework/ui/glm
        ${CMAKE_CURRENT_SOURCE_DIR}/metaphysics_deps
        "C:/Program Files/glm"
        ENV GLM_DIR
        PATH_SUFFIXES include
    )
    if(GLM_INCLUDE_DIR)
        message(STATUS "Found GLM at: ${GLM_INCLUDE_DIR}")
        add_library(glm_headers INTERFACE)
        target_include_directories(glm_headers INTERFACE ${GLM_INCLUDE_DIR})
        add_library(glm::glm ALIAS glm_headers)
    else()
        message(FATAL_ERROR "GLM not found! Please install GLM or set GLM_DIR")
    endif()
endif()

# 使用本地的Assimp
set(ASSIMP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/metaphysics_deps/assimp)

# 禁用Assimp的一些选项以加快编译和避免错误
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)

add_subdirectory(${ASSIMP_DIR} ${CMAKE_BINARY_DIR}/assimp EXCLUDE_FROM_ALL)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    OpenGL::GL 
    ${GLFW_LIBRARY}
    assimp
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE d3d11 dxgi d3dcompiler)
endif()

# 链接GLEW（如果找到）
if(TARGET GLEW::GLEW)
    target_link_libraries(${PROJECT_NAME} PRIVATE GLEW::GLEW)
endif()

# 链接GLM（如果找到）
if(TARGET glm::glm)
    target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)
endif()

if(CMAKE_COMPILER_ID MATCHES "GNU")
    # 查找 g++ 可执行文件
    find_program(MINGW_GCC g++)
    if(MINGW_GCC)
        # 获取 g++ 的目录，即 MinGW 的 bin 目录
        get_filename_component(MINGW_BIN_DIR ${MINGW_GCC} DIRECTORY)

        # 指定要拷贝的 DLL 文件（windows下依赖这些库运行）
        set(DLL_FILES
            "${MINGW_BIN_DIR}/libgcc_s_seh-1.dll"
            "${MINGW_BIN_DIR}/libstdc++-6.dll"
            "${MINGW_BIN_DIR}/libwinpthread-1.dll"
            "${MINGW_BIN_DIR}/glfw.dll"
            "${MINGW_BIN_DIR}/glew3.dll"
        )

        # 安装可执行文件
        install(TARGETS your_executable_name DESTINATION bin)

        # 拷贝指定的 DLL 文件到安装目录
        foreach(DLL ${DLL_FILES})
            if(EXISTS ${DLL})  # 检查 DLL 是否存在
                install(FILES ${DLL} DESTINATION bin)
            else()
                message(WARNING "DLL file not found: ${DLL}")
            endif()
        endforeach()
    else()
        message(FATAL_ERROR "MinGW g++ not found.")
    endif()

endif()


set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
