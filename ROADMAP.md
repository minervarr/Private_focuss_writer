# Phantom Writer - Guía de Desarrollo

## Visión del Proyecto

Editor de texto anti-distracción con enfoque en confidencialidad y concentración absoluta. Desarrollado en C/C++ desde cero, sin frameworks de UI externos.

---

## Filosofía de Arquitectura

### Principio de Separación: Genérico vs Específico de Plataforma

**REGLA DE ORO**: Si un componente puede escribirse una vez y funcionar en todas las plataformas → es GENÉRICO. Si requiere APIs específicas del sistema operativo → es ESPECÍFICO DE PLATAFORMA.

### Categorización de Componentes

#### GENÉRICOS (src/core, src/utils, src/rendering/core)
- Lógica de buffer de texto (Rope o Gap Buffer)
- Sistema de cursor y navegación
- Algoritmos de fragmentación de glifos
- Sistema de opacidad (lógica matemática)
- Gestión de configuración (parsing TOML/JSON)
- Sistema de logging
- Estructuras de datos compartidas
- Máquina de estados del editor

#### ESPECÍFICOS DE PLATAFORMA (src/platform/*)
- Creación de ventanas (Win32, X11/Wayland, Android Activity)
- Input handling (teclado, mouse, touch)
- Contexto Vulkan y surface creation
- Sistema de archivos y paths
- Atajos de teclado (no disponibles en Android)
- Eventos del sistema operativo

---

## Stack Tecnológico

### Lenguaje
- **C++17** o superior
- C para interfaz con APIs de bajo nivel cuando sea necesario

### Gráficos
- **Vulkan 1.2+** para todas las plataformas
  - Windows: Vulkan via Vulkan SDK
  - Linux: Vulkan via Vulkan SDK
  - Android: Vulkan via NDK (API diferente, misma tecnología)

### Build System
- **CMake 3.20+** 
- Configuraciones separadas: Debug y Release
- Target por plataforma sin reconfiguración manual

### Dependencias Externas (Mínimas)
- **Vulkan SDK**: Obligatorio
- **STB libraries**: Para carga de fuentes (stb_truetype.h)
- **JSON parser**: nlohmann/json o RapidJSON (para config)
- Opcional: **Vulkan Memory Allocator (VMA)** para gestión de memoria GPU

---

## Estructura de Directorios Definitiva

