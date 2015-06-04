All binaries are built with the [MinGW-builds](http://mingw-w64.sourceforge.net/download.php)
toolchain, currently with `i686-4.9.2-win32-sjlj-rt_v3-rev1`.

Just run `mingw32-make` in the root folder to produce the plugin ZIP.
Cross-compilation from Linux is also possible with the MinGW-w64 toolchain
by running `make` after installing the `mingw-w64` package in Debian.

The 3rd-party libraries (zlib, OpenSSL and libssh) were produced on
Debian 8, after installing the `python mingw-w64 cmake` packages
and running the `build_3rdparty.py` script. The necessary files
were then copied manually to the `3rdparty` folder.
