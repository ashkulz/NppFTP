Currently, NppFTP can only be built with MinGW-w64.

For running `build_3rdparty.py`, you need a working internet connection
to download/compile the 3rd-party libraries (zlib, OpenSSL and libssh).

MinGW-w64
---------

Cross-compiling from Ubuntu 16.04 "xenial":
* Install the necessary packages: `sudo apt install -y python3 mingw-w64 cmake zip`.
* Run `./build_3rdparty.py` to download and compile required libraries.
* Run `make -f Makefile.mingw` to compile and produce the zipped plugin.

Compiling on Windows:
* Install the latest [MinGW-builds](http://mingw-w64.sourceforge.net/download.php) toolchain.
* Copy the `3rdparty` folder from the above cross-compilation setup.
* Run `mingw32-make -f Makefile.mingw` to compile and produce the zipped plugin.
