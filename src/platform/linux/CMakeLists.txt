add_library(phantom_platform_linux STATIC
    window_x11.cpp
    input_x11.cpp
    file_system_linux.cpp
    platform_linux.cpp
)

target_include_directories(phantom_platform_linux PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(phantom_platform_linux PUBLIC
    phantom_utils
    Vulkan::Vulkan
)

# X11 libraries
find_package(X11 REQUIRED)
target_link_libraries(phantom_platform_linux PRIVATE
    ${X11_LIBRARIES}
)
target_include_directories(phantom_platform_linux PRIVATE
    ${X11_INCLUDE_DIR}
)
