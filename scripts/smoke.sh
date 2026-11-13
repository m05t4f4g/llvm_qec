#!/usr/bin/env bash
# Smoke-test every test/*.mlir through the QIR backend + LLVM's parser.
# Usage: ./scripts/smoke.sh [build-dir]   (default: ./build)

set -u
BUILD="${1:-./build}"

if [[ ! -x "$BUILD/qec-opt" ]]; then
    echo "error: $BUILD/qec-opt not found — build first" >&2
    exit 1
fi
if ! command -v llvm-as-19 >/dev/null 2>&1; then
    echo "warning: llvm-as-19 not on PATH — skipping QIR validation" >&2
    HAVE_AS=0
else
    HAVE_AS=1
fi

fail=0
for f in test/*.mlir; do
    if ! "$BUILD/qec-opt" --qec-schedule "$f" >/dev/null 2>&1; then
        echo "FAIL   $f   (qec-opt rejected input)"
        fail=1
        continue
    fi

    if [[ $HAVE_AS -eq 1 ]]; then
        if "$BUILD/qec-opt" --qec-schedule "$f" 2>/dev/null \
           | "$BUILD/qec-to-qir" --qec-to-qir 2>/dev/null \
           | llvm-as-19 - -o /dev/null 2>/dev/null; then
            echo "OK     $f"
        else
            echo "FAIL   $f   (QIR not accepted by llvm-as-19)"
            fail=1
        fi
    else
        echo "OK     $f   (no llvm-as check)"
    fi
done

exit $fail