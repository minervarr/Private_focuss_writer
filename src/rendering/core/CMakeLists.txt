add_library(phantom_rendering_core STATIC
    font_loader.cpp
    glyph_fragmenter.cpp
)

target_include_directories(phantom_rendering_core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/third_party/stb
)

target_link_libraries(phantom_rendering_core PUBLIC
    phantom_utils
)