```
phantom-writer/
├── CMakeLists.txt                 # Root CMake
├── README.md
├── LICENSE
├── .gitignore
│
├── src/
│   ├── main.cpp                   # Entry point genérico
│   │
│   ├── core/                      # GENÉRICO - Lógica central
│   │   ├── buffer.h/cpp          # Text buffer (Rope/Gap Buffer)
│   │   ├── cursor.h/cpp          # Cursor position and movement
│   │   ├── document.h/cpp        # Document abstraction
│   │   ├── editor_state.h/cpp    # Estado global del editor
│   │   └── CMakeLists.txt
│   │
│   ├── rendering/                 # MIXTO
│   │   ├── core/                 # GENÉRICO - Algoritmos
│   │   │   ├── glyph_fragmenter.h/cpp    # Algoritmo de fragmentación
│   │   │   ├── text_layout.h/cpp         # Layout de texto
│   │   │   ├── opacity_manager.h/cpp     # Lógica de opacidad
│   │   │   └── CMakeLists.txt
│   │   │
│   │   └── vulkan/               # ESPECÍFICO - Vulkan rendering
│   │       ├── vk_renderer.h/cpp         # Renderer base
│   │       ├── vk_pipeline.h/cpp         # Pipeline setup
│   │       ├── vk_text_renderer.h/cpp    # Text rendering
│   │       ├── vk_shader_loader.h/cpp    # Shader loading
│   │       └── CMakeLists.txt
│   │
│   ├── input/                     # MIXTO
│   │   ├── input_manager.h/cpp   # GENÉRICO - Interface abstracta
│   │   ├── keyboard_map.h/cpp    # GENÉRICO - Mapeo de teclas
│   │   └── CMakeLists.txt
│   │
│   ├── persistence/               # GENÉRICO
│   │   ├── autosave.h/cpp
│   │   ├── swap_file.h/cpp
│   │   ├── recovery.h/cpp
│   │   └── CMakeLists.txt
│   │
│   ├── ui/                        # GENÉRICO - Lógica de UI
│   │   ├── revision_mode.h/cpp
│   │   ├── confirmation_dialog.h/cpp
│   │   └── CMakeLists.txt
│   │
│   ├── config/                    # GENÉRICO
│   │   ├── settings.h/cpp
│   │   ├── config_parser.h/cpp
│   │   └── CMakeLists.txt
│   │
│   ├── utils/                     # GENÉRICO
│   │   ├── logger.h/cpp          # Sistema de logging
│   │   ├── timer.h/cpp           # Timing utilities
│   │   ├── memory_pool.h/cpp     # Memory management
│   │   └── CMakeLists.txt
│   │
│   └── platform/                  # ESPECÍFICO DE PLATAFORMA
│       ├── platform_interface.h   # Interface abstracta
│       │
│       ├── windows/
│       │   ├── window_win32.h/cpp        # Ventana Win32
│       │   ├── input_win32.h/cpp         # Input Win32
│       │   ├── vulkan_surface_win32.h/cpp
│       │   ├── file_system_win32.h/cpp
│       │   └── CMakeLists.txt
│       │
│       ├── linux/
│       │   ├── window_x11.h/cpp          # O window_wayland.h/cpp
│       │   ├── input_x11.h/cpp
│       │   ├── vulkan_surface_x11.h/cpp
│       │   ├── file_system_linux.h/cpp
│       │   └── CMakeLists.txt
│       │
│       └── android/
│           ├── android_native_app.h/cpp  # Android Native Activity
│           ├── input_android.h/cpp       # Touch + virtual keyboard
│           ├── vulkan_surface_android.h/cpp
│           ├── file_system_android.h/cpp
│           ├── android_main.cpp          # Entry point Android
│           ├── AndroidManifest.xml
│           └── CMakeLists.txt
│
├── include/
│   └── phantom_writer/            # Headers públicos
│       ├── types.h                # Tipos comunes
│       └── version.h
│
├── shaders/                       # Vulkan shaders
│   ├── text.vert                  # Vertex shader
│   ├── text.frag                  # Fragment shader (con fragmentación)
│   ├── opacity.frag               # Fragment shader para opacidad
│   ├── compile_shaders.sh         # Script de compilación
│   └── CMakeLists.txt
│
├── assets/
│   ├── fonts/
│   │   └── default_mono.ttf       # Fuente monoespaciada por defecto
│   └── config/
│       └── default_config.toml
│
├── tests/
│   ├── test_buffer.cpp
│   ├── test_glyph_fragmenter.cpp
│   └── CMakeLists.txt
│
└── build/                         # Directorio de build (gitignored)
    ├── debug/
    └── release/
```

---

## Sistema de Logging

### Niveles de Log (Debug vs Release)

```cpp
// utils/logger.h

enum class LogLevel {
    TRACE,    // Solo Debug - Detalles extremos
    DEBUG,    // Solo Debug - Info de desarrollo
    INFO,     // Ambos - Info general
    WARN,     // Ambos - Advertencias
    ERROR,    // Ambos - Errores
    FATAL     // Ambos - Errores críticos
};

// En Debug: todos los niveles activos
// En Release: solo INFO, WARN, ERROR, FATAL

#ifdef NDEBUG
    #define LOG_TRACE(...) ((void)0)
    #define LOG_DEBUG(...) ((void)0)
#else
    #define LOG_TRACE(...) Logger::log(LogLevel::TRACE, __FILE__, __LINE__, __VA_ARGS__)
    #define LOG_DEBUG(...) Logger::log(LogLevel::DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#endif

#define LOG_INFO(...)  Logger::log(LogLevel::INFO, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...)  Logger::log(LogLevel::WARN, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) Logger::log(LogLevel::ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_FATAL(...) Logger::log(LogLevel::FATAL, __FILE__, __LINE__, __VA_ARGS__)
```

### Categorías de Log para Debugging

```cpp
namespace LogCategory {
    constexpr const char* INIT = "INIT";
    constexpr const char* RENDER = "RENDER";
    constexpr const char* INPUT = "INPUT";
    constexpr const char* BUFFER = "BUFFER";
    constexpr const char* VULKAN = "VULKAN";
    constexpr const char* PLATFORM = "PLATFORM";
    constexpr const char* PERSISTENCE = "PERSISTENCE";
    constexpr const char* UI = "UI";
}

// Uso:
LOG_TRACE(LogCategory::VULKAN, "Creating Vulkan instance");
LOG_DEBUG(LogCategory::INPUT, "Key pressed: %d", keycode);
```

### Formato de Log

