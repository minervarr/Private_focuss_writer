#!/bin/bash

# Script para compilar tests sin dependencias de Vulkan
# Compila solo componentes core que no requieren GPU

set -e

echo "======================================"
echo "Compilando Tests Mínimos (Sin Vulkan)"
echo "======================================"

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

echo ""
echo ">>> Compilando test_autonomous..."
g++ -c -std=c++17 -g -Wall -Wextra -DDEBUG_BUILD \
    -I./include \
    -I./src \
    -I./third_party \
    tests/test_autonomous.cpp \
    -o build_minimal/test_autonomous.o

echo ""
echo ">>> Enlazando ejecutable..."
g++ -o build_minimal/test_autonomous \
    build_minimal/logger.o \
    build_minimal/buffer.o \
    build_minimal/cursor.o \
    build_minimal/font_loader.o \
    build_minimal/glyph_fragmenter.o \
    build_minimal/test_autonomous.o \
    -lpthread

echo ""
echo "======================================"
echo "✓ Compilación exitosa!"
echo "======================================"
echo ""
echo "Ejecutable: ./build_minimal/test_autonomous"
echo ""
