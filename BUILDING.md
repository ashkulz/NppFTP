Currently, NppFTP can be built with [MinGW-w64](https://mingw-w64.org/doku.php) or [MS Visual Studio C++](https://www.visualstudio.com/de/downloads/) compiler for windows. Dependent libraries i.e.
[zlib](https://github.com/madler/zlib), [OpenSSL](https://www.openssl.org/) and [libssh](https://www.libssh.org/) get automatically downloaded as a part of the build process, and hence require a working internet connection. The currently used version could be checked seen in [build_3rdparty.py](https://github.com/ashkulz/NppFTP/blob/master/build_3rdparty.py) . [tinyxml](http://www.grinninglizard.com/tinyxml/) is contained in a modified version based on [2.6.2](https://sourceforge.net/projects/tinyxml/files/tinyxml/2.6.2/) .

General prerequisites
---------
* **perl** (PERL 5.10 or higher, needed for OpenSSL, see [Compilation_and_Installation](https://wiki.openssl.org/index.php/Compilation_and_Installation) )
* **python** (Version 3.0 or higher, needed for libssh and NppFTP script [build_3rdparty.py](https://github.com/ashkulz/NppFTP/blob/master/build_3rdparty.py) )
* MS Visual Studio 2013 or newer for Windows builds
* MinGW-w64 for Linux or Windows builds

MinGW-w64 Makefile (Makefile.mingw)
---------

Cross-compiling from Linux has been tested with Debian/Ubuntu (see [Travis CI builds](https://travis-ci.org/ashkulz/NppFTP) ) and Fedora. You will need to install the following packages:
* **Common**: `python3 cmake zip`
* **Debian/Ubuntu**: `mingw-w64`
* **Fedora 32-bit target**: `mingw32-gcc-c++ mingw32-winpthreads-static`
* **Fedora 64-bit target**: `mingw64-gcc-c++ mingw64-winpthreads-static`

Then, run `make -f Makefile.mingw BITS=32` to compile and produce the 32-bit plugin.
Alternatively, run `make -f Makefile.mingw BITS=64` to produce the 64-bit plugin.
If no value is specified for `BITS`, a 32-bit target is assumed.

CMake build (preliminary)
---------

You will need to install:

* see [General prerequisites](https://github.com/ashkulz/NppFTP/blob/master/BUILDING.md#general%20prerequisites) and package installation above
* [cmake binary](https://cmake.org/download/) (Version 2.8 or higher) matching to your platform (Linux or Windows are supported)

Prepare build and build:
* command line:
  * create a build directory, e.g. _build beside the source code
  * change to the new directory
  * call `cmake -G "$generator" ..` with generator value `MinGW makefiles` for MinGW-w64 or `Visual Studio 12 2013`, `Visual Studio 12 2013 Win64`
  * call `cmake --build . --config Release` to build the software
* with [cmake-gui](https://cmake.org/cmake/help/v3.8/manual/cmake-gui.1.html) on windows by selection of:
  * build directory
  * the local NppFTP source directory
  * an appropriate generator (for MinGW-w64 choose `MinGW makefiles` or `Visual Studio 12 2013`, `Visual Studio 12 2013 Win64`)
  * press button "Configure"
  * press button "Generate"
  * press button "Open Project"
  * build the project
  
CI builds
---------

- [Travis CI](https://travis-ci.org/) for MinGW-w64 on Linux of [NppFtp](https://travis-ci.org/ashkulz/NppFTP) . Travis is used for the release builds.
- [Appveyor CI](https://www.appveyor.com/) for windows of [NppFtp](https://ci.appveyor.com/project/ashkulz/nppftp/)
- Appveyor artifacts
  - New pull requests are automatically build with [appveyor CI](https://www.appveyor.com/) . The build artifacts are stored and could be downloaded and tested. The link could be found in each open PR at the end under "Show all checks".
  - The build history could be found at [https://ci.appveyor.com/project/ashkulz/nppftp/history](https://ci.appveyor.com/project/ashkulz/nppftp/history) .
