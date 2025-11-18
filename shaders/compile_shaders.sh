#!/bin/bash

# Phantom Writer - Shader Compilation Script

set -e

SHADER_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "Compiling shaders in $SHADER_DIR..."

# Compile text shaders
glslangValidator -V "$SHADER_DIR/text.vert" -o "$SHADER_DIR/text_vert.spv"
glslangValidator -V "$SHADER_DIR/text.frag" -o "$SHADER_DIR/text_frag.spv"

echo "Shaders compiled successfully!"
ls -lh "$SHADER_DIR"/*.spv
