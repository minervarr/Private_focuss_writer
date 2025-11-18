add_library(phantom_platform_windows STATIC
    window_win32.cpp
    input_win32.cpp
    file_system_windows.cpp
    platform_windows.cpp
)

target_include_directories(phantom_platform_windows PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(phantom_platform_windows PUBLIC
    phantom_utils
    Vulkan::Vulkan
)

# Windows libraries
target_link_libraries(phantom_platform_windows PRIVATE
    user32
    gdi32
    shell32
)
