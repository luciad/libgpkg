#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

void fail_flm(const char* file, int line, char* msg) {
    printf("%s:%d: %s\n", file, line, msg);
    exit(1);
}

void assert_str_eql_flm(const char* file, int line, char* msg, char* exc, char* act) {
    if (
        (exc == NULL && act == NULL) ||
        (exc != NULL && act != NULL && strcmp(exc, act) == 0)
    ) {
  	  	return;
    } else {
        printf("%s:%d: %s: expected <%s> actual <%s>\n", file, line, msg, exc, act);
        exit(1);
    }
}

static char hexCharToInt(char c) {
    if ('a' <= c && c <= 'f') {
        return 10 + c - 'a';
    } else if ('A' <= c && c <= 'F') {
        return 10 + c - 'a';
    } else if ('0' <= c && c <= '9') {
        return c - '0';
    } else {
        return 0;
    }
}

static char toHexChar(char c) {
    if (0 <= c && c <= 9) {
        return '0' + c;
    } else if (10 <= c && c <= 15) {
        return 'a' + (c - 10);
    } else {
        return '_';
    }
}

char* hexToBin(char* hexString) {
    size_t hexLength = strlen(hexString);
    size_t binLength = hexLength / 2;
    char* binary = calloc(binLength + 1, sizeof(char));
    for(int i = 0, j = 0; i < hexLength; i+=2, j++) {
        char c1 = hexCharToInt(hexString[i]);
        char c2 = hexCharToInt(hexString[i + 1]);
        binary[j] = c1 << 4 | c2;
    }

    return binary;
}

char* nBinToHex(uint8_t* binString, size_t length) {
    size_t binLength = length;
    size_t hexLength = binLength * 2;
    char* hex = calloc(hexLength + 1, sizeof(char));
    for(int h = 0, b = 0; b < binLength; h +=2, b++) {
        char c = binString[b];
        hex[h] = toHexChar( (c >> 4) & 0xF );
        hex[h + 1] = toHexChar( c & 0xF );
    }

    return hex;
}