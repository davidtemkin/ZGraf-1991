/*
 * ZGraf - CGraf.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "ZGraf.h"

#define _H_CGraf

// Datatypes

typedef long CMaskTable[16][16];

// Application globals

extern RGBColor rgbBlack, rgbGreen, rgbRed, rgbAmber; 
extern CMaskTable *cGreenMasks, *cRedMasks, *cAmberMasks;
extern Boolean fakeColorDisp;

// Constants

enum {
	kRed = 128,
	kGreen,
	kAmber
	};
	
enum {
	kMonochrome = 128,
	kColor,
	kFakeColor
	};

// Function prototypes

extern void CInitGraf(Boolean colorMonitor);

extern void CPrepareDevice(void);
extern void CCleanUpDevice(long inhibitExceptions);

extern void CInitBitMap(ZBitMap *theBitMap);
extern ZBitMap *CNewBitMap(short xBound, short yBound);
extern ZBitMap *CDupBitMap(ZBitMap *srcBitMap);
extern void CFreeBitMap(ZBitMap *bitMap);

extern void CSetColor(short color);

extern short CGetPixel(short x, short y);
extern void	CSetPixel(short x, short y, short value);

extern void CHLine(short left, short right, short y);
extern void CVLine(short top, short bottom, short x);
extern void CVSLine(short top, short bottom, short x,
	short value, short skipFactor);
extern void CFrameRect(Rect *r);
extern void CFillRect(Rect *r);

extern void CCopyLongs(ZBitMap *srcBits, ZBitMap *dstBits,
	Rect *srcRectAbs, short xDestAbs, short yDestAbs);
extern void CClearLongs(Rect *rectAbs);