```
[TIMESTAMP][LEVEL][CATEGORY][FILE:LINE] Message
[2025-11-17 15:42:13.234][TRACE][VULKAN][vk_renderer.cpp:45] Creating Vulkan instance
[2025-11-17 15:42:13.456][DEBUG][INPUT][input_win32.cpp:102] Key pressed: 65 (A)
[2025-11-17 15:42:14.001][INFO][INIT][main.cpp:23] Phantom Writer started successfully
```

---

## Interfaz de Plataforma (Abstracción)

### platform_interface.h

```cpp
// src/platform/platform_interface.h

#ifndef PLATFORM_INTERFACE_H
#define PLATFORM_INTERFACE_H

#include <cstdint>
#include <string>
#include <functional>
#include <vulkan/vulkan.h>

namespace phantom {

// Forward declarations
struct WindowConfig;
struct InputEvent;

// ============================================================================
// WINDOW INTERFACE
// ============================================================================

class IPlatformWindow {
public:
    virtual ~IPlatformWindow() = default;
    
    // Lifecycle
    virtual bool create(const WindowConfig& config) = 0;
    virtual void destroy() = 0;
    virtual bool shouldClose() const = 0;
    
    // Event loop
    virtual void pollEvents() = 0;
    
    // Properties
    virtual void getFramebufferSize(int& width, int& height) const = 0;
    virtual bool isMinimized() const = 0;
    
    // Vulkan integration
    virtual VkSurfaceKHR createVulkanSurface(VkInstance instance) = 0;
    virtual const char** getRequiredVulkanExtensions(uint32_t& count) const = 0;
};

// ============================================================================
// INPUT INTERFACE
// ============================================================================

enum class KeyCode {
    Unknown = 0,
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    Space, Enter, Backspace, Delete, Tab,
    Left, Right, Up, Down,
    Home, End, PageUp, PageDown,
    Escape, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    LeftControl, RightControl,
    LeftShift, RightShift,
    LeftAlt, RightAlt,
    // ... más teclas
};

enum class MouseButton {
    Left,
    Right,
    Middle,
};

struct InputEvent {
    enum class Type {
        KeyPress,
        KeyRelease,
        KeyRepeat,
        MouseMove,
        MousePress,
        MouseRelease,
        MouseScroll,
        TouchDown,      // Android
        TouchUp,        // Android
        TouchMove,      // Android
        Character,      // Para text input
    } type;
    
    union {
        struct {
            KeyCode key;
            bool shift, ctrl, alt;
        } keyboard;
        
        struct {
            int x, y;
            int deltaX, deltaY;
        } mouse;
        
        struct {
            MouseButton button;
            int x, y;
        } mouseButton;
        
        struct {
            float deltaX, deltaY;
        } scroll;
        
        struct {
            int id;  // Touch pointer ID
            float x, y;
        } touch;
        
        struct {
            uint32_t codepoint;  // Unicode
        } character;
    } data;
};

class IPlatformInput {
public:
    virtual ~IPlatformInput() = default;
    
    // Callback registration
    using EventCallback = std::function<void(const InputEvent&)>;
    virtual void setEventCallback(EventCallback callback) = 0;
    
    // Polling (alternative to callbacks)
    virtual bool isKeyPressed(KeyCode key) const = 0;
    virtual void getMousePosition(int& x, int& y) const = 0;
};

// ============================================================================
// FILE SYSTEM INTERFACE
// ============================================================================

class IPlatformFileSystem {
public:
    virtual ~IPlatformFileSystem() = default;
    
    virtual std::string getConfigDirectory() const = 0;
    virtual std::string getDocumentsDirectory() const = 0;
    virtual std::string getTempDirectory() const = 0;
    
    virtual bool fileExists(const std::string& path) const = 0;
    virtual bool createDirectory(const std::string& path) const = 0;
    
    // Platform-specific separators
    virtual char getPathSeparator() const = 0;
};

// ============================================================================
// PLATFORM FACTORY
// ============================================================================

struct PlatformContext {
    IPlatformWindow* window = nullptr;
    IPlatformInput* input = nullptr;
    IPlatformFileSystem* fileSystem = nullptr;
    
    void cleanup() {
        delete window;
        delete input;
        delete fileSystem;
    }
};

// Factory function - implementada en cada plataforma
PlatformContext createPlatformContext();

} // namespace phantom

#endif // PLATFORM_INTERFACE_H
```

---

## CMake Configuration

### Root CMakeLists.txt

