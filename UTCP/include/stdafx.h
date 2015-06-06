#include <winsock2.h>
#include <windows.h>

inline bool isspace(char c) {
	return (c == ' ' || c == '\n' ||c == '\r' ||c == '\t');
}

inline bool isdigit(char c) {
	return (c >= '0' && c <= '9');
}

#ifndef _MSC_VER
#include <algorithm>
#define min(a,b) std::min(a,b)
#define max(a,b) std::max(a,b)
#endif