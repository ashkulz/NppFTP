#include <winsock2.h>
#include <windows.h>

inline bool isspace(char c) {
    return (c == ' ' || c == '\n' ||c == '\r' ||c == '\t');
}

inline bool isdigit(char c) {
    return (c >= '0' && c <= '9');
}

#include <algorithm>
//avoid issues with std::min and std::max and ms compilers
//see https://stackoverflow.com/questions/5004858/why-is-stdmin-failing-when-windows-h-is-included
