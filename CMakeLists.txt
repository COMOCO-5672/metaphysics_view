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

target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL glfw)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