```cmake
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

# STB (header-only)
# Asumiendo que están en third_party/stb/
include_directories(${CMAKE_SOURCE_DIR}/third_party/stb)

# ============================================================================
# Subdirectorios
# ============================================================================

add_subdirectory(src/utils)
add_subdirectory(src/core)
add_subdirectory(src/rendering)
add_subdirectory(src/input)
add_subdirectory(src/persistence)
add_subdirectory(src/ui)
add_subdirectory(src/config)
add_subdirectory(src/platform/${PHANTOM_PLATFORM_DIR})
add_subdirectory(shaders)

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
    phantom_rendering
    phantom_input
    phantom_persistence
    phantom_ui
    phantom_config
    phantom_platform_${PHANTOM_PLATFORM_DIR}
    Vulkan::Vulkan
)

# ============================================================================
# Instalación
# ============================================================================

install(TARGETS phantom-writer DESTINATION bin)
install(DIRECTORY assets/ DESTINATION share/phantom-writer)
```

### Ejemplo: src/core/CMakeLists.txt

```cmake
add_library(phantom_core STATIC
    buffer.cpp
    cursor.cpp
    document.cpp
    editor_state.cpp
)

target_include_directories(phantom_core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(phantom_core PUBLIC
    phantom_utils
)
```

### Ejemplo: src/platform/windows/CMakeLists.txt

```cmake
add_library(phantom_platform_windows STATIC
    window_win32.cpp
    input_win32.cpp
    vulkan_surface_win32.cpp
    file_system_win32.cpp
)

target_include_directories(phantom_platform_windows PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(phantom_platform_windows PUBLIC
    phantom_utils
    Vulkan::Vulkan
)

# Win32 specific libraries
if(WIN32)
    target_link_libraries(phantom_platform_windows PRIVATE
        user32
        gdi32
    )
endif()
```

---

## Arquitectura de Renderizado con Vulkan

### Separación de Responsabilidades

```
┌─────────────────────────────────────────────────┐
│  GENÉRICO: rendering/core/                      │
│  - GlyphFragmenter: Algoritmo de fragmentación  │
│  - TextLayout: Posicionamiento de texto         │
│  - OpacityManager: Cálculo de opacidad          │
└───────────────┬─────────────────────────────────┘
                │ Interface abstracta
                ▼
┌─────────────────────────────────────────────────┐
│  ESPECÍFICO: rendering/vulkan/                  │
│  - VulkanRenderer: Inicialización Vulkan       │
│  - VulkanTextRenderer: Dibujado de texto       │
│  - VulkanPipeline: Configuración de pipelines  │
└───────────────┬─────────────────────────────────┘
                │ API de Vulkan
                ▼
┌─────────────────────────────────────────────────┐
│  ESPECÍFICO: platform/*/vulkan_surface_*.cpp    │
│  - Creación de VkSurfaceKHR por plataforma      │
│  - Windows: VK_KHR_win32_surface                │
│  - Linux: VK_KHR_xlib_surface / wayland         │
│  - Android: VK_KHR_android_surface              │
└─────────────────────────────────────────────────┘
```

### Flujo de Renderizado

```
1. [GENÉRICO] TextLayout calcula posiciones de glifos
2. [GENÉRICO] GlyphFragmenter decide qué mitad mostrar (superior/inferior)
3. [GENÉRICO] OpacityManager calcula alpha de líneas previas
4. [VULKAN] VulkanTextRenderer crea vertex buffers
5. [VULKAN] Shaders aplican fragmentación y opacidad en GPU
6. [PLATFORM] Presenta en la surface específica de plataforma
```

---

## Shaders de Vulkan

### text.vert (Vertex Shader)

```glsl
#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in uint inGlyphFragmentMode;  // 0=top, 1=bottom

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) flat out uint fragMode;

layout(push_constant) uniform PushConstants {
    mat4 projection;
    vec2 cursorPosition;
} pc;

void main() {
    gl_Position = pc.projection * vec4(inPosition, 0.0, 1.0);
    fragTexCoord = inTexCoord;
    fragMode = inGlyphFragmentMode;
}
```

### text.frag (Fragment Shader - con fragmentación)

