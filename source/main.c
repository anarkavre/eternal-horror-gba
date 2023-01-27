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

#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "fixed.h"
#include "graphics.h"
#include "levels.h"

typedef struct
{
	int32_t mapIndex;
	uint32_t state;
	fixed_t offset;
	uint32_t tics;
} door_t;

typedef struct
{
	int32_t mapIndex;
	uint32_t type;
	uint32_t state;
	int32_t health;
	int32_t gridX;
	int32_t gridY;
	uint32_t damageTics;
	uint32_t attackTics;
	uint32_t render;
	uint32_t damage;
} enemy_t;

typedef struct
{
	uint32_t type;
	int32_t gridX;
	int32_t gridY;
	uint32_t render;
} health_t;

typedef struct
{
	int32_t minX;
	int32_t maxX;
	uint32_t pad1;
	uint32_t top[120];
	uint32_t pad2;
} plane_t;

const fixed_t scalarTable[256] =
{
	8192, 8224, 8256, 8289, 8322, 8355, 8388, 8422, 8456, 8490, 8525, 8559, 8594, 8630, 8665, 8701,
	8738, 8774, 8811, 8848, 8886, 8924, 8962, 9000, 9039, 9078, 9118, 9157, 9198, 9238, 9279, 9320,
	9362, 9404, 9446, 9489, 9532, 9576, 9619, 9664, 9709, 9754, 9799, 9845, 9892, 9939, 9986, 10034,
	10082, 10131, 10180, 10230, 10280, 10330, 10381, 10433, 10485, 10538, 10591, 10645, 10699, 10754, 10810, 10866,
	10922, 10979, 11037, 11096, 11155, 11214, 11275, 11335, 11397, 11459, 11522, 11586, 11650, 11715, 11781, 11848,
	11915, 11983, 12052, 12122, 12192, 12264, 12336, 12409, 12483, 12557, 12633, 12710, 12787, 12865, 12945, 13025,
	13107, 13189, 13273, 13357, 13443, 13530, 13617, 13706, 13797, 13888, 13981, 14074, 14169, 14266, 14364, 14463,
	14563, 14665, 14768, 14873, 14979, 15087, 15196, 15307, 15420, 15534, 15650, 15768, 15887, 16008, 16131, 16256,
	16384, 16513, 16644, 16777, 16912, 17050, 17189, 17331, 17476, 17623, 17772, 17924, 18078, 18236, 18396, 18558,
	18724, 18893, 19065, 19239, 19418, 19599, 19784, 19972, 20164, 20360, 20560, 20763, 20971, 21183, 21399, 21620,
	21845, 22075, 22310, 22550, 22795, 23045, 23301, 23563, 23831, 24105, 24385, 24672, 24966, 25266, 25575, 25890,
	26214, 26546, 26886, 27235, 27594, 27962, 28339, 28728, 29127, 29537, 29959, 30393, 30840, 31300, 31775, 32263,
	32768, 33288, 33825, 34379, 34952, 35544, 36157, 36792, 37449, 38130, 38836, 39568, 40329, 41120, 41943, 42799,
	43690, 44620, 45590, 46603, 47662, 48770, 49932, 51150, 52428, 53773, 55188, 56679, 58254, 59918, 61680, 63550,
	65536, 67650, 69905, 72315, 74898, 77672, 80659, 83886, 87381, 91180, 95325, 99864, 104857, 110376, 116508, 123361,
	131072, 139810, 149796, 161319, 174762, 190650, 209715, 233016, 262144, 299593, 349525, 419430, 524288, 699050, 1048576, 2097152
};

const fixed_t planeDistanceTable[32] =
{
	268435456, 89478485, 53687091, 38347922, 29826161, 24403223, 20648881, 17895697, 15790320, 14128181, 12782640, 11671106, 10737418, 9942053, 9256395, 8659208,
	8134407, 7669584, 7255012, 6882960, 6547206, 6242685, 5965232, 5711392, 5478274, 5263440, 5064819, 4880644, 4709393, 4549753, 4400581, 4260880
};

const int32_t healthBarTable[100] =
{
	0, 0, 1, 1, 2, 3, 3, 4, 5, 5,
	6, 7, 7, 8, 8, 9, 10, 10, 11, 12,
	12, 13, 14, 14, 15, 16, 16, 17, 17, 18,
	19, 19, 20, 21, 21, 22, 23, 23, 24, 24,
	25, 26, 26, 27, 28, 28, 29, 30, 30, 31,
	32, 32, 33, 33, 34, 35, 35, 36, 37, 37,
	38, 39, 39, 40, 40, 41, 42, 42, 43, 44,
	44, 45, 46, 46, 47, 48, 48, 49, 49, 50,
	51, 51, 52, 53, 53, 54, 55, 55, 56, 56,
	57, 58, 58, 59, 60, 60, 61, 62, 62, 63
};

const uint32_t mapWidth = 64;
const uint32_t mapHeight = 64;

uint32_t mapData[4096];

uint32_t level = 1;
const uint32_t numLevels = 4;
const uint32_t *levels[] =
{
	level1Data,
	level2Data,
	level3Data,
	level4Data
};

fixed_t cameraX;
fixed_t cameraY;
angle_t cameraAngle;
fixed_t oldCameraX;
fixed_t oldCameraY;
uint32_t state = 2;
int32_t health;
uint32_t attackTics = 0;

uint32_t fireWeaponPressed;
uint32_t restartLevelPressed;

door_t doors[64] =
{
	{ -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 },
	{ -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 },
	{ -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 },
	{ -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 },
	{ -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 },
	{ -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 },
	{ -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 },
	{ -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }, { -1, 0, 64 << FRACBITS, 0 }
};

enemy_t enemies[64] =
{
	{ -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

health_t healths[64] =
{
	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 },
	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 },
	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 },
	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 },
	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 },
	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 },
	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 },
	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }
};

uint16_t *yTable[2][64];
uint16_t xTable[120];
uint32_t page = 1;

plane_t plane;

uint32_t start[32];
uint32_t stop[32];
fixed_t currentX[32];
fixed_t currentY[32];
fixed_t stepX[32];
fixed_t stepY[32];
fixed_t fovInvCos = 92119;
fixed_t invViewWidth = 512;

fixed_t zBuffer[120];

