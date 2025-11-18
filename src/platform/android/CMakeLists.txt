add_library(phantom_platform_android STATIC
    window_android.cpp
    input_android.cpp
    file_system_android.cpp
    platform_android.cpp
)

target_include_directories(phantom_platform_android PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(phantom_platform_android PUBLIC
    phantom_utils
    Vulkan::Vulkan
)

# Android-specific libraries
target_link_libraries(phantom_platform_android PRIVATE
    android
    log
    native_app_glue
)
