cmake_minimum_required(VERSION 3.20)

# Test executable
add_executable(test_autonomous
    test_autonomous.cpp
)

# Include directories
target_include_directories(test_autonomous PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/third_party
)

# Link against the libraries
target_link_libraries(test_autonomous PRIVATE
    phantom_core
    phantom_rendering_core
    phantom_utils
)

# Set output directory
set_target_properties(test_autonomous PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Copy test to build directory for easy execution
add_custom_command(TARGET test_autonomous POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Test executable built: ${CMAKE_BINARY_DIR}/bin/test_autonomous"
)

# Add test to CTest
enable_testing()
add_test(NAME AutonomousTests COMMAND test_autonomous)