uint32_t frames[6] = { 24576, 28672, 32768, 36864, 40960, 45056 };
uint32_t frame = 0;
uint32_t frameTics = 0;

uint32_t bloodHeight[120];
uint32_t bloodSpeed[120];
uint32_t bloodTics = 0;

uint32_t solidPlanes = 0;

uint32_t IWRAM_CODE FindHeight(fixed_t d)
{
	int32_t l = 0;
	int32_t r = 255;
	
	while (l <= r)
	{
		int32_t m = l + ((r - l) >> 1);
		
		if ((scalarTable[m] << 6) == d)
			return 512 - 2 * m;
		
		if ((scalarTable[m] << 6) < d)
			l = m + 1;
		else
			r = m - 1;
	}
	
	if (r < 0)
		return 512;
	
	return 512 - 2 * r;
}

void IWRAM_CODE DrawWallSlice(const uint8_t *texture, uint32_t textureOffsetX, int32_t wallX, int32_t wallY, uint32_t wallHeight)
{
	uint32_t count;
	fixed_t textureOffsetY;
	fixed_t scalar = scalarTable[(512 - wallHeight) >> 1];
	texture = &texture[textureOffsetX * 64];
	
	if (wallY < 0)
	{
		count = 63;
		textureOffsetY = -wallY * scalar;
		wallY = 0;
	}
	else
	{
		count = wallHeight - 1;
		textureOffsetY = 0;
	}
	
	uint16_t *p = yTable[page][wallY] + xTable[wallX];
	
	do
	{
		int32_t color = texture[textureOffsetY >> FRACBITS];
		*p = color << 8 | color;
		p += SCREEN_WIDTH >> 1;
		*p = color << 8 | color;
		p += SCREEN_WIDTH >> 1;
		textureOffsetY += scalar;
	} while (count--);
}

void IWRAM_CODE DrawSprite(const uint8_t *sprite, int32_t spriteX, int32_t spriteY, uint32_t spriteSize, fixed_t spriteDistance, uint32_t spriteDamage)
{
	if (spriteX + (int32_t)spriteSize <= 0 || spriteX > 119)
		return;
	
	uint32_t countX;
	uint32_t countY;
	uint32_t yCount;
	fixed_t spriteOffsetX;
	fixed_t spriteOffsetY;
	fixed_t ySpriteOffset;
	fixed_t scalar = scalarTable[(512 - spriteSize) >> 1];
	int32_t colorKey = 0x0C;
	int32_t damageColor = 0x2A;
	
	if (spriteX < 0)
	{
		countX = spriteSize + spriteX - 1;
		spriteOffsetX = -spriteX * scalar;
		spriteX = 0;
	}
	else
	{
		countX = (spriteX + spriteSize > 119 ? 120 - spriteX : spriteSize) - 1;
		spriteOffsetX = 0;
	}
	
	const uint8_t *spriteColumn = &sprite[(spriteOffsetX >> FRACBITS) * 64];
	
	if (spriteY < 0)
	{
		countY = yCount = 63;
		spriteOffsetY = ySpriteOffset = -spriteY * scalar;
		spriteY = 0;
	}
	else
	{
		countY = yCount = spriteSize - 1;
		spriteOffsetY = ySpriteOffset = 0;
	}
	
	uint16_t *p = yTable[page][spriteY] + xTable[spriteX];
	uint16_t *temp = p;
	
	do
	{
		if (spriteDistance < zBuffer[spriteX++])
		{
			do
			{
				int32_t color = spriteColumn[spriteOffsetY >> FRACBITS];
				if (color != colorKey)
					*p = spriteDamage ? damageColor << 8 | damageColor : color << 8 | color;
				p += SCREEN_WIDTH >> 1;
				if (color != colorKey)
					*p = spriteDamage ? damageColor << 8 | damageColor : color << 8 | color;
				p += SCREEN_WIDTH >> 1;
				spriteOffsetY += scalar;
			} while (countY--);
		}
		
		spriteOffsetX += scalar;
		spriteColumn = &sprite[(spriteOffsetX >> FRACBITS) * 64];
		countY = yCount;
		spriteOffsetY = ySpriteOffset;
		p = ++temp;
	} while (countX--);
}

void IWRAM_CODE DrawGraphic(const uint8_t *graphic, int32_t srcX, int32_t srcY, int32_t dstX, int32_t dstY, int32_t width, int32_t height)
{
	int32_t colorKey = 0x0C;
	
	graphic = &graphic[srcY * 64 + srcX];
	
	uint16_t *p = yTable[page][dstY] + xTable[dstX];
	
	uint32_t countY = 2 * height - 1;
	
	do
	{
		uint32_t countX = width - 1;
		
		do
		{
			int32_t color = *(graphic++);
			if (color != colorKey)
				*p = color << 8 | color;
			p++;
		} while (countX--);
		
		if ((countY & 1) == 0)
			graphic += 64 - width;
		else
			graphic -= width;
		
		p += (SCREEN_WIDTH >> 1) - width;
	} while (countY--);
}

void IWRAM_CODE DrawRect(int32_t x, int32_t y, int32_t width, int32_t height, uint8_t color)
{
	uint16_t *p = yTable[page][y] + xTable[x];
	
	uint32_t countY = 2 * height - 1;
	
	do
	{
		uint32_t countX = width - 1;
		
		do
		{
			*p = color << 8 | color;
			p++;
		} while (countX--);
		
		p += (SCREEN_WIDTH >> 1) - width;
	} while (countY--);
}

