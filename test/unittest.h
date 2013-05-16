#ifndef UNITTEST_H
#define UNITTEST_H

#include <stdint.h>
#include <stdlib.h>
#include <sqlite3.h>

void fail_flm(const char* file, int line, const char* msg);
#define fail() fail_flm(__FILE__,__LINE__,NULL)
#define fail_m(msg) fail_flm(__FILE__,__LINE__,(msg))

void assert_str_eql_flm(const char* file, int line, const char* msg, const char* exc, const char* act);
#define assert_str_eql(exc,act) assert_str_eql_flm(__FILE__,__LINE__,NULL,(exc),(act))
#define assert_str_eql_m(msg,exc,act) assert_str_eql_flm(__FILE__,__LINE__,(msg),(exc),(act))

void assert_double_eql_flm(const char* file, int line, const char* msg, double exc, double act);
#define assert_double_eql(exc,act) assert_double_eql_flm(__FILE__,__LINE__,NULL,(exc),(act))
#define assert_double_eql_m(msg,exc,act) assert_double_eql_flm(__FILE__,__LINE__,(msg),(exc),(act))

void assert_int_eql_flm(const char* file, int line, const char* msg, int exc, int act);
#define assert_int_eql(exc,act) assert_int_eql_flm(__FILE__,__LINE__,NULL,(exc),(act))
#define assert_int_eql_m(msg,exc,act) assert_int_eql_flm(__FILE__,__LINE__,(msg),(exc),(act))


void unittest_init();

int open_database(sqlite3 **db);

int close_database(sqlite3 *db);

uint8_t* hexToBin(char* hexString);

char* nBinToHex(uint8_t* binString, size_t len);

#endif