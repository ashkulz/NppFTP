cmake_minimum_required (VERSION 3.5)

set (CMAKE_SYSTEM_NAME Windows)

if (MINGW)
    set (CMAKE_SYSROOT "$ENV{HOME}/bin/cross")

    set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM  NEVER)
    set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY  ONLY)
    set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE  ONLY)

    set (toolchain_prefix   i686-w64-mingw32)

    set (CMAKE_C_COMPILER   ${CMAKE_SYSROOT}/bin/${toolchain_prefix}-gcc)
    set (CMAKE_CXX_COMPILER ${CMAKE_SYSROOT}/bin/${toolchain_prefix}-g++)
    set (CMAKE_RC_COMPILER  ${CMAKE_SYSROOT}/bin/${toolchain_prefix}-windres)

    set (win32_inc_dir ${CMAKE_SYSROOT}/${toolchain_prefix}/include)
    set (win32_lib_dir ${CMAKE_SYSROOT}/${toolchain_prefix}/lib)
endif (MINGW)

# needed to build 3rdparty libs via python script
set(Python_ADDITIONAL_VERSIONS 3)
find_package(PythonInterp 3)
if( NOT PYTHONINTERP_FOUND )
  message(FATAL_ERROR
"Unable to find Python interpreter, required for builds and testing.

Please install Python or specify the PYTHON_EXECUTABLE CMake variable.")
endif()

if( ${PYTHON_VERSION_STRING} VERSION_LESS 3 )
  message(FATAL_ERROR "Python 3 or newer is required")
endif()

# Locate perl and check its version.
# needed to build 3rdparty lib openssl
find_package(Perl)
if(${PERL_VERSION_STRING} VERSION_LESS "5.10.0")
  message(FATAL_ERROR "NppFTP requires Perl 5.10.0 or later")
endif()

project (NppFTP)

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
set (output_dir "x64")
    if (MINGW)
        set (3rdparty_prefix "x86_64-w64-mingw32")
    else()
        set (3rdparty_prefix "msvc_x64")
    endif()
else()
set (output_dir "x86")
    if (MINGW)
        set (3rdparty_prefix "i686-w64-mingw32")
    endif()
endif()

#check if ExternalProject_Add could be used as replacement of the python script
#this would remove the dependency to python
add_custom_target(
    NppFTP3rdPartyPrerequists
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/build_3rdparty.py ${output_dir} ${3rdparty_prefix}
    )

if (MINGW)
    set (defs
        -DUNICODE -D_UNICODE -DMINGW_HAS_SECURE_API=1 -D_WIN32 -DWIN32
        -D_WIN32_WINNT=0x0501 -DWIN32_LEAN_AND_MEAN -DNOCOMM -DLIBSSH_STATIC
    )

    set (CMAKE_CXX_FLAGS
        "-std=c++11 -O3 -mwindows -mthreads -municode -Wall -Wno-unknown-pragmas"
    )

    set (CMAKE_MODULE_LINKER_FLAGS
        "-s -static"
    )
else (MINGW)
    set (defs
        -DUNICODE -D_UNICODE -D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES -D_CRT_SECURE_NO_WARNINGS -D_WIN32 -DWIN32
        -D_WIN32_WINNT=0x0501 -DWIN32_LEAN_AND_MEAN -DNOCOMM -DLIBSSH_STATIC
    )

    set (CMAKE_CXX_FLAGS
        "/EHsc /MP /W4"
    )
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif (MINGW)

set (project_rc_files
    src/Windows/NppFTP.rc
)

file(GLOB ftp_sources
    "src/*.h"
    "src/*.cpp"
    "src/Windows/*.h"
    "src/Windows/*.cpp"
)

file(GLOB tinyxml_sources
    "tinyxml/include/*.h"
    "tinyxml/src/*.cpp"
)


file(GLOB UTCP_sources
    "UTCP/include/*.h"
    "UTCP/src/*.cpp"
)

set (project_sources
    ${ftp_sources}
    ${tinyxml_sources}
    ${UTCP_sources}
)

include_directories (${CMAKE_SOURCE_DIR}/)
include_directories (${CMAKE_SOURCE_DIR}/src/)
include_directories (${CMAKE_SOURCE_DIR}/src/Windows/)
include_directories (${CMAKE_SOURCE_DIR}/tinyxml/include/)
include_directories (${CMAKE_SOURCE_DIR}/UTCP/include/)

#3rdparty stuff
include_directories (${CMAKE_BINARY_DIR}/${output_dir}/3rdparty/include/)
include_directories (${CMAKE_BINARY_DIR}/${output_dir}/3rdparty/include/libssh/)
include_directories (${CMAKE_BINARY_DIR}/${output_dir}/3rdparty/include/openssl/)

link_directories(${CMAKE_BINARY_DIR}/${output_dir}/3rdparty/lib/)

add_definitions (${defs})

add_library (NppFTP MODULE ${project_rc_files} ${project_sources})

add_dependencies(NppFTP NppFTP3rdPartyPrerequists)

if (MINGW)

    include_directories (${win32_inc_dir})
    #remove the lib from the generated target lib
    SET(CMAKE_SHARED_LIBRARY_PREFIX "")

    target_link_libraries (NppFTP comctl32 shlwapi ssh ssl crypto crypt32 z ws2_32 iphlpapi)
else (MINGW)
    target_link_libraries (NppFTP comctl32 shlwapi ssh libeay32 ssleay32 crypt32 zlib ws2_32 iphlpapi)
endif (MINGW)

# build a CPack driven zip package
#include (InstallRequiredSystemLibraries)

set (CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/zip)
install(TARGETS NppFTP 
  DESTINATION .)

INSTALL(DIRECTORY docs/ DESTINATION ./doc)


set(CPACK_PACKAGE_VERSION "0.29.15")
set(CPACK_GENERATOR ZIP)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)

INCLUDE(CPack)
