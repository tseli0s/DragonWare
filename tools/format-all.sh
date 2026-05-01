#!/bin/sh

# Just so that we don't have to type the entire command every time.
find . -type f \( -name "*.c" -o -name "*.h" \) -exec clang-format -i {} \;

