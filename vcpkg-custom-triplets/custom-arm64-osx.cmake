# Prevent CMake from tracking the compiler in the port hash
set(VCPKG_DISABLE_COMPILER_TRACKING ON)

# On Apple Silicon macOS:
# - Do NOT set VCPKG_CMAKE_SYSTEM_NAME to "Linux" or leave it blank to let CMake autodetect Darwin
# - or you can explicitly set it to "Darwin" to clarify it’s macOS.

# Uncomment the next line if you want to explicitly name Darwin:
set(VCPKG_CMAKE_SYSTEM_NAME Darwin)

# Set the architecture to arm64 for Apple Silicon
set(VCPKG_TARGET_ARCHITECTURE arm64)

# Set static linking for the libraries
set(VCPKG_LIBRARY_LINKAGE static)

# On macOS, this is typically ignored; but you can leave it as-is
set(VCPKG_CRT_LINKAGE dynamic)

# If you need to handle special cases for, e.g., Catch2 on macOS,
# you can keep your existing logic but direct it to the right OS:
if (CMAKE_HOST_APPLE)
    set(VCPKG_CMAKE_CONFIGURE_OPTIONS "-DCATCH_CONFIG_NO_POSIX_SIGNALS=ON")
endif()
