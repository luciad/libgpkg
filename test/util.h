#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

char* hexToBin(char* hexString);

char* nBinToHex(uint8_t* binString, size_t len);

#endif