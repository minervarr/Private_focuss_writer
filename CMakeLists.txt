cmake_minimum_required(VERSION 3.20)
project(PhantomWriter VERSION 0.1.0 LANGUAGES C CXX)

# ============================================================================
# Configuración global
# ============================================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Directorio de salida
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ============================================================================
# Detección de plataforma
# ============================================================================

if(WIN32)
    set(PHANTOM_PLATFORM "Windows")
    set(PHANTOM_PLATFORM_DIR "windows")
elseif(ANDROID)
    set(PHANTOM_PLATFORM "Android")
    set(PHANTOM_PLATFORM_DIR "android")
elseif(UNIX AND NOT APPLE)
    set(PHANTOM_PLATFORM "Linux")
    set(PHANTOM_PLATFORM_DIR "linux")
else()
    message(FATAL_ERROR "Plataforma no soportada")
endif()

message(STATUS "Building for: ${PHANTOM_PLATFORM}")

# ============================================================================
# Configuraciones Debug/Release
# ============================================================================

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Flags de compilación
if(CMAKE_BUILD_TYPE MATCHES Debug)
    message(STATUS "=== BUILD MODE: DEBUG ===")
    add_compile_definitions(DEBUG_BUILD)

    if(MSVC)
        add_compile_options(/W4 /Zi /Od)
    else()
        add_compile_options(-Wall -Wextra -g -O0)
    endif()

elseif(CMAKE_BUILD_TYPE MATCHES Release)
    message(STATUS "=== BUILD MODE: RELEASE ===")
    add_compile_definitions(NDEBUG RELEASE_BUILD)

    if(MSVC)
        add_compile_options(/O2 /GL)
        add_link_options(/LTCG)
    else()
        add_compile_options(-O3 -flto)
    endif()
endif()

# ============================================================================
# Dependencias
# ============================================================================

# Vulkan
find_package(Vulkan REQUIRED)
message(STATUS "Vulkan found: ${Vulkan_VERSION}")

# Find shader compiler
if(Vulkan_GLSLC_EXECUTABLE)
    set(GLSL_COMPILER ${Vulkan_GLSLC_EXECUTABLE})
    message(STATUS "Using glslc for shader compilation")
elseif(Vulkan_GLSLANG_VALIDATOR_EXECUTABLE)
    set(GLSL_COMPILER ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE})
    set(GLSL_COMPILER_ARGS "-V")
    message(STATUS "Using glslangValidator for shader compilation")
else()
    message(FATAL_ERROR "No Vulkan shader compiler found (glslc or glslangValidator)")
endif()

# ============================================================================
# Shader Compilation
# ============================================================================

set(SHADER_SOURCE_DIR ${CMAKE_SOURCE_DIR}/shaders)
set(SHADER_BINARY_DIR ${CMAKE_BINARY_DIR}/shaders)

# Create shader output directory
file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})

# Shader source files
set(VERTEX_SHADER ${SHADER_SOURCE_DIR}/text.vert)
set(FRAGMENT_SHADER ${SHADER_SOURCE_DIR}/text.frag)

# Shader output files
set(VERTEX_SHADER_SPV ${SHADER_BINARY_DIR}/text_vert.spv)
set(FRAGMENT_SHADER_SPV ${SHADER_BINARY_DIR}/text_frag.spv)

# Custom command to compile vertex shader
add_custom_command(
    OUTPUT ${VERTEX_SHADER_SPV}
    COMMAND ${GLSL_COMPILER} ${GLSL_COMPILER_ARGS} ${VERTEX_SHADER} -o ${VERTEX_SHADER_SPV}
    DEPENDS ${VERTEX_SHADER}
    COMMENT "Compiling vertex shader: text.vert"
    VERBATIM
)

# Custom command to compile fragment shader
add_custom_command(
    OUTPUT ${FRAGMENT_SHADER_SPV}
    COMMAND ${GLSL_COMPILER} ${GLSL_COMPILER_ARGS} ${FRAGMENT_SHADER} -o ${FRAGMENT_SHADER_SPV}
    DEPENDS ${FRAGMENT_SHADER}
    COMMENT "Compiling fragment shader: text.frag"
    VERBATIM
)

# Custom target that depends on all compiled shaders
add_custom_target(compile_shaders ALL
    DEPENDS ${VERTEX_SHADER_SPV} ${FRAGMENT_SHADER_SPV}
    COMMENT "Compiling all shaders"
)

message(STATUS "Shader compilation configured")

# ============================================================================
# Subdirectorios
# ============================================================================

add_subdirectory(src/utils)
add_subdirectory(src/core)
add_subdirectory(src/rendering/core)
add_subdirectory(src/rendering/vulkan)
add_subdirectory(src/persistence)
add_subdirectory(src/ui)
add_subdirectory(src/platform/${PHANTOM_PLATFORM_DIR})

# ============================================================================
# Ejecutable principal
# ============================================================================

add_executable(phantom-writer
    src/main.cpp
)

target_include_directories(phantom-writer PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(phantom-writer PRIVATE
    phantom_utils
    phantom_core
    phantom_rendering_core
    phantom_rendering_vulkan
    phantom_persistence
    phantom_ui
    phantom_platform_${PHANTOM_PLATFORM_DIR}
    Vulkan::Vulkan
)

# Make executable depend on shader compilation
add_dependencies(phantom-writer compile_shaders)

# ============================================================================
# Asset Deployment
# ============================================================================

# Copy shaders to binary directory
add_custom_command(TARGET phantom-writer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${VERTEX_SHADER_SPV}
        ${CMAKE_BINARY_DIR}/bin/shaders/text_vert.spv
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${FRAGMENT_SHADER_SPV}
        ${CMAKE_BINARY_DIR}/bin/shaders/text_frag.spv
    COMMENT "Copying compiled shaders to binary directory"
)

# Copy fonts to binary directory
add_custom_command(TARGET phantom-writer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/assets/fonts
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets/fonts
        ${CMAKE_BINARY_DIR}/bin/assets/fonts
    COMMENT "Copying fonts to binary directory"
)

message(STATUS "Asset deployment configured")

# ============================================================================
# Instalación
# ============================================================================

install(TARGETS phantom-writer DESTINATION bin)
install(DIRECTORY assets/ DESTINATION share/phantom-writer)
