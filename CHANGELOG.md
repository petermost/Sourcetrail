### Changelog

#### 2025.12.8
- C++: Add indexing of structured binding declarations
- C++: Add indexing of `auto` prvalue casts
- GUI: Fix error/status view not cleared between indexing ([#51](https://github.com/petermost/Sourcetrail/issues/51))
- C/C++: Replace msvc mulitithreading library switches with corresponding clang switch
- C/C++: Add Visual Studio 2026 support
- Database: Enable simple database performance improvement
- Python: Remove non-working Python support
- C/C++: Remove support for Visual Studio 2010 to 2015

#### 2025.10.13
- C/C++: Add indexing of `concept` type constraints
- C/C++: Add indexing of abbreviated function templates
- C/C++: Use correct keyword for template template parameters (clang/LLVM >= 20)

#### 2025.9.9
- C/C++: Add indexing of `auto` return types
- GUI: Allow tab closing with middle mouse click
- GUI: Improve layout of license window content
- GUI: Add `Open` to context menu of start window

#### 2025.7.11
- C/C++: Add indexing of `constexpr`
- C/C++: Replace most `msvc` compiler switches with the correct `clang` switches (Fixes a long standing issue ["no such file or directory (sourcetrail is treating MSVC options as file/dir)"](https://github.com/CoatiSoftware/Sourcetrail/issues/744)
- Java: Add support for Java 24
- C/C++/Java: Revised the list of keywords for syntax highlighting

#### 2025.6.19
* GUI: Allow removing projects from the `Recent Projects` list
* GUI: Fix highlighting of `Text` and `On-Screen` search results for UTF-16/UTF-32 text
* GUI: Show configured text encoding in the status bar
* Internal: Switch to ['UTF-8 Everywhere'](https://utf8everywhere.org/)
* Internal: Switch to Qt resource system for most GUI resources

#### 2025.5.1
* GUI: Fix handling of Esc/Return keys for dialogs (Indexing, Bookmark, etc.) (Fixes [issue 27](https://github.com/petermost/Sourcetrail/issues/27))
* GUI: Activate bookmark with double click and close bookmark manager
* GUI: Highlight the taskbar entry when indexing has finished
* GUI: Show indexing progress in window title
* GUI: Added tooltips or prompt texts to many widgets

#### 2025.4.1
* Java: Add Support for record classes
* macOS: Fix vcpkg build. Thanks to [ChristianWieden](https://github.com/ChristianWieden) for the help

#### 2025.3.3
* Java: Add support for Eclipse JDT 3.40 (Java 23)
* Java: Update Gradle support to 8.12

#### 2025.1.28
* C/C++: Add support for Clang 19 (C++23).
* C/C++: Re-enable detection of non-trivial destructor calls.
* Fix: Keep the console window open when logging is enabled.
* Framework: Replace/Remove last Qt5 dependency.

#### 2024.9.23
* GUI: Try to hide the external console window on Windows. See ["The console window is not hidden under Windows 11"](https://github.com/petermost/Sourcetrail/issues/19) for additional information.
* GUI: Add the 'Hack' font.
* Fix: Copy the tutorial project files on initial run.

#### 2024.8.2
* GUI: Remove `qt.conf` which seems to improve the menu font rendering under Windows

#### 2024.7.3
* GUI: Fix non-working dialogs i.e. the selected action weren't executed
* C/C++: Disabled indexing of non-trivial destructor calls. See [#7 (comment)](https://github.com/petermost/Sourcetrail/issues/7#issuecomment-2199640807) for further details.

#### 2024.7.2
* Installation: Add Debian packaging

#### 2024.7.1
* C/C++: Add indexing of the deduced type of auto variables
* C/C++: Add indexing of user defined conversion operators
* C/C++: Add indexing of non-trivial destructor calls

#### 2024.7.0
* C/C++: Update libClang/LibTooling to Clang 18

#### 2024.6.0
* C/C++: Add indexing of the deduced type of auto variables

#### 2024.5.9
* C/C++: Add indexing of user defined conversion operators
* C/C++: Update support for C++ standards C++20, C++23
* C/C++: Update detection of 'Global Include Paths' for Visual Studio 2017, 2019, 2022
* Java: Generalize detection of JRE/JVM
* Java: Generalize detection of Maven
* Java: Update support for Java Standard 16, 17, 18, 19, 20 
* Framework: Update libClang/LibTooling to Clang 16/17
* Framework: Switch from Qt5 to Qt6

#### 2021.4.19 - 0.3.0
[Coati Changelog](docs/COATI_CHANGELOG.md)