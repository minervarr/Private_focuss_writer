# Option to use Sokol+Vulkan instead of pure Win32
option(USE_SOKOL_VULKAN "Use Sokol with Vulkan backend for Windows" ON)

if(USE_SOKOL_VULKAN)
    message(STATUS "Windows: Using Sokol+Vulkan backend")

    add_library(phantom_platform_windows STATIC
        window_sokol_vulkan.cpp
        input_win32.cpp
        file_system_windows.cpp
        platform_windows.cpp
    )

    target_compile_definitions(phantom_platform_windows PUBLIC
        PHANTOM_USE_SOKOL_VULKAN
    )

    target_include_directories(phantom_platform_windows PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/third_party
    )

    target_link_libraries(phantom_platform_windows PUBLIC
        phantom_utils
        Vulkan::Vulkan
    )

    # Windows libraries (Sokol requires these)
    target_link_libraries(phantom_platform_windows PRIVATE
        user32
        gdi32
        shell32
        ole32
        uuid
    )

else()
    message(STATUS "Windows: Using pure Win32+Vulkan backend")

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

endif()
