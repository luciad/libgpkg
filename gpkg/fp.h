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
#ifndef GPKG_FP_H
#define GPKG_FP_H

#include <stdint.h>

/**
 * Determines if the given value is a NaN value.
 * @param x any double value
 * @return 1 if x is NaN; 0 otherwise
 */
int fp_isnan(double x);

/**
 * Returns a quiet NaN value.
 * @return a quiet NaN
 */
double fp_nan();

/**
 * Returns the IEEE-754 representation of the given double value as a 64-bit unsigned integer value.
 * @param x the double value to convert
 * @return an unsigned 64-bit integer containing the IEEE-754 representation of the given value.
 */
uint64_t fp_double_to_uint64(double x);

/**
 * Returns a double obtained by interpreting the bits in the given 64-bit unsigned integer value as an IEEE-754
 * double precision floating point number.
 * @param x an unsigned 64-bit integer containing an IEEE-754 double precision floating point number
 * @return the double floating-point value with the same bit pattern
 */
double fp_uint64_to_double(uint64_t x);

#endif
