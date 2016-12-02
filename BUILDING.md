Currently, NppFTP can only be built with MinGW-w64. Dependent libraries i.e.
zlib, OpenSSL and libssh get automatically downloaded as a part of the build
process, and hence require a working internet connection.

MinGW-w64
---------

Cross-compiling from Ubuntu 16.04 "xenial":
* Install the necessary packages: `sudo apt install -y python3 mingw-w64 cmake zip`.
* Run `make -f Makefile.mingw BITS=32` to compile and produce the 32-bit plugin.
  Alternatively, run `make -f Makefile.mingw BITS=64` to produce the 64-bit plugin.
  If no value is specified for `BITS`, a 32-bit target is assumed.
