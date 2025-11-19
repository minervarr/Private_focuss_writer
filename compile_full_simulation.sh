#!/bin/bash

# Script para compilar la simulación completa del sistema
# Compila todos los componentes necesarios sin Vulkan

set -e

echo "=========================================="
echo "Compilando Full System Simulation"
echo "=========================================="

# Crear directorio de salida
mkdir -p build_minimal

echo ""
echo ">>> Compilando utils/logger..."
g++ -c -std=c++17 -g -Wall -Wextra -DDEBUG_BUILD \
    -I./include \
    -I./src \
    -I./third_party \
    src/utils/logger.cpp \
    -o build_minimal/logger.o

echo ">>> Compilando core/buffer..."
g++ -c -std=c++17 -g -Wall -Wextra -DDEBUG_BUILD \
    -I./include \
    -I./src \
    src/core/buffer.cpp \
    -o build_minimal/buffer.o

echo ">>> Compilando core/cursor..."
g++ -c -std=c++17 -g -Wall -Wextra -DDEBUG_BUILD \
    -I./include \
    -I./src \
    src/core/cursor.cpp \
    -o build_minimal/cursor.o

echo ">>> Compilando core/editor_state..."
g++ -c -std=c++17 -g -Wall -Wextra -DDEBUG_BUILD \
    -I./include \
    -I./src \
    src/core/editor_state.cpp \
    -o build_minimal/editor_state.o

echo ">>> Compilando rendering/core/font_loader..."
g++ -c -std=c++17 -g -Wall -Wextra -DDEBUG_BUILD \
    -I./include \
    -I./src \
    -I./third_party \
    src/rendering/core/font_loader.cpp \
    -o build_minimal/font_loader.o

echo ">>> Compilando rendering/core/glyph_fragmenter..."
g++ -c -std=c++17 -g -Wall -Wextra -DDEBUG_BUILD \
    -I./include \
    -I./src \
    src/rendering/core/glyph_fragmenter.cpp \
    -o build_minimal/glyph_fragmenter.o

echo ">>> Compilando rendering/core/opacity_manager..."
g++ -c -std=c++17 -g -Wall -Wextra -DDEBUG_BUILD \
    -I./include \
    -I./src \
    src/rendering/core/opacity_manager.cpp \
    -o build_minimal/opacity_manager.o

echo ">>> Compilando persistence/swap_file..."
g++ -c -std=c++17 -g -Wall -Wextra -DDEBUG_BUILD \
    -I./include \
    -I./src \
    src/persistence/swap_file.cpp \
    -o build_minimal/swap_file.o

echo ">>> Compilando persistence/autosave..."
g++ -c -std=c++17 -g -Wall -Wextra -DDEBUG_BUILD \
    -I./include \
    -I./src \
    src/persistence/autosave.cpp \
    -o build_minimal/autosave.o

echo ">>> Compilando ui/revision_mode..."
g++ -c -std=c++17 -g -Wall -Wextra -DDEBUG_BUILD \
    -I./include \
    -I./src \
    src/ui/revision_mode.cpp \
    -o build_minimal/revision_mode.o

echo ">>> Compilando ui/confirmation_dialog..."
g++ -c -std=c++17 -g -Wall -Wextra -DDEBUG_BUILD \
    -I./include \
    -I./src \
    src/ui/confirmation_dialog.cpp \
    -o build_minimal/confirmation_dialog.o

echo ""
echo ">>> Compilando test_full_simulation..."
g++ -c -std=c++17 -g -Wall -Wextra -DDEBUG_BUILD \
    -I./include \
    -I./src \
    -I./third_party \
    tests/test_full_simulation.cpp \
    -o build_minimal/test_full_simulation.o

echo ""
echo ">>> Enlazando ejecutable..."
g++ -o build_minimal/test_full_simulation \
    build_minimal/logger.o \
    build_minimal/buffer.o \
    build_minimal/cursor.o \
    build_minimal/editor_state.o \
    build_minimal/font_loader.o \
    build_minimal/glyph_fragmenter.o \
    build_minimal/opacity_manager.o \
    build_minimal/swap_file.o \
    build_minimal/autosave.o \
    build_minimal/revision_mode.o \
    build_minimal/confirmation_dialog.o \
    build_minimal/test_full_simulation.o \
    -lpthread

echo ""
echo "=========================================="
echo "✓ Compilación exitosa!"
echo "=========================================="
echo ""
echo "Ejecutable: ./build_minimal/test_full_simulation"
echo ""
