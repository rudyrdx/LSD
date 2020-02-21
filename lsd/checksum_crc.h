//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Generic CRC functions
//
// $NoKeywords: $
//=============================================================================//
#include <windows.h>
#include <stdio.h>
#ifndef CHECKSUM_CRC_H
#define CHECKSUM_CRC_H
#ifdef _WIN32
#pragma once
#endif

typedef unsigned int CRC32_t;

void CRC32_Init(CRC32_t *pulCRC);
void CRC32_ProcessBuffer(CRC32_t *pulCRC, const void *p, int len);
void CRC32_Final(CRC32_t *pulCRC);
CRC32_t	CRC32_GetTableEntry(unsigned int slot);

inline CRC32_t CRC32_ProcessSingleBuffer(const void *p, int len)
{
	CRC32_t crc;

	CRC32_Init(&crc);
	CRC32_ProcessBuffer(&crc, p, len);
	CRC32_Final(&crc);

	return crc;
}

#endif // CHECKSUM_CRC_H
// The four core functions - F1 is optimized somewhat
// #define F1(x, y, z) (x & y | ~x & z)
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

// This is the central step in the MD5 algorithm.
#define MD5STEP(f, w, x, y, z, data, s) \
        ( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )

//-----------------------------------------------------------------------------
// Purpose: The core of the MD5 algorithm, this alters an existing MD5 hash to
//  reflect the addition of 16 longwords of new data.  MD5Update blocks
//  the data and converts bytes into longwords for this routine.
// Input  : buf[4] - 
//			in[16] - 
// Output : static void
//-----------------------------------------------------------------------------
// 16 bytes == 128 bit digest
#define MD5_DIGEST_LENGTH 16

// MD5 Hash
typedef struct
{
	unsigned int buf[4];
	unsigned int bits[2];
	unsigned char in[64];
} MD5Context_t;

void MD5Init(MD5Context_t* context);
void MD5Update(MD5Context_t* context, unsigned char const* buf, unsigned int len);
void MD5Final(unsigned char digest[MD5_DIGEST_LENGTH], MD5Context_t* context);

char* MD5_Print(unsigned char* digest, int hashlen);

unsigned int MD5_PseudoRandom(unsigned int nSeed);

