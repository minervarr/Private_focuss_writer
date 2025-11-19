#!/bin/bash
#
# Script de ejecución automática de tests
# Compila y ejecuta todos los tests sin requerir intervención manual
#

set -e

echo ""
echo "╔════════════════════════════════════════════════════════════════╗"
echo "║                                                                ║"
echo "║         PHANTOM WRITER - AUTONOMOUS TEST RUNNER               ║"
echo "║                                                                ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""

# Verificar que estamos en el directorio correcto
if [ ! -f "CMakeLists.txt" ]; then
    echo "ERROR: Debe ejecutar este script desde el directorio raíz del proyecto"
    exit 1
fi

echo ">>> Paso 1: Compilando tests..."
echo ""
./compile_tests_minimal.sh

if [ $? -ne 0 ]; then
    echo ""
    echo "ERROR: Falló la compilación"
    exit 1
fi

echo ""
echo ">>> Paso 2: Ejecutando tests autónomos..."
echo ""

# Ejecutar tests y capturar el código de salida
./build_minimal/test_autonomous
TEST_EXIT_CODE=$?

echo ""
echo "╔════════════════════════════════════════════════════════════════╗"
echo "║                      RESUMEN FINAL                             ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""

if [ $TEST_EXIT_CODE -eq 0 ]; then
    echo "✓ TODOS LOS TESTS PASARON"
    echo ""
    echo "El sistema está funcionando correctamente."
    echo "Todos los componentes core han sido validados."
    exit 0
else
    echo "✗ ALGUNOS TESTS FALLARON"
    echo ""
    echo "Revise los logs arriba para más detalles."
    echo "Nota: Fallos relacionados con Vulkan/shaders son esperables"
    echo "      si no tiene el Vulkan SDK instalado."
    exit $TEST_EXIT_CODE
fi
