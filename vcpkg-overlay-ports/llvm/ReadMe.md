To update LLVM:
1) `vcpkg.json`: Set `version-string` to the new version number
1) `portfile.cmake`: Set `SHA512` to 0
1) Run `cmake --preset vcpkg-release`
1) Copy/Paste correct SHA512 value to `version-string` in `vcpkg-json`
