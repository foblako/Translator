#!/bin/bash
set -e

# Build the project if the build directory doesn't exist or isn't built
if [ ! -f "build/main" ]; then
    echo "Building project..."
    cmake -B build
    cmake --build build -j$(nproc)
fi

COMMAND=$1

if [ "$COMMAND" = "tests" ]; then
    echo "Running tests..."
    ./examples/run_tests.sh ./build/main
elif [ "$COMMAND" = "run" ]; then
    shift
    ./build/main "$@"
else
    exec "$@"
fi
