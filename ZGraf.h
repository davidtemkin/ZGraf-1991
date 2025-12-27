/*
 * ZGraf - ZGraf.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#define _H_ZGraf

/* Datatypes */

typedef long ZPat[32];
typedef ZPat ZPatTable[32];

typedef struct {
	long *baseAddr;			/* Starting address of bitmap				*/
	short rowBytes;			/* How many bytes across bitmap		 		*/
	short xBound, yBound;	/* Bitmap's boundary coordinates (absolute)	*/
	long *((*theLines)[]);	/* Pointer to array of pointers to lines in	*/
							/* 	bitmap									*/
	Rect clipRect;			/* Clipping rectange -- ABSOLUTE coords		*/
							/* 	these are boundary lines				*/
	short xOrig, yOrig;		/* Corresponds to absolute coords (0,0)		*/
	GrafPort qdPort;		/* Maintained in sync with above structure	*/
							/* 	so QuickDraw calls can be intermixed	*/
							/* 	with ZGraf calls						*/
	short qdRowBytes;		/* This is QuickDraw�s rowBytes field, 		*/
							/* 	including the flags						*/
	short bitDepth;			/* How many bits per pixel?					*/
	long *maskPtr;			/* Pointer to color mask table (2-bit only)	*/
	long fullMask;			/* Full mask from mask table (2-bit only) 	*/
	} ZBitMap;

/* Constants */

#define PAGEDIFF 0x8000
#ifndef NIL
#define NIL ((void *) 0L)
#endif

/* System global */

extern Byte addrMode : 0xCB2;

/* Application globals */

extern ZBitMap *zCurBitMap;
extern ZBitMap *zPage1BitMap, *zPage2BitMap;
extern void *zMaskTable;
extern long zMaskLongs[32][32];
extern ZPatTable *zShades;
extern ZPat *zCurPat;
extern ZPat *zBlack, *zDk4, *zDk3, *zDk2, *zDk1, *zMed;
extern ZPat *zLt1, *zLt2, *zLt3, *zLt4, *zWhite;
extern Pattern qdShades[11];
extern ZPatTable (*zDitherShades)[8];	// Pointer to 8 sets of 32 patterns each
extern short zCurDitherIndex;			// Index into zDitherShades array,
										//   which points to
extern ZPatTable *zCurDitherTable;		//   current set of 32 patterns

/* Function prototypes */

extern void ZInitGraf(void);
extern void ZInitBitMap(ZBitMap *theBitMap);
extern ZBitMap *ZDupBitMap(ZBitMap *theBitMap);
extern ZBitMap *ZNewBitMap(short xBound, short yBound);
extern void ZFreeBitMap(ZBitMap *bitMap);

extern void ZSetBit(long *longPtr, short bitNo);
extern void ZSetRect(Rect *r,
	short left, short top, short right, short bottom);
extern void ZOffsetRect(Rect *r, short x, short y);

extern void ZSetOrigin(short xOrig, short yOrig);
extern void ZSetAbsClipRect(Rect *clipRect);
extern void ZSetRelClipRect(Rect *clipRect);
extern void ZSetShade(short shade);
extern void ZSetBitMap(ZBitMap *bitMap);

extern short ZGetPixel(short x, short y);
extern void	ZSetPixel(short x, short y, short value);

extern void ZHLine(short left, short right, short y);
extern void ZVLine(short top, short bottom, short x);
extern void ZVSLine(short top, short bottom, short x,
	short value, short skipFactor);
extern void ZFrameRect(Rect *r);
extern void ZFillRect(Rect *r);

extern void ZCopyLongs(ZBitMap *srcBits, ZBitMap *dstBits,
	Rect *srcRectAbs, short xDestAbs, short yDestAbs);
extern void ZClearLongs(Rect *rectAbs);

/* Macros */

#define SHADE(s) zCurPat = s
#define BITS(b) zCurBitMap = b
#define DITHERSHADE(n) zCurPat = &(*zCurDitherTable)[n];

#define ZSETRECT(rect, x1, y1, x2, y2) \
	{ rect.left = x1; rect.top = y1; \
	  rect.right = x2; rect.bottom = y2; }

#define ZNewPtr(size)			((Ptr) NewPtrClear(size))
#define ZDisposePtr(p)			DisposePtr(p)
