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

add_executable(${PROJECT_NAME} ${SOURCES} ${PLATFORM_SOURCES})

if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Cocoa")
    if(CMAKE_GENERATOR MATCHES "Xcode")
        set_target_properties(${PROJECT_NAME} PROPERTIES 
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/macos/Info.plist
        )
    endif()
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
