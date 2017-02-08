Currently, NppFTP can only be built with MinGW-w64. Dependent libraries i.e.
zlib, OpenSSL and libssh get automatically downloaded as a part of the build
process, and hence require a working internet connection.

MinGW-w64
---------

Cross-compiling from Linux has been tested with Debian/Ubuntu and Fedora. You will need to
install the following packages:
* **Common**: `python3 cmake zip`
* **Debian/Ubuntu**: `mingw-w64`
* **Fedora 32-bit target**: `mingw32-gcc-c++ mingw32-winpthreads-static`
* **Fedora 64-bit target**: `mingw64-gcc-c++ mingw64-winpthreads-static`

Then, run `make -f Makefile.mingw BITS=32` to compile and produce the 32-bit plugin.
Alternatively, run `make -f Makefile.mingw BITS=64` to produce the 64-bit plugin.
If no value is specified for `BITS`, a 32-bit target is assumed.