void Update()
{
	uint16_t keys = keysDown();
	
	restartLevelPressed = 0;
	
	if (keys & KEY_START)
		restartLevelPressed = 1;
	
	if (state == 1)
	{
		if (keys & KEY_SELECT)
			solidPlanes = !solidPlanes;
		
		keys = keysHeld();
		
		oldCameraX = cameraX;
		oldCameraY = cameraY;
		
		if (keys & KEY_UP)
		{
			cameraX += fixedMul(559240, fixedCos(cameraAngle));
			cameraY -= fixedMul(559240, fixedSin(cameraAngle));
		}
		
		if (keys & KEY_DOWN)
		{
			cameraX -= fixedMul(559240, fixedCos(cameraAngle));
			cameraY += fixedMul(559240, fixedSin(cameraAngle));
		}
		
		if (keys & KEY_L)
		{
			cameraX += fixedMul(559240, fixedCos(cameraAngle + 128));
			cameraY -= fixedMul(559240, fixedSin(cameraAngle + 128));
		}
		
		if (keys & KEY_R)
		{
			cameraX -= fixedMul(559240, fixedCos(cameraAngle + 128));
			cameraY += fixedMul(559240, fixedSin(cameraAngle + 128));
		}
		
		uint32_t fireWeapon = 0;
		
		fireWeaponPressed = 0;
		
		if (keys & KEY_A)
		{
			attackTics++;
			
			if (attackTics > 15)
			{
				fireWeapon = 1;
				attackTics = 0;
			}
			
			fireWeaponPressed = 1;
		}
		
		if (keys & KEY_LEFT)
			cameraAngle = (cameraAngle + 8) & ANGLESMASK;
		
		if (keys & KEY_RIGHT)
			cameraAngle = (cameraAngle - 8) & ANGLESMASK;
		
		uint32_t tx = cameraX >> 22;
		uint32_t txm = (cameraX - (9 << FRACBITS)) >> 22;
		uint32_t txp = (cameraX + (9 << FRACBITS)) >> 22;
		uint32_t ty = cameraY >> 22;
		uint32_t tym = (cameraY - (9 << FRACBITS)) >> 22;
		uint32_t typ = (cameraY + (9 << FRACBITS)) >> 22;
		
		if (cameraX - (9 << FRACBITS) < 0 || (cameraX + (9 << FRACBITS)) >> 22 >= mapWidth)
			cameraX = oldCameraX;
		else if (mapData[ty * mapWidth + txp] == 1 || mapData[ty * mapWidth + txm] == 1 || mapData[ty * mapWidth + txp] == 3 || mapData[ty * mapWidth + txm] == 3 || mapData[ty * mapWidth + txp] == 4 || mapData[ty * mapWidth + txm] == 4)
			cameraX = oldCameraX;
		else
		{
			if (mapData[typ * mapWidth + tx] == 1 || mapData[typ * mapWidth + tx] == 3 || mapData[typ * mapWidth + tx] == 4)
				cameraY = (typ << 22) - (9 << FRACBITS);
			
			if (mapData[tym * mapWidth + tx] == 1 || mapData[tym * mapWidth + tx] == 3 || mapData[tym * mapWidth + tx] == 4)
				cameraY = (tym << 22) + (73 << FRACBITS);
		}
		
		if (cameraY - (9 << FRACBITS) < 0 || (cameraY + (9 << FRACBITS)) >> 22 >= mapHeight)
			cameraY = oldCameraY;
		else if (mapData[typ * mapWidth + tx] == 1 || mapData[tym * mapWidth + tx] == 1 || mapData[typ * mapWidth + tx] == 3 || mapData[tym * mapWidth + tx] == 3 || mapData[typ * mapWidth + tx] == 4 || mapData[tym * mapWidth + tx] == 4)
			cameraY = oldCameraY;
		else
		{
			if (mapData[ty * mapWidth + txp] == 1 || mapData[ty * mapWidth + txp] == 3 || mapData[ty * mapWidth + txp] == 4)
				cameraX = (txp << 22) - (9 << FRACBITS);
			
			if (mapData[ty * mapWidth + txm] == 1 || mapData[ty * mapWidth + txm] == 3 || mapData[ty * mapWidth + txm] == 4)
				cameraX = (txm << 22) + (73 << FRACBITS);
		}
		
		int32_t mapIndex = ty * mapWidth + tx;
		
		if (mapData[mapIndex] == 5)
		{
			if (health < 100)
			{
				mapData[mapIndex] = 0;
				
				health += 10;
				
				if (health > 100)
					health = 100;
			}
		}
		else if (mapData[mapIndex] == 6)
		{
			if (health < 100)
			{
				mapData[mapIndex] = 0;
				
				health += 25;
				
				if (health > 100)
					health = 100;
			}
		}
		else if (mapData[mapIndex] == 8)
		{
			level++;
			
			if (level <= numLevels)
			{
				const uint32_t *levelData = levels[level - 1];
				int32_t cameraGridX = levelData[0];
				int32_t cameraGridY = levelData[1];
				cameraX = (cameraGridX * 64 + 32) << FRACBITS;
				cameraY = (cameraGridY * 64 + 32) << FRACBITS;
				cameraAngle = levelData[2];
				memcpy(mapData, &levelData[3], mapWidth * mapHeight * sizeof(uint32_t));
			}
			else
			{
				level = 1;
				state = 4;
			}
		}
		
		mapIndex = (ty - 1) * mapWidth + tx;
		
		if (mapData[mapIndex] == 2)
		{
			door_t *door = &doors[(((ty - 1) & 7) << 3) + (tx & 7)];
			
			if (door->mapIndex != mapIndex)
			{
				door->mapIndex = mapIndex;
				door->state = 0;
				door->offset = 64 << FRACBITS;
				door->tics = 0;
			}
			
			if (door->state == 0 || door->state == 1)
			{
				if (door->mapIndex == (ty * mapWidth + txp) || door->mapIndex == (ty * mapWidth + txm))
					cameraX = oldCameraX;
				
				if (door->mapIndex == (typ * mapWidth + tx) || door->mapIndex == (tym * mapWidth + tx))
					cameraY = oldCameraY;
			}
		}
		else if (mapData[mapIndex] == 3 || mapData[mapIndex] == 4)
		{
			enemy_t *enemy1 = &enemies[(((ty - 1) & 7) << 3) + (tx & 7)];
			
			if (enemy1->mapIndex != mapIndex)
			{
				enemy1->mapIndex = mapIndex;
				enemy1->type = mapData[mapIndex];
				enemy1->state = 1;
				enemy1->health = 100;
				enemy1->damageTics = 0;
				enemy1->attackTics = 0;
				enemy1->damage = 0;
			}
			
			if (mapData[(ty + 1) * mapWidth + tx] == 0)
			{
				mapData[(ty + 1) * mapWidth + tx] = 4;
				
				enemy_t *enemy2 = &enemies[(((ty + 1) & 7) << 3) + (tx & 7)];
				enemy2->mapIndex = (ty + 1) * mapWidth + tx;
				enemy2->type = 1;
				enemy2->state = 1;
				enemy2->health = 100;
				enemy2->damageTics = 0;
				enemy2->attackTics = 0;
				enemy2->damage = 0;
				
				if (rand() % 2 == 0)
					enemy1->state = 2;
				else
					enemy2->state = 2;
			}
			
			if (fireWeapon && enemy1->state == 2 && cameraAngle >= 64 && cameraAngle < 192)
			{
				enemy1->health -= 25;
				enemy1->damage = 1;
				
				if (enemy1->health <= 0)
				{
					enemy1->state = 0;
					enemy1->health = 0;
				}
			}
			
			if (enemy1->state == 2)
			{
				enemy1->attackTics++;
				
				if (enemy1->attackTics > 30)
				{
					health -= 10;
					
					if (health <= 0)
					{
						state = 0;
						health = 0;
					}
					
					enemy1->attackTics = 0;
				}
			}
			
			if (enemy1->damage == 1)
			{
				enemy1->damageTics++;
				
				if (enemy1->damageTics > 7)
				{
					enemy1->damageTics = 0;
					enemy1->damage = 0;
					
					if (enemy1->state == 0)
					{
						mapData[mapIndex] = 7;
						enemy1->mapIndex = -1;
						enemy1->type = 0;
						enemy1->gridX = 0;
						enemy1->gridY = 0;
						enemy_t *enemy2 = &enemies[(((ty + 1) & 7) << 3) + (tx & 7)];
						enemy2->state = 2;
					}
				}
			}
		}
		
		mapIndex = (ty + 1) * mapWidth + tx;
		
		if (mapData[mapIndex] == 2)
		{
			door_t *door = &doors[(((ty + 1) & 7) << 3) + (tx & 7)];
			
			if (door->mapIndex != mapIndex)
			{
				door->mapIndex = mapIndex;
				door->state = 0;
				door->offset = 64 << FRACBITS;
				door->tics = 0;
			}
			
			if (door->state == 0 || door->state == 1)
			{
				if (door->mapIndex == (ty * mapWidth + txp) || door->mapIndex == (ty * mapWidth + txm))
					cameraX = oldCameraX;
				
				if (door->mapIndex == (typ * mapWidth + tx) || door->mapIndex == (tym * mapWidth + tx))
					cameraY = oldCameraY;
			}
		}
		else if (mapData[mapIndex] == 3 || mapData[mapIndex] == 4)
		{
			enemy_t *enemy1 = &enemies[(((ty + 1) & 7) << 3) + (tx & 7)];
			
			if (enemy1->mapIndex != mapIndex)
			{
				enemy1->mapIndex = mapIndex;
				enemy1->type = mapData[mapIndex];
				enemy1->state = 1;
				enemy1->health = 100;
				enemy1->damageTics = 0;
				enemy1->attackTics = 0;
				enemy1->damage = 0;
			}
			
			if (mapData[(ty - 1) * mapWidth + tx] == 0)
			{
				mapData[(ty - 1) * mapWidth + tx] = 4;
				
				enemy_t *enemy2 = &enemies[(((ty - 1) & 7) << 3) + (tx & 7)];
				enemy2->mapIndex = (ty - 1) * mapWidth + tx;
				enemy2->type = 1;
				enemy2->state = 1;
				enemy2->health = 100;
				enemy2->damageTics = 0;
				enemy2->attackTics = 0;
				enemy2->damage = 0;
				
				if (rand() % 2 == 0)
					enemy1->state = 2;
				else
					enemy2->state = 2;
			}
			
			if (fireWeapon && enemy1->state == 2 && cameraAngle >= 320 && cameraAngle < 448)
			{
				enemy1->health -= 25;
				enemy1->damage = 1;
				
				if (enemy1->health <= 0)
				{
					enemy1->state = 0;
					enemy1->health = 0;
				}
			}
			
			if (enemy1->state == 2)
			{
				enemy1->attackTics++;
				
				if (enemy1->attackTics > 30)
				{
					health -= 10;
					
					if (health <= 0)
					{
						state = 0;
						health = 0;
					}
					
					enemy1->attackTics = 0;
				}
			}
			
			if (enemy1->damage == 1)
			{
				enemy1->damageTics++;
				
				if (enemy1->damageTics > 7)
				{
					enemy1->damageTics = 0;
					enemy1->damage = 0;
					
					if (enemy1->state == 0)
					{
						mapData[mapIndex] = 7;
						enemy1->mapIndex = -1;
						enemy1->type = 0;
						enemy1->gridX = 0;
						enemy1->gridY = 0;
						enemy_t *enemy2 = &enemies[(((ty - 1) & 7) << 3) + (tx & 7)];
						enemy2->state = 2;
					}
				}
			}
		}
		
		mapIndex = ty * mapWidth + (tx - 1);
		
		if (mapData[mapIndex] == 2)
		{
			door_t *door = &doors[((ty & 7) << 3) + ((tx - 1) & 7)];
			
			if (door->mapIndex != mapIndex)
			{
				door->mapIndex = mapIndex;
				door->state = 0;
				door->offset = 64 << FRACBITS;
				door->tics = 0;
			}
			
			if (door->state == 0 || door->state == 1)
			{
				if (door->mapIndex == (ty * mapWidth + txp) || door->mapIndex == (ty * mapWidth + txm))
					cameraX = oldCameraX;
				
				if (door->mapIndex == (typ * mapWidth + tx) || door->mapIndex == (tym * mapWidth + tx))
					cameraY = oldCameraY;
			}
		}
		else if (mapData[mapIndex] == 3 || mapData[mapIndex] == 4)
		{
			enemy_t *enemy1 = &enemies[((ty & 7) << 3) + ((tx - 1) & 7)];
			
			if (enemy1->mapIndex != mapIndex)
			{
				enemy1->mapIndex = mapIndex;
				enemy1->type = mapData[mapIndex];
				enemy1->state = 1;
				enemy1->health = 100;
				enemy1->damageTics = 0;
				enemy1->attackTics = 0;
				enemy1->damage = 0;
			}
			
			if (mapData[ty * mapWidth + (tx + 1)] == 0)
			{
				mapData[ty * mapWidth + (tx + 1)] = 4;
				
				enemy_t *enemy2 = &enemies[((ty & 7) << 3) + ((tx + 1) & 7)];
				enemy2->mapIndex = ty * mapWidth + (tx + 1);
				enemy2->type = 1;
				enemy2->state = 1;
				enemy2->health = 100;
				enemy2->damageTics = 0;
				enemy2->attackTics = 0;
				enemy2->damage = 0;
				
				if (rand() % 2 == 0)
					enemy1->state = 2;
				else
					enemy2->state = 2;
			}
			
			if (fireWeapon && enemy1->state == 2 && cameraAngle >= 192 && cameraAngle < 320)
			{
				enemy1->health -= 25;
				enemy1->damage = 1;
				
				if (enemy1->health <= 0)
				{
					enemy1->state = 0;
					enemy1->health = 0;
				}
			}
			
			if (enemy1->state == 2)
			{
				enemy1->attackTics++;
				
				if (enemy1->attackTics > 30)
				{
					health -= 10;
					
					if (health <= 0)
					{
						state = 0;
						health = 0;
					}

					enemy1->attackTics = 0;
				}
			}
			
			if (enemy1->damage == 1)
			{
				enemy1->damageTics++;
				
				if (enemy1->damageTics > 7)
				{
					enemy1->damageTics = 0;
					enemy1->damage = 0;
					
					if (enemy1->state == 0)
					{
						mapData[mapIndex] = 7;
						enemy1->mapIndex = -1;
						enemy1->type = 0;
						enemy1->gridX = 0;
						enemy1->gridY = 0;
						enemy_t *enemy2 = &enemies[((ty & 7) << 3) + ((tx + 1) & 7)];
						enemy2->state = 2;
					}
				}
			}
		}
		
		mapIndex = ty * mapWidth + (tx + 1);
		
		if (mapData[mapIndex] == 2)
		{
			door_t *door = &doors[((ty & 7) << 3) + ((tx + 1) & 7)];
			
			if (door->mapIndex != mapIndex)
			{
				door->mapIndex = mapIndex;
				door->state = 0;
				door->offset = 64 << FRACBITS;
				door->tics = 0;
			}
			
			if (door->state == 0 || door->state == 1)
			{
				if (door->mapIndex == (ty * mapWidth + txp) || door->mapIndex == (ty * mapWidth + txm))
					cameraX = oldCameraX;
				
				if (door->mapIndex == (typ * mapWidth + tx) || door->mapIndex == (tym * mapWidth + tx))
					cameraY = oldCameraY;
			}
		}
		else if (mapData[mapIndex] == 3 || mapData[mapIndex] == 4)
		{
			enemy_t *enemy1 = &enemies[((ty & 7) << 3) + ((tx + 1) & 7)];
			
			if (enemy1->mapIndex != mapIndex)
			{
				enemy1->mapIndex = mapIndex;
				enemy1->type = mapData[mapIndex];
				enemy1->state = 1;
				enemy1->health = 100;
				enemy1->damageTics = 0;
				enemy1->attackTics = 0;
				enemy1->damage = 0;
			}
			
			if (mapData[ty * mapWidth + (tx - 1)] == 0)
			{
				mapData[ty * mapWidth + (tx - 1)] = 4;
				
				enemy_t *enemy2 = &enemies[((ty & 7) << 3) + ((tx - 1) & 7)];
				enemy2->mapIndex = ty * mapWidth + (tx - 1);
				enemy2->type = 1;
				enemy2->state = 1;
				enemy2->health = 100;
				enemy2->damageTics = 0;
				enemy2->attackTics = 0;
				enemy2->damage = 0;
				
				if (rand() % 2 == 0)
					enemy1->state = 2;
				else
					enemy2->state = 2;
			}
			
			if (fireWeapon && enemy1->state == 2 && (cameraAngle >= 448 || cameraAngle < 64))
			{
				enemy1->health -= 25;
				enemy1->damage = 1;
				
				if (enemy1->health <= 0)
				{
					enemy1->state = 0;
					enemy1->health = 0;
				}
			}
			
			if (enemy1->state == 2)
			{
				enemy1->attackTics++;
				
				if (enemy1->attackTics > 30)
				{
					health -= 10;
					
					if (health <= 0)
					{
						state = 0;
						health = 0;
					}
					
					enemy1->attackTics = 0;
				}
			}
			
			if (enemy1->damage == 1)
			{
				enemy1->damageTics++;
				
				if (enemy1->damageTics > 7)
				{
					enemy1->damageTics = 0;
					enemy1->damage = 0;
					
					if (enemy1->state == 0)
					{
						mapData[mapIndex] = 7;
						enemy1->mapIndex = -1;
						enemy1->type = 0;
						enemy1->gridX = 0;
						enemy1->gridY = 0;
						enemy_t *enemy2 = &enemies[((ty & 7) << 3) + ((tx - 1) & 7)];
						enemy2->state = 2;
					}
				}
			}
		}
		
		for (int32_t i = 0; i < 64; i++)
		{
			door_t *door = &doors[i];
			
			if (door->mapIndex != -1)
			{
				if (door->state == 0)
				{
					door->tics++;
					
					if (door->tics == 30)
					{
						door->state = 1;
						door->tics = 0;
					}
				}
				else if (door->state == 1)
				{
					door->offset -= 279620;
					
					if (door->offset < 0)
					{
						door->state = 2;
						door->offset = 0;
					}
				}
				else if (door->state == 2)
				{
					if (door->mapIndex != (ty * mapWidth + tx))
					{
						door->tics++;
						
						if (door->tics == 30)
						{
							door->state = 3;
							door->tics = 0;
						}
					}
				}
				else if (door->state == 3)
				{
					door->offset += 279620;
					
					if (door->offset > (64 << FRACBITS))
					{
						door->mapIndex = -1;
						door->state = 0;
						door->offset = 64 << FRACBITS;
					}
				}
			}
		}
		
		frameTics++;
		
		if (frameTics > 7)
		{
			frame = !frame;
			frameTics = 0;
		}
	}
	else if (state == 0)
	{
		bloodTics++;
		
		if (bloodTics > 3)
		{
			uint32_t area = 0;
			
			for (uint32_t i = 0; i < 120; i++)
			{
				bloodHeight[i] += bloodSpeed[i];
				
				if (bloodHeight[i] > 64)
					bloodHeight[i] = 64;

				area += bloodHeight[i];
			}
			
			if (area == 7680)
				state = 3;

			bloodTics = 0;
		}
	}
	else if (state == 2 || state == 3 || state == 4 || state == 5)
	{
		if (state < 4 && restartLevelPressed)
		{
			const uint32_t *levelData = levels[level - 1];
			int32_t cameraGridX = levelData[0];
			int32_t cameraGridY = levelData[1];
			cameraX = (cameraGridX * 64 + 32) << FRACBITS;
			cameraY = (cameraGridY * 64 + 32) << FRACBITS;
			cameraAngle = levelData[2];
			memcpy(mapData, &levelData[3], mapWidth * mapHeight * sizeof(uint32_t));
			state = 1;
			health = 100;
			
			for (uint32_t i = 0; i < 120; i++)
				bloodHeight[i] = 0;
		}
		else if (state == 4 && restartLevelPressed)
			state = 5;
		else if (restartLevelPressed)
			state = 2;
	}
}

