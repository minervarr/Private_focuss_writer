add_library(phantom_ui STATIC
    revision_mode.cpp
    confirmation_dialog.cpp
)

target_include_directories(phantom_ui PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(phantom_ui PUBLIC
    phantom_utils
)
