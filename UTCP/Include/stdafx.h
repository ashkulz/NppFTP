#include <windows.h>

inline bool isspace(char c) {
	return (c == ' ' || c == '\n' ||c == '\r' ||c == '\t');
}

inline bool isdigit(char c) {
	return (c >= '0' && c <= '9');
}


