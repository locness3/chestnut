#!/usr/bin/env bash
set -e

# build lib with coverage
# build uts with coverage
cd unittests/build/debug/obj
gcov main.gcno
lcov -c -d . --output-file coverage.info
lcov -r coverage.info  "*Qt*.framework*" "*.moc" "*moc_*.cpp" -o coverage-filtered.info
genhtml -o results coverage-filtered.info
