# Windows:
#
# @echo off
# python "C:\Program Files\LLVM\bin\run-clang-tidy" -p ..\..\build\Engine-Debug-Ninja -fix -extra-arg="-w"

#!/bin/sh

# Running 'run-clang-tidy' with '-j 1' doesn't solve the problem of multiple fixes!
# run-clang-tidy-18 -p ../build/system-debug/ -fix -extra-arg="-w"
run-clang-tidy-20 -p ../build/system-release/ -fix
