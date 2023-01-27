// Eternal Horror
// Copyright(C) 2020 John D. Corrado
// Planes based on Doom visplane code
// Copyright(C) 1993-1996 Id Software, Inc.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#include <limits.h>
#include <stdint.h>

#include "fixed.h"

const fixed_t sinTable[ANGLES >> 2] =
{
	402, 1206, 2010, 2814, 3617, 4420, 5222, 6023, 6823, 7623, 8421, 9218, 10013, 10807, 11600, 12390,
	13179, 13966, 14751, 15533, 16313, 17091, 17866, 18638, 19408, 20175, 20938, 21699, 22456, 23210, 23960, 24707,
	25450, 26189, 26925, 27656, 28383, 29105, 29824, 30538, 31247, 31952, 32651, 33346, 34036, 34721, 35400, 36074,
	36743, 37406, 38064, 38716, 39362, 40002, 40636, 41263, 41885, 42501, 43110, 43712, 44308, 44897, 45480, 46055,
	46624, 47186, 47740, 48288, 48828, 49360, 49886, 50403, 50914, 51416, 51911, 52398, 52877, 53348, 53811, 54266,
	54713, 55152, 55582, 56004, 56417, 56822, 57219, 57606, 57986, 58356, 58718, 59070, 59414, 59749, 60075, 60392,
	60700, 60998, 61288, 61568, 61839, 62100, 62353, 62596, 62829, 63053, 63268, 63473, 63668, 63854, 64030, 64197,
	64353, 64501, 64638, 64766, 64884, 64992, 65091, 65179, 65258, 65327, 65386, 65436, 65475, 65505, 65524, 65534
};

const fixed_t tanTable[ANGLES >> 2] =
{
	402, 1206, 2011, 2816, 3622, 4430, 5238, 6048, 6861, 7675, 8491, 9310, 10132, 10957, 11786, 12618,
	13454, 14294, 15139, 15989, 16843, 17704, 18569, 19441, 20320, 21205, 22097, 22996, 23903, 24819, 25743, 26675,
	27618, 28570, 29532, 30505, 31489, 32485, 33493, 34514, 35548, 36596, 37658, 38736, 39829, 40939, 42065, 43210,
	44373, 45556, 46759, 47984, 49230, 50500, 51794, 53114, 54460, 55834, 57236, 58669, 60134, 61633, 63166, 64736,
	66345, 67994, 69685, 71422, 73205, 75038, 76923, 78864, 80862, 82922, 85047, 87241, 89507, 91851, 94277, 96790,
	99396, 102101, 104911, 107834, 110877, 114049, 117360, 120820, 124439, 128232, 132211, 136392, 140792, 145431, 150329, 155512,
	161005, 166839, 173050, 179677, 186765, 194367, 202544, 211365, 220913, 231286, 242597, 254986, 268616, 283691, 300457, 319222,
	340373, 364404, 391956, 423871, 461291, 505787, 559593, 625996, 710035, 819849, 969498, 1185538, 1524876, 2135471, 3559833, 10680573
};

fixed_t fixedSin(angle_t a)
{
	const uint32_t quadrant = (((a & ANGLESMASK) & 0x180) >> 7);
	const uint32_t index = (((a & ANGLESMASK) & 0x7F) >> 0);
	switch (quadrant)
	{
	case 0: return sinTable[index];
	case 1: return sinTable[127 - index];
	case 2: return -sinTable[index];
	case 3: return -sinTable[127 - index];
	default: return 0;
	}
}

fixed_t fixedCos(angle_t a)
{
	const uint32_t quadrant = (((a & ANGLESMASK) & 0x180) >> 7);
	const uint32_t index = (((a & ANGLESMASK) & 0x7F) >> 0);
	switch (quadrant)
	{
	case 0: return sinTable[127 - index];
	case 1: return -sinTable[index];
	case 2: return -sinTable[127 - index];
	case 3: return sinTable[index];
	default: return 0;
	}
}

fixed_t fixedTan(angle_t a)
{
	const uint32_t quadrant = (((a & ANGLESMASK) & 0x180) >> 7);
	const uint32_t index = (((a & ANGLESMASK) & 0x7F) >> 0);
	switch (quadrant)
	{
	case 0: case 2: return tanTable[index];
	case 1: case 3: return -tanTable[127 - index];
	default: return 0;
	}
}

fixed_t fixedCot(angle_t a)
{
	const uint8_t quadrant = (((a & ANGLESMASK) & 0x180) >> 7);
	const uint8_t index = (((a & ANGLESMASK) & 0x7F) >> 0);
	switch (quadrant)
	{
	case 0: case 2: return tanTable[127 - index];
	case 1: case 3: return -tanTable[index];
	default: return 0;
	}
}

fixed_t fixedMul(fixed_t a, fixed_t b)
{
	int64_t result = ((int64_t) a * (int64_t) b) >> FRACBITS;
	return (result < INT_MIN ? INT_MIN : result > INT_MAX ? INT_MAX : (fixed_t) result);
}