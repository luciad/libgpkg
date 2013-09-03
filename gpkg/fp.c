/*
 * Copyright 2013 Luciad (http://www.luciad.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string.h>
#include "fp.h"

int fp_isnan(double x) {
  return x != x;
}

double fp_nan() {
  /*
   * IEEE-754 double format
   *   63   : sign
   *   62-52: exponent
   *   51-00: fraction
   * IEEE-754 NaN
   *   exponent all 1 bits
   *   non zero fraction
   * IEEE-754 quiet NaN
   *   most significant fraction bit set
   */
  uint64_t nan_bits = 0x7ff8000000000000ULL;
  return fp_uint64_to_double(nan_bits);
}

uint64_t fp_double_to_uint64(double x) {
  /* Assumes double matches IEEE 754 double format */
  uint64_t bits;
  memcpy(&bits, &x, sizeof(uint64_t));
  return bits;
}

double fp_uint64_to_double(uint64_t x) {
  /* Assumes double matches IEEE 754 double format */
  double dbl;
  memcpy(&dbl, &x, sizeof(uint64_t));
  return dbl;
}
