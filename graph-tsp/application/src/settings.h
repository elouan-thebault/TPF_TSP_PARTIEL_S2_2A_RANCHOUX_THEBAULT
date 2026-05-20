/*
    Copyright (c) Arnaud BANNIER, Nicolas BODIN and Matthieu LE BERRE.
    Licensed under the MIT License.
    See LICENSE.md in the project root for license information.
*/

#pragma once

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#define AssertNew(ptr) { if (ptr == NULL) { assert(false); abort(); } }

#ifdef _MSC_VER
#  define INLINE inline
#else
#  define INLINE static inline
#endif

#ifndef M_PI
#define M_PI 3.1415926535897931
#endif

#define ROOT_PATH "../../.."
#define DATA_PATH ROOT_PATH"/data"
#define TEST_PATH ROOT_PATH"/test"
#define GEOJSON_PATH DATA_PATH "/result.geojson"
#define TESTTEST_PATH TEST_PATH "/3_tsp_heuristic/input1.txt"

#define EARTH_RADIUS_KM 6371.0
#define DEG_TO_RAD(x) ((x) * M_PI / 180.0)
