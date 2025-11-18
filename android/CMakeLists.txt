# Android-specific CMakeLists.txt for Phantom Writer
cmake_minimum_required(VERSION 3.22.1)

# Set the project
project(PhantomWriterAndroid VERSION 0.1.0 LANGUAGES C CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
find_library(log-lib log)
find_library(android-lib android)

# Get the parent directory (project root)
get_filename_component(PROJECT_ROOT ".." ABSOLUTE)

# Include parent CMakeLists for source files
set(PHANTOM_PLATFORM "Android")
set(PHANTOM_PLATFORM_DIR "android")

# Add subdirectories from parent project
add_subdirectory(${PROJECT_ROOT}/src/utils ${CMAKE_CURRENT_BINARY_DIR}/utils)
add_subdirectory(${PROJECT_ROOT}/src/core ${CMAKE_CURRENT_BINARY_DIR}/core)
add_subdirectory(${PROJECT_ROOT}/src/rendering/core ${CMAKE_CURRENT_BINARY_DIR}/rendering/core)
add_subdirectory(${PROJECT_ROOT}/src/rendering/vulkan ${CMAKE_CURRENT_BINARY_DIR}/rendering/vulkan)
add_subdirectory(${PROJECT_ROOT}/src/persistence ${CMAKE_CURRENT_BINARY_DIR}/persistence)
add_subdirectory(${PROJECT_ROOT}/src/ui ${CMAKE_CURRENT_BINARY_DIR}/ui)
add_subdirectory(${PROJECT_ROOT}/src/platform/android ${CMAKE_CURRENT_BINARY_DIR}/platform/android)

# Find Vulkan
find_library(vulkan-lib vulkan REQUIRED)

# Add native_app_glue
add_library(native_app_glue STATIC
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
target_include_directories(native_app_glue PUBLIC
    ${ANDROID_NDK}/sources/android/native_app_glue)

# Create the shared library for Android
add_library(phantom-writer SHARED
    ${PROJECT_ROOT}/src/platform/android/android_main.cpp
)

target_include_directories(phantom-writer PRIVATE
    ${PROJECT_ROOT}/include
    ${PROJECT_ROOT}/src
    ${ANDROID_NDK}/sources/android/native_app_glue
)

target_link_libraries(phantom-writer PRIVATE
    phantom_utils
    phantom_core
    phantom_rendering_core
    phantom_rendering_vulkan
    phantom_persistence
    phantom_ui
    phantom_platform_android
    native_app_glue
    ${vulkan-lib}
    ${log-lib}
    ${android-lib}
)

# Copy assets to APK
file(COPY ${PROJECT_ROOT}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${PROJECT_ROOT}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
