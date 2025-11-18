add_library(phantom_utils STATIC
    logger.cpp
)

target_include_directories(phantom_utils PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)
