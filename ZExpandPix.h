/*
 * ZGraf - ZExpandPix.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#define _H_ZExpandPix

#ifndef _H_ZGraf
	#include "ZGraf.h"
#endif

extern long ((*zFourBitTable)[]);	/* Pointers to arrays of longs */
extern long ((*zEightBitTable)[]);

extern void ZInitFourBitTable(void);	/* Allocates and initializes table */
extern void ZInitEightBitTable(void);

extern void ZExpand2To4(ZBitMap *srcBits, ZBitMap *dstBits,
	Rect *srcRectAbs, short xDestAbs, short yDestAbs);
extern void ZExpand2To8(ZBitMap *srcBits, ZBitMap *dstBits,
	Rect *srcRectAbs, short xDestAbs, short yDestAbs);
