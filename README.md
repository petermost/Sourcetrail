# Sourcetrail

Sourcetrail is a free and open-source cross-platform source explorer that helps you get productive on unfamiliar source code. It is:
* free
* working offline
* operating on Linux, Windows (and macOS)
* supporting C, C++ and Java
* offering an SDK ([SourcetrailDB](https://github.com/CoatiSoftware/SourcetrailDB)) to write custom language extensions

# 🆕 Binary Releases

Binary releases are available for [sponsors](https://github.com/sponsors/petermost).


## __Links__
* [Quick Start Guide (Version 2021.4)](DOCUMENTATION.md#getting-started)
* [Documentation (Version 2021.4)](DOCUMENTATION.md)
* [Building](#building)

!["Sourcetrail User Interface"](docs/readme/user_interface.png "Sourcetrail User Interface")



## License

Sourcetrail is licensed under the [GNU General Public License Version 3](LICENSE.txt).

## Note

__Important:__ This project was archived by the original autors and maintainers of Sourcetrail by the end of 2021. If you want to know more about this decision, read more in the original [README.md](unused_coati_software_files/README.md)

This is a fork of the Sourcetrail project and I want to keep this project at least buildable. 

# Building

There are 2 ways to build the project:
1. With **vcpkg** provided packages ([Vcpkg build](#vcpkg-build))
2. With the **system** provided packages ([System build](#system-build))

## Cloning

It is important to clone the repository with the **submodules** and the **symlinks**:
```
# note under windows either development mode or an elevated git terminal/console is required or the git clone fails
# due to failing of symlink creation
git clone https://github.com/petermost/Sourcetrail.git --recurse-submodules --config core.symlinks=true
```
and get the updates with:
```
git pull --recurse-submodules
```



## Vcpkg Build

* In order to build the **java_indexer** additional software must be installed which are not provided by vcpkg:
    * [OpenJDK](https://jdk.java.net/)
    * [Maven](https://maven.apache.org/)
* **Linux:** Because **Qt6** is build from source, additional packages are needed. Install these packages with `script/install-qt6-dependencies.sh`.
* **Windows:** The build must be done in a **"x64 Native Tools Command Prompt for VS 2022"**.
* **MacOS:** to compile under MacOS several things have to be installed beforehand, even with vcpkg  
* * XCode has to be installed
* * the following packages have to be installed via brew `brew install libtools maven autoconf autoconf-archive automake patchelf ninja`

Prepare the build in Windows and Linux:
```
$ cd Sourcetrail
$ cmake --preset vcpkg-ninja-release
```

Prepare the build in MacOs:
```
$ cd Sourcetrail
$ cmake --preset vcpkg-ninja-release-macos
```

Note that the initial compilation of the vcpkg packages (especially LLVM) will take a **long** time! Depending on your machine, it probably will take about 2 to 3 hours!

Build:
```
$ cd ../build/vcpkg-ninja-release
$ cmake --build .
```


## System build

### Linux

To compile it under (K)ubuntu 24.10, "Oracular Oriole", install the following packages:

**General packages:** cmake, ninja-build, libboost1.83-all-dev, qt6-base-dev, qt6-svg-dev, libsqlite3-dev, libtinyxml-dev

**C++ packages:** clang-19, libclang-19-dev

**Java packages:** maven, openjdk-23-jdk

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
It's probably also possible to build with pre-installed libraries, like the original build instructions describe, but it's untested and unsupported.

### MacOS
System build was not tested under MacOS

### Used/Supported libraries: ###

**C++**

|Name|System|Vcpkg|Notes| 
|----|------|-----|-----|
|Clang|19.1.1|18.1.6|C++23 Support|

**Java**

|Name|System/Vcpkg|Notes|
|----|------------|-----|
|Eclipse JDT|3.40|Java 23 Support|
|Maven|Locally installed version|
|Gradle|8.12|

**Miscellaneous**

|Name|System|Vcpkg|
|----|------|-----|
|Qt|6.6.2|6.8.2|
|Boost|1.83.0|1.87.0|
|SQLite3|3.46.1|3.47.2|
|TinyXML|2.6.2|2.6.2|

**Tests**
|Name|System|Vcpkg|
|----|------|-----|
|Catch2|3.4.0|3.8.0|
|GTest|1.15.2|1.16.0|
