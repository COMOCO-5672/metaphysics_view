cmake_minimum_required(VERSION 3.12)

project(metaphysics VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -stdlib=libc++ -mmacosx-version-min=10.15")
    set(PLATFORM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/macos/main.mm)
    enable_language(OBJCXX)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
    set(PLATFORM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/windows/main.cpp)
endif()

file(GLOB_RECURSE SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.mm"
)

set(IMGUI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/ui/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/ui/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/ui/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/ui/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/ui/imgui/backends/imgui_impl_opengl3.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/ui/imgui/backends/imgui_impl_glfw.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${PLATFORM_SOURCES} ${IMGUI_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/framework/ui/imgui)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/framework/ui/imgui/backends)

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
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL glfw GLEW::GLEW)

if(CMAKE_COMPILER_ID MATCHES "GNU")
    # 查找 g++ 可执行文件
    find_program(MINGW_GCC g++)
    if(MINGW_GCC)
        # 获取 g++ 的目录，即 MinGW 的 bin 目录
        get_filename_component(MINGW_BIN_DIR ${MINGW_GCC} DIRECTORY)

        # 指定要拷贝的 DLL 文件
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
