# Sourcetrail

Sourcetrail is a free and open-source cross-platform source explorer that helps you get productive on unfamiliar source code. It is:
* Free
* Working offline
* Supporting C/C++ and Java
* Operating on Linux, Windows (and macOS)
* Offering a SDK ([SourcetrailDB](https://github.com/CoatiSoftware/SourcetrailDB)) to write custom language extensions

Sourcetrail is licensed under the [GNU General Public License Version 3](LICENSE.txt).

!["Sourcetrail User Interface"](docs/readme/user_interface.png "Sourcetrail User Interface")

## Important
This project was archived by the original authors and maintainers of Sourcetrail by the end of 2021. You can read more about this decision in this [blog entry](https://web.archive.org/web/20211115131149/https://www.sourcetrail.com/blog/discontinue_sourcetrail/).

This is a **fork** of the Sourcetrail project and I want to keep this project at least buildable.

## __Contents__
* [Quick Start Guide (Version 2021.4)](DOCUMENTATION.md#getting-started)
* [Documentation (Version 2021.4)](DOCUMENTATION.md)
* [Sponsoring](#star-sponsoring)
* [Changes](#changes)
* [Building](#building)

# :star: Sponsoring

If you like the changes I've done so far, then please consider [sponsoring me](https://github.com/sponsors/petermost). 
As a sponser you have access to the following **binary releases**:

|Platform|Packaging|Build|
|--------|---------|-----|
|Linux   |ZIP Archive, Debian Package|Vcpkg, System|
|Windows | ZIP Archive|Vcpkg|

### Used/Tested/Supported libraries: ###

**C++**

|Name|System|Vcpkg|Notes| 
|----|------|-----|-----|
|Clang/LLVM|20.1.2|18.1.6|C++23 Support|

**Java**

|Name|System/Vcpkg|Notes|
|----|------------|-----|
|Eclipse JDT|3.42|Java 24 Support|
|Maven|3.9.9|
|Gradle|8.14.2|

**Miscellaneous**

|Name|System|Vcpkg|
|----|------|-----|
|Qt|6.8.3|6.9.1|
|Boost|1.88.0|1.89.0|
|SQLite3|3.46.1|3.50.4|
|TinyXML|2.6.2|2.6.2|

**Tests**
|Name|System|Vcpkg|
|----|------|-----|
|Catch2|3.7.1|3.11.0|
|GTest|1.15.0|1.17.0|

### Changes

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

#### 2024.05.9
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

# Building

There are 2 ways to build the project:
1. With **vcpkg** provided packages ([Vcpkg build](#vcpkg-build))
2. With the **system** provided packages ([System build](#system-build))

## Cloning

It is important to clone the repository with the **submodules** and the **symlinks**:
```
git clone https://github.com/petermost/Sourcetrail.git --recurse-submodules --config core.symlinks=true
```
and get the updates with:
```
git pull --recurse-submodules
```



## Vcpkg Build
Depending on the platform and the selected indexer, additional software/packages must be installed.
* **Java Indexer:**
    * [OpenJDK](https://jdk.java.net/)
    * [Maven](https://maven.apache.org/)
* **Linux:** 
    * **Qt6:** Install additional packages with `scripts/install-qt6-dependencies.sh`.
* **Windows:**
    * [Visual Studio 2022 Community Edition](https://visualstudio.microsoft.com/vs/community/) 
* **macOS:**
  * [Xcode](https://developer.apple.com/xcode/)
  * libtools, autoconf, autoconf-archive, automake, patchelf, ninja

Prepare the build in a terminal or command prompt ("x64 Native Tools Command Prompt"):
```
$ cd Sourcetrail
$ cmake --preset vcpkg-ninja-release
```
Note that the initial compilation of the vcpkg packages (especially LLVM) will take a **long** time!

> [!TIP]
> Download a [binary release](#star-binary-releases).

Build:
```
$ cd ../build/vcpkg-ninja-release
$ cmake --build .
```


## System build

### Linux

To compile it under (K)ubuntu 25.04, "Plucky Puffin", install the following packages:

**General packages:** cmake, ninja-build, libboost1.88-all-dev, libboost-charconv1.88-dev, qt6-base-dev, qt6-svg-dev, libsqlite3-dev, libtinyxml-dev

**C++ packages:** clang-20, libclang-20-dev

**Java packages:** maven, openjdk-24-jdk

**Unit test packages:** catch2, libgtest-dev

Prepare the build:
```
$ cd Sourcetrail
$ cmake --preset system-ninja-release
```

Build:
```
$ cd ../build/system-ninja-release
$ cmake --build .
```

### Windows
System build is not tested and therefore not supported.

### macOS
System build is not tested and therefore not supported.