```glsl
#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) flat in uint fragMode;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D glyphAtlas;

layout(push_constant) uniform PushConstants {
    mat4 projection;
    vec2 cursorPosition;
    float opacity;  // Para líneas previas
} pc;

void main() {
    vec2 adjustedTexCoord = fragTexCoord;
    
    // Fragmentación: mostrar solo mitad superior o inferior
    if (fragMode == 0) {
        // Mitad superior: Y de 0.0 a 0.5
        adjustedTexCoord.y *= 0.5;
    } else {
        // Mitad inferior: Y de 0.5 a 1.0
        adjustedTexCoord.y = 0.5 + adjustedTexCoord.y * 0.5;
    }
    
    float alpha = texture(glyphAtlas, adjustedTexCoord).r;
    
    // Aplicar opacidad de líneas previas
    alpha *= pc.opacity;
    
    outColor = vec4(1.0, 1.0, 1.0, alpha);
}
```

---

## Sistema de Input: Genérico + Específico

### Arquitectura

```
┌──────────────────────────────────────────┐
│ input/input_manager.h (GENÉRICO)        │
│ - Recibe InputEvent desde plataforma     │
│ - Traduce a acciones del editor          │
│ - No conoce Win32/X11/Android            │
└──────────────┬───────────────────────────┘
               │
      ┌────────┴────────┬────────────────┐
      ▼                 ▼                ▼
┌──────────┐      ┌──────────┐    ┌──────────┐
│ Win32    │      │ X11      │    │ Android  │
│ Keyboard │      │ Keyboard │    │ Touch    │
│ + Mouse  │      │ + Mouse  │    │ + VKB    │
└──────────┘      └──────────┘    └──────────┘
```

### Mapeo de Atajos de Teclado

**Windows/Linux**: Atajos estándar
```cpp
// input/keyboard_map.cpp (GENÉRICO)
struct KeyBinding {
    KeyCode key;
    bool ctrl;
    bool shift;
    bool alt;
    EditorAction action;
};

// Desktop bindings
std::vector<KeyBinding> desktopBindings = {
    {KeyCode::S, true, false, false, EditorAction::Save},
    {KeyCode::R, true, false, false, EditorAction::EnterRevisionMode},
    {KeyCode::Z, true, false, false, EditorAction::Undo},
    // ...
};
```

**Android**: Comandos desde menú o gestos
```cpp
// platform/android/input_android.cpp
// No hay Ctrl+S, pero podemos:
// - Botón flotante para guardar
// - Gesto de 3 dedos para modo revisión
// - Long press para menú contextual
```

---

## Implementación del Buffer de Texto

### Decisión: Rope Data Structure

**Ventajas**:
- O(log n) para inserción/eliminación
- Eficiente para documentos grandes
- Fácil de implementar undo/redo

```cpp
// core/buffer.h

#include <string>
#include <memory>

namespace phantom {

class TextBuffer {
public:
    TextBuffer();
    ~TextBuffer();
    
    // Operaciones básicas
    void insert(size_t position, const std::string& text);
    void erase(size_t position, size_t length);
    void clear();
    
    // Queries
    size_t length() const;
    std::string getText(size_t start, size_t length) const;
    std::string getLine(size_t lineNumber) const;
    size_t getLineCount() const;
    
    // Cursor utilities
    size_t lineStartPosition(size_t lineNumber) const;
    size_t lineEndPosition(size_t lineNumber) const;
    
private:
    struct RopeNode;
    std::unique_ptr<RopeNode> root_;
    
    void split(size_t position);
    void merge();
};

} // namespace phantom
```

---

## Sistema de Persistencia

### Estructura de Archivos

```
~/Documents/phantom-writer/
├── my-document.txt              # Archivo principal
├── .my-document.txt.swp         # Swap file (cambios no guardados)
└── .my-document.txt.meta        # Metadata (cursor pos, timestamps)
```

### Formato del Swap File

```
PHANTOM_SWAP_V1
timestamp: 1700000000
cursor_line: 42
cursor_column: 15
buffer_length: 1024
---BEGIN_CONTENT---
[contenido del buffer en UTF-8]
---END_CONTENT---
```

### Lógica de Recuperación

```
Al iniciar editor:
1. ¿Existe archivo principal?
   Sí → Continuar a 2
   No → Nuevo documento
   
2. ¿Existe .swp para este archivo?
   No → Cargar archivo principal normalmente
   Sí → Continuar a 3
   
3. Comparar timestamp .swp vs archivo principal
   .swp más nuevo → Posible crash, ofrecer recuperación
   .swp más viejo → Eliminar .swp obsoleto, cargar principal
```

---

## Plan de Implementación por Fases

### FASE 1A: Fundamentos (Semanas 1-2)

**Objetivo**: Ventana vacía + logging funcional

1. **Setup del proyecto**
   - [ ] Estructura de directorios
   - [ ] CMakeLists.txt configurado
   - [ ] Build exitoso para una plataforma (Windows o Linux)

