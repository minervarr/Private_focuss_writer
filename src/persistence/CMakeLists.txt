add_library(phantom_persistence STATIC
    swap_file.cpp
    autosave.cpp
)

target_include_directories(phantom_persistence PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(phantom_persistence PUBLIC
    phantom_utils
)
