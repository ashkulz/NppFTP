All binaries are built with the Mingw-builds [1] toolchain, currently with
i686-4.9.2-win32-sjlj-rt_v3-rev1. Just run `mingw32-make` in the root
folder to produce the plugin ZIP. Cross-compilation from Linux is also
possible with the MinGW-w64 toolchain (on Debian/Ubuntu just run
`sudo apt-get install mingw-w64` to install it) by running `make`.

Library versions used:
 * zlib 1.2.8
 * OpenSSL 1.0.1l
 * libssh 0.6.4

All the above libraries were cross-compiled on Ubuntu 14.04.1 using
the MinGW-w64 package present on it. For compiling libssh, the
mingw-w64-cross-compile.cmake was used for cross-compiling setup and
libssh-0.6.4.patch was applied on top of a bug fix for Windows [2].
The following commands were used to perform the build:

    cmake -DCMAKE_TOOLCHAIN_FILE=~/mingw-w64-cross-compile.cmake -DWITH_STATIC_LIB=ON -DCMAKE_INSTALL_PREFIX=/home/build/mingw-w64-cross-win32 <libssh-src-dir>
    make && make install

[1] http://mingw-w64.sourceforge.net/download.php#mingw-builds
[2] http://git.libssh.org/projects/libssh.git/commit/?id=e051135