2. **Sistema de logging**
   - [ ] `utils/logger.h/cpp` implementado
   - [ ] Macros LOG_* funcionando
   - [ ] Output a consola y archivo

3. **Ventana básica (una plataforma)**
   - [ ] `platform/windows/window_win32.cpp` O `platform/linux/window_x11.cpp`
   - [ ] Crear ventana negra
   - [ ] Event loop funcionando
   - [ ] Cerrar ventana correctamente

4. **Inicialización de Vulkan**
   - [ ] `rendering/vulkan/vk_renderer.cpp`
   - [ ] Crear VkInstance
   - [ ] Crear VkDevice y queues
   - [ ] Crear swapchain
   - [ ] Limpiar recursos correctamente

**Entregable**: Ventana negra con logs detallados de cada paso.

### FASE 1B: Renderizado Básico de Texto (Semanas 3-4)

**Objetivo**: Mostrar texto estático en pantalla

1. **Carga de fuentes**
   - [ ] Usar stb_truetype para cargar .ttf
   - [ ] Rasterizar glifos a texturas
   - [ ] Crear atlas de glifos en GPU

2. **Pipeline de Vulkan para texto**
   - [ ] Shaders básicos (sin fragmentación aún)
   - [ ] Vertex buffer para quads de texto
   - [ ] Uniform buffers para matrices

3. **Renderizado de texto simple**
   - [ ] Mostrar "Hello, Phantom!" en el centro
   - [ ] Verificar que se ve correctamente

**Entregable**: Ventana con texto estático centrado.

### FASE 1C: Input y Buffer (Semana 5)

**Objetivo**: Escribir texto en tiempo real

1. **Input handling**
   - [ ] `platform/*/input_*.cpp` implementado
   - [ ] Detectar keypresses
   - [ ] Callback a sistema genérico

2. **Buffer de texto**
   - [ ] `core/buffer.cpp` implementado (Rope o Gap Buffer simple)
   - [ ] Insertar caracteres
   - [ ] Backspace funcional

3. **Cursor**
   - [ ] `core/cursor.cpp` implementado
   - [ ] Posición del cursor
   - [ ] Movimiento con flechas

**Entregable**: Puedes escribir texto y borrarlo.

### FASE 1D: Fragmentación de Glifos (Semana 6)

**Objetivo**: Implementar la fragmentación aleatoria

1. **Algoritmo de fragmentación**
   - [ ] `rendering/core/glyph_fragmenter.cpp`
   - [ ] Determinar mitad (superior/inferior) por posición
   - [ ] Seed consistente

2. **Shaders de fragmentación**
   - [ ] Modificar `text.frag` para cortar glifos
   - [ ] Vertex attributes para modo (top/bottom)

3. **Integración**
   - [ ] VulkanTextRenderer usa fragmentación
   - [ ] Verificar que se ve "ilegible pero reconocible"

**Entregable**: Texto fragmentado visible mientras escribes.

### FASE 1E: Opacidad Dinámica (Semana 7)

**Objetivo**: Opacar líneas previas

1. **Opacity Manager**
   - [ ] `rendering/core/opacity_manager.cpp`
   - [ ] Timer de 1 segundo
   - [ ] Detección de idle vs typing

2. **Shader de opacidad**
   - [ ] Push constant para opacidad por línea
   - [ ] Interpolación suave

3. **Integración**
   - [ ] Líneas previas se opacan al escribir
   - [ ] Vuelven a 100% tras 1s

**Entregable**: Opacidad funcional.

### FASE 1F: Persistencia (Semana 8)

**Objetivo**: Guardar y recuperar

1. **Autosave**
   - [ ] `persistence/autosave.cpp`
   - [ ] Timer de 3 segundos
   - [ ] Thread separado para I/O

2. **Swap file**
   - [ ] `persistence/swap_file.cpp`
   - [ ] Escribir cambios incrementales
   - [ ] Recuperar tras crash

3. **Guardado manual**
   - [ ] Ctrl+S guarda explícitamente
   - [ ] Limpia .swp tras guardar exitoso

**Entregable**: No se pierde texto tras crash.

### FASE 1G: Modo Revisión (Semana 9)

**Objetivo**: Ver texto completo con confirmación

1. **Sistema de confirmación**
   - [ ] `ui/confirmation_dialog.cpp`
   - [ ] Doble confirmación
   - [ ] Input de texto "REVELAR"

