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

# ============================================================================
# Subdirectorios
# ============================================================================

add_subdirectory(src/utils)
add_subdirectory(src/core)
add_subdirectory(src/rendering/core)
add_subdirectory(src/rendering/vulkan)
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
    phantom_platform_${PHANTOM_PLATFORM_DIR}
    Vulkan::Vulkan
)

# ============================================================================
# Instalación
# ============================================================================

install(TARGETS phantom-writer DESTINATION bin)
install(DIRECTORY assets/ DESTINATION share/phantom-writer)
