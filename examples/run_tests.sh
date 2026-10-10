#!/bin/bash

if [ -z "$1" ]; then
    echo "Usage: $0 <path_to_executable>"
    exit 1
fi

EXECUTABLE=$1
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"

echo "Translator: $EXECUTABLE"
echo "──────────────────────────────────────────"

PASSED=0
FAILED=0
TOTAL=0

echo "Positive examples (valid/):"
for file in "$SCRIPT_DIR/valid/"*.txt; do
    [ -e "$file" ] || continue
    TOTAL=$((TOTAL + 1))

    yes 1 | $EXECUTABLE "$file" > /dev/null 2>&1
    EXIT_CODE=$?

    if [ $EXIT_CODE -eq 0 ]; then
        echo "  PASS  valid/$(basename "$file")"
        PASSED=$((PASSED + 1))
    else
        echo "  FAIL  valid/$(basename "$file") (exit code $EXIT_CODE, expected 0)"
        FAILED=$((FAILED + 1))
    fi
done

echo ""
echo "Negative examples (invalid/):"
for file in "$SCRIPT_DIR/invalid/"*.txt; do
    [ -e "$file" ] || continue
    TOTAL=$((TOTAL + 1))

    yes 1 | $EXECUTABLE "$file" > /dev/null 2>&1
    EXIT_CODE=$?

    if [ $EXIT_CODE -ne 0 ]; then
        echo "  PASS  invalid/$(basename "$file")"
        PASSED=$((PASSED + 1))
    else
        echo "  FAIL  invalid/$(basename "$file") (exit code 0, expected non-zero)"
        FAILED=$((FAILED + 1))
    fi
done

echo "──────────────────────────────────────────"
echo "Result: $PASSED passed / $FAILED failed / $TOTAL total"

if [ $FAILED -eq 0 ]; then
    exit 0
else
    exit 1
fi
