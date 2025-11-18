add_library(phantom_core STATIC
    buffer.cpp
    cursor.cpp
    editor_state.cpp
)

target_include_directories(phantom_core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(phantom_core PUBLIC
    phantom_utils
    phantom_rendering_core
    phantom_persistence
)
