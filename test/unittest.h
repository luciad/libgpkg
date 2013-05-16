#ifndef UNITTEST_H
#define UNITTEST_H

#include <stdint.h>
#include <stdlib.h>

void fail_flm(const char* file, int line, char* msg);
#define fail() fail_flm(__FILE__,__LINE__,NULL)
#define fail_m(msg) fail_flm(__FILE__,__LINE__,(msg))

void assert_str_eql_flm(const char* file, int line, char* msg, char* exc, char* act);
#define assert_str_eql(exc,act) assert_str_eql_flm(__FILE__,__LINE__,NULL,(exc),(act))
#define assert_str_eql_m(msg,exc,act) assert_str_eql_flm(__FILE__,__LINE__,(msg),(exc),(act))

void unittest_init();

uint8_t* hexToBin(char* hexString);

char* nBinToHex(uint8_t* binString, size_t len);

#endif