void IWRAM_CODE Render()
{
	if (state == 1 || state == 0)
	{
		plane.minX = 120;
		plane.maxX = -1;
		
		plane.pad1 = 64;
		
		for (int32_t i = 0; i < 120; i++)
			plane.top[i] = 64;
		
		plane.pad2 = 64;
		
		angle_t rayAngle = (cameraAngle + 59) & ANGLESMASK;
		
		for (int32_t i = 0; i < 120; i++)
		{
			fixed_t horizontalIntersectionY;
			fixed_t stepY;
			
			if (rayAngle < 256)
			{
				horizontalIntersectionY = (cameraY >> 22) * (64 << FRACBITS);
				stepY = -64 << FRACBITS;
			}
			else
			{
				horizontalIntersectionY = (cameraY >> 22) * (64 << FRACBITS) + (64 << FRACBITS);
				stepY = 64 << FRACBITS;
			}
			
			fixed_t horizontalIntersectionX = cameraX - fixedMul(horizontalIntersectionY - cameraY, fixedCot(rayAngle));
			fixed_t stepX = -fixedMul(stepY, fixedCot(rayAngle));
			fixed_t horizontalIntersectionDistance;
			uint32_t horizontalIntersectionType;
			int32_t horizontalDoorOffset;
			
			if (rayAngle == 0 || rayAngle == 256)
				horizontalIntersectionDistance = INT_MAX;
			else
			{
				while (1)
				{
					int32_t gridX = horizontalIntersectionX >> 22;
					int32_t gridY = (horizontalIntersectionY >> 22) - (stepY < 0 ? 1 : 0);
					
					if (gridX < 0 || gridY < 0 || gridX >= mapWidth || gridY >= mapHeight)
					{
						horizontalIntersectionDistance = INT_MAX;
						break;
					}
					
					horizontalIntersectionType = mapData[gridY * mapWidth + gridX];
					
					if (horizontalIntersectionType == 1)
					{
						horizontalIntersectionDistance = fixedMul(horizontalIntersectionX - cameraX, fixedCos(cameraAngle)) - fixedMul(horizontalIntersectionY - cameraY, fixedSin(cameraAngle));
						break;
					}
					else if (horizontalIntersectionType == 2 && (((horizontalIntersectionX + (stepX >> 1)) >> FRACBITS) & 63) < (horizontalDoorOffset = (doors[((gridY & 7) << 3) + (gridX & 7)].mapIndex == (gridY * mapWidth + gridX) ? doors[((gridY & 7) << 3) + (gridX & 7)].offset >> FRACBITS : 64)))
					{
						horizontalIntersectionX += stepX >> 1;
						horizontalIntersectionY += stepY >> 1;
						horizontalIntersectionDistance = fixedMul(horizontalIntersectionX - cameraX, fixedCos(cameraAngle)) - fixedMul(horizontalIntersectionY - cameraY, fixedSin(cameraAngle));
						break;
					}
					else if (horizontalIntersectionType == 3 || horizontalIntersectionType == 4)
					{
						enemy_t *enemy = &enemies[((gridY & 7) << 3) + (gridX & 7)];
						enemy->type = horizontalIntersectionType - 3;
						enemy->gridX = gridX;
						enemy->gridY = gridY;
						enemy->render = 1;
					}
					else if (horizontalIntersectionType == 5 || horizontalIntersectionType == 6)
					{
						health_t *health = &healths[((gridY & 7) << 3) + (gridX & 7)];
						health->type = horizontalIntersectionType - 5;
						health->gridX = gridX;
						health->gridY = gridY;
						health->render = 1;
					}
					
					horizontalIntersectionX += stepX;
					horizontalIntersectionY += stepY;
				}
			}
			
			fixed_t verticalIntersectionX;
			
			if (rayAngle >= 128 && rayAngle < 384)
			{
				verticalIntersectionX = (cameraX >> 22) * (64 << FRACBITS);
				stepX = -64 << FRACBITS;
			}
			else
			{
				verticalIntersectionX = (cameraX >> 22) * (64 << FRACBITS) + (64 << FRACBITS);
				stepX = 64 << FRACBITS;
			}
			
			fixed_t verticalIntersectionY = cameraY - fixedMul(verticalIntersectionX - cameraX, fixedTan(rayAngle));
			stepY = -fixedMul(stepX, fixedTan(rayAngle));
			fixed_t verticalIntersectionDistance;
			uint32_t verticalIntersectionType;
			int32_t verticalDoorOffset;
			
			if (rayAngle == 128 || rayAngle == 384)
				verticalIntersectionDistance = INT_MAX;
			else
			{
				while (1)
				{
					int32_t gridX = (verticalIntersectionX >> 22) - (stepX < 0 ? 1 : 0);
					int32_t gridY = verticalIntersectionY >> 22;
					
					if (gridX < 0 || gridY < 0 || gridX >= mapWidth || gridY >= mapHeight)
					{
						verticalIntersectionDistance = INT_MAX;
						break;
					}
					
					verticalIntersectionType = mapData[gridY * mapWidth + gridX];
					
					if (verticalIntersectionType == 1)
					{
						verticalIntersectionDistance = fixedMul(verticalIntersectionX - cameraX, fixedCos(cameraAngle)) - fixedMul((verticalIntersectionY - cameraY), fixedSin(cameraAngle));
						break;
					}
					else if (verticalIntersectionType == 2 && (((verticalIntersectionY + (stepY >> 1)) >> FRACBITS) & 63) < (verticalDoorOffset = (doors[((gridY & 7) << 3) + (gridX & 7)].mapIndex == (gridY * mapWidth + gridX) ? doors[((gridY & 7) << 3) + (gridX & 7)].offset >> FRACBITS : 64)))
					{
						verticalIntersectionX += stepX >> 1;
						verticalIntersectionY += stepY >> 1;
						verticalIntersectionDistance = fixedMul(verticalIntersectionX - cameraX, fixedCos(cameraAngle)) - fixedMul((verticalIntersectionY - cameraY), fixedSin(cameraAngle));
						break;
					}
					else if (verticalIntersectionType == 3 || verticalIntersectionType == 4)
					{
						enemy_t *enemy = &enemies[((gridY & 7) << 3) + (gridX & 7)];
						enemy->type = verticalIntersectionType - 3;
						enemy->gridX = gridX;
						enemy->gridY = gridY;
						enemy->render = 1;
					}
					else if (verticalIntersectionType == 5 || verticalIntersectionType == 6)
					{
						health_t *health = &healths[((gridY & 7) << 3) + (gridX & 7)];
						health->type = verticalIntersectionType - 5;
						health->gridX = gridX;
						health->gridY = gridY;
						health->render = 1;
					}
					
					verticalIntersectionX += stepX;
					verticalIntersectionY += stepY;
				}
			}
			
			fixed_t distance;
			const uint8_t *texture;
			int32_t textureOffsetX;
			
			if (horizontalIntersectionDistance < verticalIntersectionDistance)
			{
				distance = horizontalIntersectionDistance;
				texture = &graphicsBitmap[0];
				textureOffsetX = (horizontalIntersectionX >> FRACBITS) & 63;
				
				if (horizontalIntersectionType == 2)
				{
					texture = &graphicsBitmap[8192];
					textureOffsetX += 64 - horizontalDoorOffset;
				}
				
				if (horizontalIntersectionType != 2 && rayAngle >= 256)
					textureOffsetX = 63 - textureOffsetX;
			}
			else
			{
				distance = verticalIntersectionDistance;
				texture = &graphicsBitmap[12288];
				textureOffsetX = (verticalIntersectionY >> FRACBITS) & 63;
				
				if (verticalIntersectionType == 2)
				{
					texture = &graphicsBitmap[4096];
					textureOffsetX += 64 - verticalDoorOffset;
				}
				
				if (verticalIntersectionType != 2 && rayAngle >= 128 && rayAngle < 384)
					textureOffsetX = 63 - textureOffsetX;
			}
			
			int32_t wallHeight = FindHeight(distance);
			int32_t wallStart = (64 - wallHeight) >> 1;
			
			if (solidPlanes && wallHeight < 64)
			{
				uint16_t *p = yTable[page][0] + xTable[i];
				
				int32_t count = wallStart * 2 - 1;
				
				do
				{
					*p = 0x00;
					p += SCREEN_WIDTH >> 1;
				} while (count--);
			}
			
			DrawWallSlice(texture, textureOffsetX, i, wallStart, wallHeight);
			
			if (solidPlanes && wallHeight < 64)
			{
				uint16_t *p = yTable[page][wallStart + wallHeight] + xTable[i];
				
				int32_t count = wallStart * 2 - 1;
				
				do
				{
					*p = 0x00;
					p += SCREEN_WIDTH >> 1;
				} while (count--);
			}
			else if (wallHeight < 64)
			{
				if (i < plane.minX)
					plane.minX = i;
				
				if (i > plane.maxX)
					plane.maxX = i;
				
				plane.top[i] = wallStart + wallHeight;
			}
			
			zBuffer[i] = distance;
			
			rayAngle = (rayAngle - 1) & ANGLESMASK;
		}
		
		if (!solidPlanes)
		{
			const uint8_t *floorTexture = &graphicsBitmap[16384];
			const uint8_t *ceilingTexture = &graphicsBitmap[20480];

			for (int32_t i = 0; i < 32; i++)
				stop[i] = 0;
			
			for (int32_t x = plane.minX; x <= plane.maxX + 1; x++)
			{
				uint32_t t1 = plane.top[x - 1];
				uint32_t t2 = plane.top[x];
				
				while (t1 < t2)
				{
					uint32_t index = t1 - 32;
					
					if (stop[index] == 0)
					{
						fixed_t distance = fixedMul(planeDistanceTable[index], fovInvCos);
						fixed_t x1 = fixedMul(distance, fixedCos((cameraAngle + 63) & ANGLESMASK));
						fixed_t y1 = -fixedMul(distance, fixedSin((cameraAngle + 63) & ANGLESMASK));
						fixed_t x2 = fixedMul(distance, fixedCos((cameraAngle - 64) & ANGLESMASK));
						fixed_t y2 = -fixedMul(distance, fixedSin((cameraAngle - 64) & ANGLESMASK));
						currentX[index] = cameraX + x1;
						currentY[index] = cameraY + y1;
						stepX[index] = fixedMul(x2 - x1, invViewWidth);
						stepY[index] = fixedMul(y2 - y1, invViewWidth);
						currentX[index] += (start[index] + 4) * stepX[index];
						currentY[index] += (start[index] + 4) * stepY[index];
					}
					else
					{
						currentX[index] += (start[index] - stop[index]) * stepX[index];
						currentY[index] += (start[index] - stop[index]) * stepY[index];
					}
					
					uint32_t count = (x - 1) - start[index];
					uint16_t *p1 = yTable[page][t1] + xTable[start[index]];
					uint16_t *p2 = yTable[page][63 - t1] + xTable[start[index]];
					
					do
					{
						int32_t tx = (currentX[index] >> FRACBITS) & 63;
						int32_t ty = (currentY[index] >> FRACBITS) & 63;
						int32_t textureIndex = ty * 64 + tx;
						int32_t color = floorTexture[textureIndex];
						*p1 = color << 8 | color;
						*(p1 + (SCREEN_WIDTH >> 1)) = color << 8 | color;
						p1++;
						color = ceilingTexture[textureIndex];
						*p2 = color << 8 | color;
						*(p2 + (SCREEN_WIDTH >> 1)) = color << 8 | color;
						p2++;
						currentX[index] += stepX[index];
						currentY[index] += stepY[index];
					} while (count--);
					
					stop[index] = x;
					
					t1++;
				}
				
				while (t2 < t1)
				{
					start[t2 - 32] = x;
					t2++;
				}
			}
		}
		
		for (int32_t i = 0; i < 64; i++)
		{
			health_t *health = &healths[i];
			
			if (health->render)
			{
				fixed_t distance = fixedMul(((health->gridX << 22) + (32 << FRACBITS)) - cameraX, fixedCos(cameraAngle)) - fixedMul(((health->gridY << 22) + (32 << FRACBITS)) - cameraY, fixedSin(cameraAngle));
				fixed_t x = fixedMul(((health->gridX << 22) + (32 << FRACBITS)) - cameraX, fixedSin(cameraAngle)) + fixedMul(((health->gridY << 22) + (32 << FRACBITS)) - cameraY, fixedCos(cameraAngle));
				int32_t spriteSize = FindHeight(distance);
				x = fixedMul(x, spriteSize << FRACBITS) >> 6;
				int32_t spriteX = 60 + (x >> FRACBITS) - (spriteSize >> 1);
				int32_t spriteY = (64 - spriteSize) >> 1;
				const uint8_t *sprite = &graphicsBitmap[frames[4 + health->type]];
				DrawSprite(sprite, spriteX, spriteY, spriteSize, distance, 0);
				health->render = 0;
			}
		}
		
		for (int32_t i = 0; i < 64; i++)
		{
			enemy_t *enemy = &enemies[i];
			
			if (enemy->render)
			{
				fixed_t distance = fixedMul(((enemy->gridX << 22) + (32 << FRACBITS)) - cameraX, fixedCos(cameraAngle)) - fixedMul(((enemy->gridY << 22) + (32 << FRACBITS)) - cameraY, fixedSin(cameraAngle));
				fixed_t x = fixedMul(((enemy->gridX << 22) + (32 << FRACBITS)) - cameraX, fixedSin(cameraAngle)) + fixedMul(((enemy->gridY << 22) + (32 << FRACBITS)) - cameraY, fixedCos(cameraAngle));
				int32_t spriteSize = FindHeight(distance);
				x = fixedMul(x, spriteSize << FRACBITS) >> 6;
				int32_t spriteX = 60 + (x >> FRACBITS) - (spriteSize >> 1);
				int32_t spriteY = (64 - spriteSize) >> 1;
				const uint8_t *sprite = &graphicsBitmap[frames[enemy->type * 2 + frame]];
				DrawSprite(sprite, spriteX, spriteY, spriteSize, distance, enemy->damage);
				enemy->render = 0;
			}
		}
		
		const uint8_t *hand = &graphicsBitmap[49152];
		
		if (fireWeaponPressed)
			DrawGraphic(hand, 25, 0, 95, 38, 25, 26);
		else
			DrawGraphic(hand, 0, 0, 95, 38, 25, 26);
		
		if (health > 0)
			DrawRect(28, 60, healthBarTable[health - 1], 2, 0x2A);
		
		if (state == 0)
		{
			for (uint32_t i = 0; i < 120; i++)
			{
				if (bloodHeight[i] > 0)
					DrawRect(i, 0, 1, bloodHeight[i], 0x2A);
			}
		}
	}
	else if (state == 2)
	{
		DrawRect(0, 0, 28, 64, 0x00);
		const uint8_t *title = &graphicsBitmap[50816];
		DrawGraphic(title, 0, 0, 28, 0, 64, 64);
		DrawRect(92, 0, 28, 64, 0x00);
	}
	else if (state == 3)
	{
		DrawRect(0, 0, 28, 64, 0x2A);
		const uint8_t *dead = &graphicsBitmap[54912];
		DrawGraphic(dead, 0, 0, 28, 0, 64, 64);
		DrawRect(92, 0, 28, 64, 0x2A);
	}
	else if (state == 4)
	{
		DrawRect(0, 0, 28, 64, 0x00);
		const uint8_t *end = &graphicsBitmap[59008];
		DrawGraphic(end, 0, 0, 28, 0, 64, 64);
		DrawRect(92, 0, 28, 64, 0x00);
	}
	else if (state == 5)
	{
		DrawRect(0, 0, 28, 64, 0x00);
		const uint8_t *credits = &graphicsBitmap[63104];
		DrawGraphic(credits, 0, 0, 28, 0, 64, 64);
		DrawRect(92, 0, 28, 64, 0x00);
	}
}