2. **Modo revisión**
   - [ ] `ui/revision_mode.cpp`
   - [ ] Desactivar fragmentación
   - [ ] Desactivar opacidad
   - [ ] Indicador visual (borde rojo)

3. **Integración**
   - [ ] Ctrl+R activa modo
   - [ ] Esc sale del modo

**Entregable**: Modo revisión funcional con doble confirmación.

### FASE 1H: Multiplataforma (Semanas 10-11)

**Objetivo**: Compilar en Windows, Linux, Android

1. **Segunda plataforma (Linux/Windows)**
   - [ ] Implementar `platform/linux/*` o `platform/windows/*`
   - [ ] Verificar que CMake genera build correcto
   - [ ] Probar todas las funcionalidades

2. **Tercera plataforma (Android)**
   - [ ] `platform/android/*` implementado
   - [ ] Native Activity setup
   - [ ] Touch input adaptado
   - [ ] Menú para funciones sin atajos

**Entregable**: Phantom Writer funciona en 3 plataformas.

---

## Consideraciones Especiales: Android

### Diferencias Clave

1. **No hay atajos de teclado tradicionales**
   - Solución: Botones flotantes + menú deslizante
   - Long press para menú contextual
   - Gestos multi-touch para funciones especiales

2. **Entrada táctil + teclado virtual**
   - Touch para posicionar cursor
   - Virtual keyboard para texto
   - Gestos para navegación

3. **Ciclo de vida de aplicación**
   - Activity puede pausarse/destruirse en cualquier momento
   - Necesita guardar estado agresivamente
   - Recuperar estado al reanudar

4. **Permisos de almacenamiento**
   - Android 11+: Scoped Storage
   - Necesita permisos explícitos
   - Usar carpeta de documentos de la app

### Ejemplo de Menú Android

```
┌────────────────────────────┐
│  ☰ Menú                    │ <- Swipe desde borde izquierdo
│  ─────────────────────────  │
│  💾 Guardar                 │
│  👁️ Modo Revisión           │
│  ⚙️ Configuración           │
│  ℹ️ Acerca de               │
│  ❌ Cerrar                  │
└────────────────────────────┘

Gestos:
- 3 dedos tap: Modo revisión
- Pinch: Zoom (ajustar tamaño de fuente)
- Swipe 2 dedos arriba/abajo: Scroll rápido
```

---

## Testing Strategy

### Unit Tests (Google Test)

```cpp
// tests/test_buffer.cpp

#include <gtest/gtest.h>
#include "core/buffer.h"

TEST(TextBuffer, InsertText) {
    phantom::TextBuffer buffer;
    buffer.insert(0, "Hello");
    EXPECT_EQ(buffer.length(), 5);
    EXPECT_EQ(buffer.getText(0, 5), "Hello");
}

TEST(TextBuffer, EraseText) {
    phantom::TextBuffer buffer;
    buffer.insert(0, "Hello World");
    buffer.erase(5, 6);  // Borra " World"
    EXPECT_EQ(buffer.getText(0, 5), "Hello");
}

// ... más tests
```

### Integration Tests

```cpp
// tests/test_integration.cpp

TEST(Integration, TypeAndSave) {
    // 1. Crear editor
    // 2. Simular keypresses: "Hello"
    // 3. Trigger autosave
    // 4. Verificar archivo swap
    // 5. Crash simulado
    // 6. Recuperar
    // 7. Verificar texto: "Hello"
}
```

### Manual Testing Checklist

```
[ ] Escribir 1000 líneas de texto
[ ] Fragmentación visible y consistente
[ ] Opacidad funciona correctamente
[ ] Guardar y recuperar funciona
[ ] Crash recovery funciona
[ ] Modo revisión muestra todo correctamente
[ ] Atajos de teclado funcionan (desktop)
[ ] Touch input funciona (Android)
[ ] Performance: < 16ms por frame
[ ] Memoria: < 100MB para documento de 10MB
```

---

## Debugging Tips

### Logs Críticos a Implementar

