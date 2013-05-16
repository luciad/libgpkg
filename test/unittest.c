#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "gpkg.h"
#include "unittest.h"

#define SQLITE_CORE
#include <sqlite3ext.h>
#undef SQLITE_CORE

static struct sqlite3_api_routines sqlite3_api;

#define reg(func) (sqlite3_api.func = sqlite3_ ## func)

void unittest_init() {
    // Generated using grep sqlite3_\.*\( common/*.c | sed 's/^.*sqlite3_\([^(]*\)(.*$/reg(\1);/' | sort | uniq
    reg(bind_double);
    reg(bind_int);
    reg(bind_text);
    reg(column_bytes);
    reg(column_count);
    reg(column_int);
    reg(column_text);
    reg(context_db_handle);
    reg(create_function_v2);
    reg(errmsg);
    reg(finalize);
    reg(free);
    reg(malloc);
    reg(prepare_v2);
    reg(realloc);
    reg(reset);
    reg(result_blob);
    reg(result_double);
    reg(result_error);
    reg(result_error_code);
    reg(result_int);
    reg(result_null);
    reg(result_text);
    reg(step);
    reg(value_bytes);
    reg(value_int);
    reg(value_text);
    reg(vmprintf);
    gpkg_init(&sqlite3_api);
}

static uint8_t hexCharToInt(uint8_t c) {
    if ('a' <= c && c <= 'f') {
        return 10 + c - 'a';
    } else if ('A' <= c && c <= 'F') {
        return 10 + c - 'A';
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

uint8_t* hexToBin(char* hexString) {
    size_t hexLength = strlen(hexString);
    size_t binLength = hexLength / 2;
    uint8_t* binary = calloc(binLength + 1, sizeof(uint8_t));
    for(int i = 0, j = 0; i < hexLength; i+=2, j++) {
        uint8_t c1 = hexCharToInt(hexString[i]);
        uint8_t c2 = hexCharToInt(hexString[i + 1]);
        binary[j] = c1 << 4 | c2;
    }

    return binary;
}

char* nBinToHex(uint8_t* binString, size_t length) {
    size_t binLength = length;
    size_t hexLength = binLength * 2;
    char* hex = calloc(hexLength + 1, sizeof(char));
    for(int h = 0, b = 0; b < binLength; h +=2, b++) {
        uint8_t c = binString[b];
        hex[h] = toHexChar( (c >> 4) & 0xF );
        hex[h + 1] = toHexChar( c & 0xF );
    }

    return hex;
}

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