uint32_t count = 0;

void vblankInterrupt()
{
	count++;
}

int main(void)
{
	irqInit();
	irqSet(IRQ_VBLANK, vblankInterrupt);
	irqEnable(IRQ_VBLANK);
	
	REG_IME = 1;
	
	SetMode(MODE_4 | BG2_ON);
	
	//BG_COLORS[1] = RGB8(255, 0, 0);
	//BG_COLORS[2] = RGB8(0, 255, 0);
	//BG_COLORS[3] = RGB8(0, 0, 255);
	
	memcpy(BG_COLORS, graphicsPal, graphicsPalLen);
	
	uint16_t *vid_mem_front = (uint16_t *) (VRAM);
	uint16_t *vid_mem_back = (uint16_t *) (VRAM | 0xA000);
	
	for (uint32_t i = 0; i < 64; i++)
	{
		yTable[0][i] = (uint16_t *) &vid_mem_front[(((SCREEN_HEIGHT - 128) >> 1) + 2 * i) * (SCREEN_WIDTH >> 1)];
		yTable[1][i] = (uint16_t *) &vid_mem_back[(((SCREEN_HEIGHT - 128) >> 1) + 2 * i) * (SCREEN_WIDTH >> 1)];
	}
	
	for (uint32_t i = 0; i < 120; i++)
		xTable[i] = (((SCREEN_WIDTH >> 1) - 120) >> 1) + i;
	
	srand((unsigned)time(NULL));
	
	for (uint32_t i = 0; i < 120; i++)
		bloodSpeed[i] = rand() % 4 + 2;
	
	while (1)
	{
		scanKeys();
		Update();
		Render();
		VBlankIntrWait();
		page = !page;
		REG_DISPCNT ^= BACKBUFFER;
		//count = count & 3;
		//vid_mem[(144 * SCREEN_WIDTH + 120) / 2] = count << 8 | count;
		//vid_mem[(145 * SCREEN_WIDTH + 120) / 2] = count << 8 | count;
		count = 0;
	}
}