```cpp
// Al iniciar
LOG_INFO(LogCategory::INIT, "=== Phantom Writer v%d.%d.%d ===", 
    VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
LOG_INFO(LogCategory::INIT, "Platform: %s", PLATFORM_NAME);
LOG_INFO(LogCategory::INIT, "Build: %s", BUILD_TYPE);

// Vulkan
LOG_DEBUG(LogCategory::VULKAN, "Creating Vulkan instance...");
LOG_DEBUG(LogCategory::VULKAN, "Available extensions: %d", extensionCount);
LOG_TRACE(LogCategory::VULKAN, "Extension[%d]: %s", i, extensions[i].extensionName);

// Input
LOG_TRACE(LogCategory::INPUT, "Key event: code=%d, action=%d", keycode, action);
LOG_TRACE(LogCategory::INPUT, "Mouse moved: (%d, %d)", x, y);

// Rendering
LOG_DEBUG(LogCategory::RENDER, "Frame %lu: %d draw calls", frameNumber, drawCallCount);
LOG_TRACE(LogCategory::RENDER, "Glyph '%c': fragmented as %s", ch, 
    fragmentMode == 0 ? "TOP" : "BOTTOM");

// Buffer
LOG_TRACE(LogCategory::BUFFER, "Insert '%c' at pos %zu", ch, position);
LOG_TRACE(LogCategory::BUFFER, "Erase %zu chars at pos %zu", length, position);

// Persistence
LOG_INFO(LogCategory::PERSISTENCE, "Autosaving to swap file...");
LOG_INFO(LogCategory::PERSISTENCE, "Swap file written: %zu bytes", bytesWritten);
LOG_WARN(LogCategory::PERSISTENCE, "Swap file detected - possible crash recovery");
```

### Validation Layers de Vulkan

```cpp
// En Debug, activar validation layers
#ifdef DEBUG_BUILD
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    
    // Setup debug callback
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    // ...
#endif
```

---

## Métricas de Éxito (Verificables en cada commit)

### Performance
- [ ] Input latency < 16ms (60 FPS mínimo)
- [ ] Frametime consistente: < 20ms en el 99% de frames
- [ ] Memoria: < 50MB para documento vacío
- [ ] Memoria: < 100MB para documento de 1MB
- [ ] Startup time: < 1 segundo

### Funcionalidad
- [ ] Zero pérdida de datos tras crash
- [ ] Fragmentación de glifos funcionando correctamente
- [ ] Opacidad se activa/desactiva según especificación
- [ ] Modo revisión requiere doble confirmación
- [ ] Autosave no bloquea el input

### Multiplataforma
- [ ] Build exitoso en Windows
- [ ] Build exitoso en Linux
- [ ] Build exitoso en Android
- [ ] Misma funcionalidad en las 3 plataformas (adaptada por input)

---

## Próximos Pasos Inmediatos

1. **Setup inicial del proyecto**
   ```bash
   mkdir phantom-writer && cd phantom-writer
   # Crear estructura de directorios
   # Crear CMakeLists.txt root
   ```

2. **Implementar logger primero**
   - Es la base para debugging de todo lo demás
   - `src/utils/logger.h` y `logger.cpp`

3. **Ventana básica en una plataforma**
   - Elegir Windows o Linux para empezar
   - Implementar `platform/windows/window_win32.cpp` o `platform/linux/window_x11.cpp`

4. **Vulkan "Hello Triangle"**
   - Inicializar Vulkan
   - Limpiar pantalla con un color
   - Verificar que el loop de renderizado funciona

5. **Iterar desde ahí**
   - Seguir el plan de FASE 1A → 1B → 1C → ...

---

## Recursos Recomendados

### Vulkan
- **Vulkan Tutorial**: https://vulkan-tutorial.com/
- **Vulkan Guide**: https://vkguide.dev/
- **Vulkan Samples**: https://github.com/KhronosGroup/Vulkan-Samples

### Text Rendering
- **stb_truetype.h**: https://github.com/nothings/stb/blob/master/stb_truetype.h
- **Valve's paper**: "Improved Alpha-Tested Magnification for Vector Textures and Special Effects"

### Platform APIs
- **Win32**: MSDN documentation
- **X11**: https://www.x.org/releases/current/doc/
- **Android NDK**: https://developer.android.com/ndk/guides

### Data Structures
- **Rope implementation**: https://github.com/component/rope
- **Gap Buffer**: Editor de GNU Emacs usa este approach

---

## Notas Finales

Este documento es tu **fuente de verdad** para el desarrollo de Phantom Writer. Consulta cada sección según avances en la implementación.

**Recuerda**:
- Prioriza logging desde el inicio
- Commits pequeños y frecuentes
- Testing continuo
- Documentación en el código

**Filosofía**: Genérico cuando sea posible, específico cuando sea necesario.

---

**Versión**: 1.0  
**Fecha**: 2025-11-17  
**Autor**: Arquitectura colaborativa  
**Estado**: Listo para implementación
