#include "utf8.h"
#include <stdio.h>

int utf8Next(const char** strRef, int* codeRef) {
	unsigned const char* str = (unsigned const char*) *strRef;
	unsigned const char first = *str;
	if (isASCII(first)) {
		*codeRef = first;
		++*strRef;
		return 0;
	}
	if (isContinuation(first))
		return ERR_UNEXPECTED_CONTINUATION | first;
	if (first >= 248)
		return ERR_INVALID_UTF8_BYTE | first;
	int expectedContinuations =
		is2ByteStarter(first) ? 1 :
		is3ByteStarter(first) ? 2 : 3;
	int code = first - (
		is2ByteStarter(first) ? 192 :
		is3ByteStarter(first) ? 224 : 240);
	for (int i = 1; i <= expectedContinuations; ++i) {
		unsigned char b = str[i];
		if (b >= 248) return ERR_INVALID_UTF8_BYTE | (i << 8) | b;
		if (!isContinuation(b)) return ERR_CONTINUATION_EXPECTED | (i << 8) | b;
		code = (code << 6) | (b & 0x7f);
	}
	*codeRef = code;
	*strRef += 1 + expectedContinuations;
	return 0;
}