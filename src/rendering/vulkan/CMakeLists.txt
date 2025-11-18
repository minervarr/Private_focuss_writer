add_library(phantom_rendering_vulkan STATIC
    vk_renderer.cpp
    vk_text_renderer.cpp
)

target_include_directories(phantom_rendering_vulkan PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(phantom_rendering_vulkan PUBLIC
    phantom_utils
    phantom_rendering_core
    Vulkan::Vulkan
)
