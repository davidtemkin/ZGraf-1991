/*
 * ZGraf - ZGraf.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include "ZConfig.h"
#include "ZGraf.h"

ZBitMap *zCurBitMap;
ZBitMap *zPage1BitMap;
ZBitMap *zPage2BitMap;

long zMaskLongs[32][32];
Pattern qdShades[11];
ZPatTable *zShades;
ZPat *zCurPat;
ZPat *zBlack, *zDk4, *zDk3, *zDk2, *zDk1, *zMed,
	*zLt1, *zLt2, *zLt3, *zLt4, *zWhite;
ZPatTable (*zDitherShades)[8];
short zCurDitherIndex;
ZPatTable *zCurDitherTable;

void ZInitGraf ()
{
	short begin;
	short end;
	short bit;
	short i;
	long theMask;
	Handle dithPats;

	// GetConfig() must have been called previously

	zShades = nil;
	dithPats = nil;
	zPage1BitMap = zPage2BitMap = nil;

	for (begin = 0; begin < 32; begin++) {
		for (end = 0; end < 32; end++) {
			zMaskLongs[begin][end] = 0L;
			for (bit = begin; bit <= end; bit++)
				ZSetBit(&zMaskLongs[begin][end], bit);
			}
		}
	
	FailNIL(zShades = (ZPatTable *) ZNewPtr(sizeof(ZPatTable)));
	
	for (i=0; i<11; i++) {
		unsigned char *p;
		unsigned char b;
		short line;

		GetIndPattern((void *) &qdShades[i], 2000, i+1);
		FailResError();
		
		for (line = 0; line < 8; line++) {
			p = (unsigned char *) &(*zShades)[i][line];
			b = qdShades[i][line];
			*p = b; *(p+1) = b; *(p+2) = b; *(p+3) = b;
			(*zShades)[i][line+8] = (*zShades)[i][line];
			(*zShades)[i][line+16] = (*zShades)[i][line];
			(*zShades)[i][line+24] = (*zShades)[i][line];
			}
		}

	zBlack = &(*zShades)[0];
	zDk4 = &(*zShades)[1];
	zDk3 = &(*zShades)[2];
	zDk2 = &(*zShades)[3];
	zDk1 = &(*zShades)[4];
	zMed = &(*zShades)[5];
	zLt1 = &(*zShades)[6];
	zLt2 = &(*zShades)[7];
	zLt3 = &(*zShades)[8];
	zLt4 = &(*zShades)[9];
	zWhite = &(*zShades)[10];

	// Now load in dithered patterns. Set current table to index 0.

	FailNILRes(dithPats = GetResource('dith', 128));
	DetachResource(dithPats);
	HLock(dithPats);
	zDitherShades = (ZPatTable (*)[8]) *dithPats;
	zCurDitherIndex = 0;
	zCurDitherTable = &(*zDitherShades)[zCurDitherIndex];

	// Uh, yeah...

	FailNIL(zPage1BitMap = (ZBitMap *) ZNewPtr(sizeof(ZBitMap)));

	if (gConfig.hasColorQD) {
		PixMapHandle pmap;
		GDHandle gdev;

		gdev = gConfig.gdev;
		pmap = gConfig.pmap;
		
		/* Might want to use GetPixBaseAddr -- test for 32 bit CQD */
		
		zPage1BitMap->baseAddr = (long *) (**pmap).baseAddr;
		zPage1BitMap->rowBytes = (**pmap).rowBytes & 0x3FFF;
		zPage1BitMap->xBound = (**pmap).bounds.right - (**pmap).bounds.left;
		zPage1BitMap->yBound = (**pmap).bounds.bottom - (**pmap).bounds.top;
		zPage1BitMap->qdRowBytes = (**pmap).rowBytes;
		ZInitBitMap(zPage1BitMap);
	
		videoParam.ioCompletion = NIL;
		videoParam.ioCRefNum = (**gdev).gdRefNum;
		*(void **)(&videoParam.csParam) = &modeRecord;
		videoParam.csCode = cscGetMode;	
		FailOSErr(PBStatus((ParmBlkPtr) &videoParam, 0));

		videoParam.csCode = cscGetPageCnt;
		FailOSErr(PBStatus((ParmBlkPtr) &videoParam, 0));
		
		if (modeRecord.csPage > 1) {
			FailNIL(zPage2BitMap = (ZBitMap *) ZNewPtr(sizeof(ZBitMap)));
			modeRecord.csPage = 1;
			videoParam.csCode = cscGetBaseAddr;
			
			// FIX: exceptions
			
			FailOSErr(PBStatus((ParmBlkPtr) &videoParam, 0));
			zPage2BitMap->baseAddr = (long *) modeRecord.csBaseAddr;
			zPage2BitMap->rowBytes = zPage1BitMap->rowBytes;
			zPage2BitMap->xBound = zPage1BitMap->xBound;
			zPage2BitMap->yBound = zPage1BitMap->yBound;
			zPage2BitMap->qdRowBytes = zPage1BitMap->rowBytes;
			ZInitBitMap(zPage2BitMap);
			}
		else
			zPage2BitMap = NIL;
		}
	else {
		zPage1BitMap->baseAddr = (long *) qd.screenBits.baseAddr;
		zPage1BitMap->rowBytes = qd.screenBits.rowBytes;
		zPage1BitMap->xBound = qd.screenBits.bounds.right;
		zPage1BitMap->yBound = qd.screenBits.bounds.bottom;
		ZInitBitMap(zPage1BitMap);
		if (gConfig.useOrigAltScreen) {
			zPage2BitMap = (ZBitMap *) ZNewPtr(sizeof(ZBitMap));
			zPage2BitMap->baseAddr =
				(long *) ((char *) zPage1BitMap->baseAddr - PAGEDIFF);
			zPage2BitMap->rowBytes = qd.screenBits.rowBytes;
			zPage2BitMap->xBound = qd.screenBits.bounds.right;
			zPage2BitMap->yBound = qd.screenBits.bounds.bottom;
			ZInitBitMap(zPage2BitMap);
			}
		else
			zPage2BitMap = NIL;
		}

	ZSetBitMap(zPage1BitMap);
	ZSetShade(10);	
}

void ZInitBitMap (
	ZBitMap *theBitMap )
{
	GrafPort *holdPort;
	BitMap qdbm;			/* QuickDraw shadow bitmap - no color QuickDraw */
	PixMapHandle qdpm;		/* Handle to QuickDraw shadow PixMap - if on CQD */
	long lineNo;
	long lineAddr;
	long needMem;

	theBitMap->theLines = nil;

	ZSetRect(&theBitMap->clipRect,
		0, 0, theBitMap->xBound, theBitMap->yBound);
	theBitMap->xOrig = theBitMap->yOrig = 0;

	needMem = (long) 4 * (long) theBitMap->yBound;
	FailNIL(theBitMap->theLines = (long *((*)[])) ZNewPtr(needMem));
	lineAddr = (long) theBitMap->baseAddr;
	for (lineNo = 0; lineNo < theBitMap->yBound; lineNo++) {
		(*theBitMap->theLines)[lineNo] = (long *) lineAddr;
		lineAddr += theBitMap->rowBytes;
		}

	holdPort = qd.thePort;
	OpenPort(&theBitMap->qdPort);
	qdbm.baseAddr = (QDPtr) theBitMap->baseAddr;
	qdbm.rowBytes = theBitMap->rowBytes;
	SetRect(&qdbm.bounds, 0, 0,
		theBitMap->rowBytes * 8, theBitMap->yBound);
	SetPortBits(&qdbm);
	
	SetRect(&theBitMap->qdPort.portRect, 0, 0,
		theBitMap->rowBytes * 8, theBitMap->yBound);
	RectRgn(theBitMap->qdPort.clipRgn, &theBitMap->qdPort.portRect); 
	RectRgn(theBitMap->qdPort.visRgn, &theBitMap->qdPort.portRect); 

	PenMode(patCopy);
	PenPat(qd.white);
	SetPort(holdPort);
	
	theBitMap->bitDepth = 1;

}

ZBitMap *ZNewBitMap (
	short xBound, short yBound )
{
	long needMem;
	short rowLongs;
	ZBitMap *newBitMap;

	newBitMap = nil;

	if ((xBound > 0) && (yBound > 0)) {
		FailNIL(newBitMap = (ZBitMap *) ZNewPtr((sizeof(ZBitMap))));
		newBitMap->baseAddr = nil;
		newBitMap->yBound = yBound;
		rowLongs = (xBound - 1) / 32 + 1;
		newBitMap->rowBytes = rowLongs * 4;
		newBitMap->xBound = xBound;
		needMem = (long) newBitMap->rowBytes * (long) newBitMap->yBound;
		FailNIL(newBitMap->baseAddr = (long *) ZNewPtr(needMem));
		newBitMap->qdRowBytes = 0;
		ZInitBitMap(newBitMap);
		return (newBitMap);
		}
	else
		return (nil);
}

ZBitMap *ZDupBitMap (
	ZBitMap *srcBitMap )
{
	ZBitMap *newBitMap, *holdBM;

	newBitMap = nil;
	holdBM = zCurBitMap;

	FailNIL(newBitMap = (ZBitMap *) ZNewPtr(sizeof(ZBitMap)));
	newBitMap->baseAddr = srcBitMap->baseAddr;
	newBitMap->xBound = srcBitMap->xBound;
	newBitMap->yBound = srcBitMap->yBound;
	newBitMap->rowBytes = srcBitMap->rowBytes;
	newBitMap->qdRowBytes = srcBitMap->qdRowBytes;

	ZInitBitMap(newBitMap);
	ZSetBitMap(newBitMap);
	ZSetOrigin(srcBitMap->xOrig, srcBitMap->yOrig);
	ZSetAbsClipRect(&srcBitMap->clipRect);
	ZSetBitMap(holdBM);
	
	return (newBitMap);
}

void ZFreeBitMap (
	ZBitMap *bitMap )
{
	ClosePort(&bitMap->qdPort);
	ForgetPtr((Ptr) bitMap->baseAddr);
	ForgetPtr((Ptr) bitMap->theLines);
	ForgetPtr((Ptr) bitMap);
}

void ZSetBit (
	long *longPtr,
	short bitNo ) 		// highest order bit is 0
{
	asm {
		move.l	longPtr, a0
		move.l	(a0), d0
		move.l	#31, d1
		sub.w	bitNo, d1
		bset	d1, d0
		move.l	d0, (a0)
		}
}

void ZSetRect (
	Rect *r,
	short left, short top,
	short right, short bottom )
{
	asm 68000 {
		move.l	r, a0
		move.w	left, OFFSET(Rect, left) (a0)
		move.w	top, OFFSET(Rect, top) (a0)
		move.w	right, OFFSET(Rect, right) (a0)
		move.w	bottom, OFFSET(Rect, bottom) (a0)
		}
}

void ZOffsetRect (
	Rect *r,
	short x, short y )
{
	asm 68000 {
		move.l	r, a0
		move.w	x, d0
		move.w	y, d1
		add.w	d0, OFFSET(Rect, left) (a0)
		add.w	d1, OFFSET(Rect, top) (a0)
		add.w	d0, OFFSET(Rect, right) (a0)
		add.w	d1, OFFSET(Rect, bottom) (a0)
		}
}

void ZSetOrigin (
	short xOrig,
	short yOrig )
{
	Rect r;

	zCurBitMap->xOrig = xOrig;
	zCurBitMap->yOrig = yOrig;
	SetOrigin(xOrig, yOrig);
	
	/*
		The clipRect is maintained across calls to ZSetOrigin. This means
		the QuickDraw clipping rectangle needs to be reset each time the
		origin is changed.
	*/
	
	r = zCurBitMap->clipRect;
	ZOffsetRect(&r, xOrig, yOrig);
	ClipRect(&r);
}

void ZSetAbsClipRect (
	Rect *r )
{
	Rect shiftedRect;

	zCurBitMap->clipRect = *r;
	shiftedRect = *r;
	ZOffsetRect(&shiftedRect, zCurBitMap->xOrig, zCurBitMap->yOrig);
	ClipRect(&shiftedRect);
}

void ZSetRelClipRect (
	Rect *r )
{
	Rect shiftedRect;

	ClipRect(r);
	shiftedRect = *r;
	ZOffsetRect(&shiftedRect, -zCurBitMap->xOrig, -zCurBitMap->yOrig);

	/* shiftedRect should be in absolute cooridinates */

	zCurBitMap->clipRect = shiftedRect;
}

void ZSetShade (
	short shade )
{
	zCurPat = &(*zShades)[shade];
	PenPat(qdShades[shade]);
}

void ZSetBitMap (
	ZBitMap *bitMap )
{
	zCurBitMap = bitMap;
	SetPort(&bitMap->qdPort);
}

short ZGetPixel (
	short x, short y )
{
	asm 68000 {
		movem.l	d7, -(sp)
		clr.l	d1
		move.w	x, d0
		move.w	y, d1

		/*	d0 = x
			d1 = y */

		/* Check to see if the pixel */
		/* falls within the clipping rectangle */
		
		move.l	zCurBitMap, a0
		
		/* a0 = address of current ZBitMap record */

		sub.w	OFFSET(ZBitMap, xOrig) (a0), d0			/* too far right */
		cmp.w	OFFSET(ZBitMap, xBound) (a0), d0		
		bge		@out
		
		tst.w	d0			/* too far left */
		bmi		@out

		sub.w	OFFSET(ZBitMap, yOrig) (a0), d1			/* too far down */
		cmp.w	OFFSET(ZBitMap, yBound) (a0), d1
		bge		@out

		tst.w	d1			/* too far up */
		bmi		@out

		/* division by 8 via masking & shifting */
		/* to obtain byte & bit nos. */
		
		move.w	d0, d2
		and.w	#0x0007, d2		/* d2.w = x % 8 */
		move.w	#7, d7
		sub.w	d2, d7			/* d7.w = bit no. */
		
		and.w	#0xFFF8, d0		/* d0.w = number of bytes to add to */
		lsr.w	#3, d0			/* row address for byte */

		move.l	OFFSET(ZBitMap, theLines) (a0), a1
								/* a1 = address of theLines array */
		lsl.l	#2, d1			/* d1.w = index into theLines */
		move.l	0(a1,d1.l), a1
		adda.w	d0, a1			/* a2 = address of byte */
		btst.b	d7, (a1)
		seq		d0				/* return true if bit is 0 (white) */
		and.w	#1, d0
@out:
		movem.l	(sp)+, d7
	}

}

void ZSetPixel (
	short x, short y,
	short value )
{
	asm 68000 {
		movem.l	d7, -(sp)
		clr.l	d1
		move.w	x, d0
		move.w	y, d1

		/*	d0 = x
			d1 = y */

		/* Check to see if the pixel */
		/* falls within the clipping rectangle */
		
		move.l	zCurBitMap, a0	/* a0 = address of current ZBitMap record */
		sub.w	OFFSET(ZBitMap, xOrig) (a0), d0			/* too far right */
		cmp.w	OFFSET(ZBitMap, clipRect.right) (a0), d0		
		bge		@noDraw
		
		cmp.w	OFFSET(ZBitMap, clipRect.left) (a0), d0	/* too far left */
		blt		@noDraw

		sub.w	OFFSET(ZBitMap, yOrig) (a0), d1			/* too far down */
		cmp.w	OFFSET(ZBitMap, clipRect.bottom) (a0), d1
		bge		@noDraw

		/* too far up */

		cmp.w	OFFSET(ZBitMap, clipRect.top) (a0), d1
		blt		@noDraw
				
		/* division by 8 via masking & shifting */
		/* to obtain byte & bit nos. */
		
		move.w	d0, d2
		and.w	#0x0007, d2		/* d2.w = x % 8 */
		move.w	#7, d7
		sub.w	d2, d7			/* d7.w = bit no. */
		
		and.w	#0xFFF8, d0		/* d0.w = number of bytes to add to */
		lsr.w	#3, d0			/* row address for byte */

		move.l	OFFSET(ZBitMap, theLines) (a0), a1
								/* a1 = address of theLines array */
		lsl.l	#2, d1			/* d4.w = index into theLines */
		move.l	0(a1,d1.l), a1
		adda.w	d0, a1			/* a2 = address of byte */
		tst.w	value
		beq		@blackPixel
		bclr.b	d7, (a1)		/* clear bits are white */
		bra		@out
@blackPixel:
		bset.b	d7, (a1)		/* set bits are black */				
@noDraw:
@out:
		movem.l	(sp)+, d7
	}

}

void ZHLine (
	short left, short right,
	short y )
{
	asm 68000 {

		movem.l	d3-d7/a2-a4, -(sp)
 		move.w	left, d0
		move.w	right, d2
		move.w	y, d1

		/*	d0 = left
			d1 = y
			d2 = right
 		*/
 		
 		addq.w	#1, d2			/* line includes last pixel beyond last */
 								/* coordinate line, so move right */
 								/* boundary to right */
 	
		cmp.w	d0, d2			/* if x1 <= x0, line is invisible */
		ble		@noDraw			/* or invalid */
		
		/* line is valid and possibly visible; check to see if any of it */
		/* falls within the clipping rectangle */
		
		move.l	zCurBitMap, a0	/* a0 = address of current ZBitMap record */
		sub.w	OFFSET(ZBitMap, xOrig) (a0), d0			/* too far right */
		cmp.w	OFFSET(ZBitMap, clipRect.right) (a0), d0		
		bge		@noDraw
	
		sub.w	OFFSET(ZBitMap, yOrig) (a0), d1			/* too far down */
		cmp.w	OFFSET(ZBitMap, clipRect.bottom) (a0), d1
		bge		@noDraw

		sub.w	OFFSET(ZBitMap, xOrig) (a0), d2			/* too far left */
		cmp.w	OFFSET(ZBitMap, clipRect.left) (a0), d2
		ble		@noDraw

		cmp.w	OFFSET(ZBitMap, clipRect.top) (a0), d1	/* too far up */
		blt		@noDraw
		
		/* at least some of the line is visible; now clip it */
				
		cmp.w	OFFSET(ZBitMap, clipRect.left) (a0), d0
		bge		@leftOK
		move.w	OFFSET(ZBitMap, clipRect.left) (a0), d0
@leftOK:
		cmp.w	OFFSET(ZBitMap, clipRect.right) (a0), d2
		ble		@lineOK
		move.w	OFFSET(ZBitMap, clipRect.right) (a0), d2

		/* line is valid and clipped */
		/* now convert from boundary lines to pixels */

@lineOK:
		subq.w	#1, d2
		
		/* division by 32 via masking & shifting */
		/* to obtain begin/end addr & bit nos. */
		
		move.w	d0, d4
		and.w	#0x001F, d4		/* d4.w = beginBit */
		and.w	#0xFFE0, d0
		lsr.w	#3, d0			/* d0.w = number of bytes to add to */
								/* row address for beginLong */

		move.w	d2, d5
		and.w	#0x001F, d5		/* d5.w = endBit */
		and.w	#0xFFE0, d2
		lsr.w	#3, d2			/* d2.w = number of bytes to add to */
								/* row address for endLong */

		move.l	OFFSET(ZBitMap, theLines) (a0), a1	/* a1 = address of theLines array */
		clr.l	d7
		move.w	d1, d7
		lsl.l	#2, d7			/* d7.w = index into theLines */
		move.l	0(a1,d7.l), a2
		adda.w	d0, a2			/* a2 = address of beginLong */
		
		/* retrieve begin/end masks and zCurPat address */
		
		lea		zMaskLongs, a1
		move.l	zCurPat, a3 
		and.w	#0x0007, d1		/* low 5 bits of vert coord determine */
								/* index into pattern */
		lsl.w	#2, d1			/* valid indices: 0, 4, 8, 12... */
		move.w	d2, d6			/* a3 = address of pattern */
		sub.w	d0, d6			/* d6 = difference (bytes) between */
		bgt		@notEqual		/* beginLong and endLong */
		
		/* begin and end long are the same */	

		lsl.w	#7, d4
		lsl.w	#2, d5			/* array index: zMaskLongs[begin][end] */
		add.w	d4, d5
		move.l	0(a1, d5.w), d5		/* d5 holds mask */
		move.l	d5, d6
		not.l	d6
		move.l	0(a3, d1.w), d7		/* retrieve pattern long */
		and.l	d5, d7
		and.l	d6, (a2)
		or.l	d7, (a2)
		bra		@out				/* done */
		
@notEqual:		
		cmp.w	#4, d6
		bgt		@doHoriz
		
		/* begin and end longs are adjacent */

		lsl.w	#7, d4				
		move.l	0x7c(a1, d4.w), d4	/* d4 holds mask for left long */
		move.l	d4, d2
		not.l	d2
		lsl.w	#2, d5				/*    -- zMaskLongs[beginLong][31] */	
		move.l	0(a1, d5.w), d5		/* d5 holds mask for right long */
									/*    -- zMaskLongs[0][endLong] */
		move.l	d5, d3
		not.l	d3
		move.l	0(a3, d1.w), d7		/* retrieve pattern long */
		and.l	d4, d7
		and.l	d2, (a2)
		or.l	d7, (a2)+
		move.l	0(a3, d1.w), d7		/* get clean pattern long */
		and.l	d5, d7
		and.l	d3, (a2)
		or.l	d7, (a2)
		bra		@out				/* done */

		/* begin and end longs are separated by one or more longs */
		
@doHoriz:
		lsl.w	#7, d4				/* d4 holds mask for left long */
		move.l	0x7c(a1, d4.w), d4	/*    -- zMaskLongs[beginLong][31] */				
		lsl.w	#2, d5				/* d5 holds mask for right long */
		move.l	0(a1, d5.w), d5		/*    -- zMaskLongs[0][endLong] */

		subq.w	#4, d6				/* get difference in terms */
		lsr.w	#1, d6				/* of instructions (for jmp); */
		lea		@finish, a1			/* two bytes per long to be moved */
		suba.w	d6, a1
		move.l	d4, d0
		not.l	d0
		move.l	d5, d3
		not.l	d3
@back3:
		move.l	0(a3, d1.w), d7		/* retrieve pattern long */
		and.l	d4, d7
		and.l	d0, (a2)
		or.l	d7, (a2)+
		move.l	0(a3, d1.w), d7		/* retrieve clean pattern long */
		jmp		(a1)

		move.l	d7, (a2)+			/* 50 longs (1600 pixels, the max) */
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+

		move.l	d7, (a2)+			/* 40 longs */
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+

		move.l	d7, (a2)+			/* 30 longs */
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+

		move.l	d7, (a2)+			/* 20 longs */
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+

		move.l	d7, (a2)+			/* 10 longs */	
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
@finish:
		and.l	d5, d7
		and.l	d3, (a2)
		or.l	d7, (a2)+
@out:
@noDraw:
		movem.l	(sp)+,d3-d7/a2-a4
	}
}

void ZVLine (
	short top, short bottom,
	short x )
{
	asm 68000 {
	
		movem.l	d3-d7/a2-a4, -(sp)
		move.w	x, d0
		move.w	top, d1
		move.w	bottom, d3

		/*	d0 = x
			d1 = top
			d3 = bottom */	
			
		addq.w	#1, d3

		cmp.w	d1, d3			/* if y1 <= y0, line is invisible	*/
		ble		@noDraw			/* or invalid */

		
		/* line is valid and possibly visible; check to see if any of it */
		/* falls within the clipping rectangle */
		
		move.l	zCurBitMap, a0	/* a0 = address of current ZBitMap record */
		sub.w	OFFSET(ZBitMap, xOrig) (a0), d0			/* too far right */
		cmp.w	OFFSET(ZBitMap, clipRect.right) (a0), d0		
		bge		@noDraw
	
		cmp.w	OFFSET(ZBitMap, clipRect.left) (a0), d0	/* too far left */
		blt		@noDraw

		sub.w	OFFSET(ZBitMap, yOrig) (a0), d1			/* too far down */
		cmp.w	OFFSET(ZBitMap, clipRect.bottom) (a0), d1
		bge		@noDraw

		sub.w	OFFSET(ZBitMap, yOrig) (a0), d3			/* too far up */
		cmp.w	OFFSET(ZBitMap, clipRect.top) (a0), d3
		ble		@noDraw
		
		/* at least some is visible; now clip it */
				
		cmp.w	OFFSET(ZBitMap, clipRect.top) (a0), d1
		bge		@topOK
		move.w	OFFSET(ZBitMap, clipRect.top) (a0), d1
@topOK:
		cmp.w	OFFSET(ZBitMap, clipRect.bottom) (a0), d3
		ble		@lineOK
		move.w	OFFSET(ZBitMap, clipRect.bottom) (a0), d3

		/* line is valid and clipped */
		/* now convert from boundary lines to first/last pixels */

@lineOK:
		subq.w	#1, d3
		
		/* division by 32 via masking & shifting */
		/* to obtain begin/end addr & bit nos. */
		
		move.w	d0, d4
		and.w	#0x0007, d4		/* d4.w = bit no. */
		and.w	#0xFFF8, d0
		lsr.w	#3, d0			/* d0.w = number of bytes to add to */
								/* row address for beginByte */

		move.l	OFFSET(ZBitMap, theLines) (a0), a1
		clr.l	d7
		move.w	d1, d7			/* a1 = address of theLines array */
		lsl.l	#2, d7			/* d7.w = index into theLines */
		move.l	0(a1,d7.l), a2
		adda.w	d0, a2			/* a2 = address of beginByte on first line */
		clr.l	d7
		move.w	d3, d7
		lsl.l	#2, d7
		move.l	0(a1,d7.l),	a4
		adda.w	d0, a4			/* a4 = address of beginByte on last line */
		
		/* retrieve pattern address */
		
		move.l	zCurPat, a3 
		and.w	#0x0007, d1		/* low 5 bits of top coord determine */
								/* index into pattern */
		lsl.w	#2, d1			/* valid indices: 0, 4, 8, 12... */
								/* a3 = address of pattern */
		move.w	d0, d7
		and.w	#0x0003, d7
		add.w	d7, d1
		move.w	#128, d6
		add.w	d7, d6
		move.b	#7, d5
		sub.b	d4, d5
		clr.b	d2
		bset	d5, d2
		move.b	d2, d5
		not.b	d5
		move.w	OFFSET(ZBitMap, rowBytes) (a0), a1
@back:
		move.b	0(a3, d1.w), d3		/* d3 holds pattern byte */
		and.b	d2, d3
		move.b	(a2), d4
		and.b	d5, d4
		or.b	d3, d4
		move.b	d4, (a2)
		addq.w	#4, d1
		cmp.w	d6, d1
		bne		@down
		move.w	d7, d1
@down:
		adda.w	a1, a2
		cmpa.l	a4, a2
		ble		@back
@out:
@noDraw:
		movem.l	(sp)+,d3-d7/a2-a4
	}
}

void ZVSLine (
	short top, short bottom,
	short x,
	short value,
	short skipFactor )
{
	asm 68000 {
		movem.l	d3-d7/a2-a4, -(sp)
		clr.l	d3
		move.w	x, d0
		move.w	top, d1
		move.w	bottom, d3

		/*	d0 = x
			d1 = top
			d3 = bottom */	

		addq.w	#1, d3

		cmp.w	d1, d3			/* if y1 <= y0, line is invisible	*/
		ble		@noDraw			/* or invalid */

		
		/* line is valid and possibly visible; check to see if any of it */
		/* falls within the clipping rectangle */
		
		move.l	zCurBitMap, a0	/* a0 = address of current ZBitMap record */
		sub.w	OFFSET(ZBitMap, xOrig) (a0), d0			/* too far right */
		cmp.w	OFFSET(ZBitMap, clipRect.right) (a0), d0		
		bge		@noDraw
	
		cmp.w	OFFSET(ZBitMap, clipRect.left) (a0), d0	/* too far left */
		blt		@noDraw

		sub.w	OFFSET(ZBitMap, yOrig) (a0), d1			/* too far down */
		cmp.w	OFFSET(ZBitMap, clipRect.bottom) (a0), d1
		bge		@noDraw

		sub.w	OFFSET(ZBitMap, yOrig) (a0), d3			/* too far up */
		cmp.w	OFFSET(ZBitMap, clipRect.top) (a0), d3
		ble		@noDraw
		
		/* at least some is visible; now clip it */
				
		cmp.w	OFFSET(ZBitMap, clipRect.top) (a0), d1
		bge		@topOK
		move.w	OFFSET(ZBitMap, clipRect.top) (a0), d1
@topOK:
		cmp.w	OFFSET(ZBitMap, clipRect.bottom) (a0), d3
		ble		@lineOK
		move.w	OFFSET(ZBitMap, clipRect.bottom) (a0), d3

		/* line is valid and clipped */
		/* now convert from boundary lines to first/last pixels */

@lineOK:
		subq.w	#1, d3
		
		/*
		
		If the line is being drawn in medium gray (skipFactor = 1)
		make sure the dots are aligned with the medium gray dot
		pattern by incrementing the top coordinate when the sum
		of the horizontal and vertical starting coordinates is odd.
		
		*/
		
		cmp.w	#1, skipFactor
		bne		@drawIt	
		move.w	d1, d2
		add.w	d0, d2
		btst	#0, d2
		beq		@drawIt
		add.w	#1, d1
		
@drawIt:
		/* division by 32 via masking & shifting */
		/* to obtain begin/end addr & bit nos. */
		
		move.w	d0, d4
		and.w	#0x0007, d4		/* d4.w = x % 8 */
		move.w	#7, d7
		sub.w	d4, d7			/* d7.w = bit no. */
		
		and.w	#0xFFF8, d0		/* d0.w = number of bytes to add to */
		lsr.w	#3, d0			/* row address for byte */

		move.l	OFFSET(ZBitMap, theLines) (a0), a1
		clr.l	d4
		move.w	d1, d4			/* a1 = address of theLines array */
		lsl.l	#2, d4			/* d4.w = index into theLines */
		move.l	0(a1,d4.l), a2
		adda.w	d0, a2			/* a2 = address of byte on first line */
		lsl.l	#2, d3
		move.l	0(a1,d3.l),	a4
		adda.w	d0, a4			/* a4 = address of byte on last line */
		move.w	OFFSET(ZBitMap, rowBytes) (a0), d4
		move.w	skipFactor, d5
		lsl.w	d5, d4
		tst.w	value
		beq		@blackLine
@whiteLine:
		bclr.b	d7, (a2)
		adda.w	d4, a2
		cmpa.l	a4, a2
		ble		@whiteLine
		bra		@out
@blackLine:
		bset.b	d7, (a2)
		adda.w	d4, a2
		cmpa.l	a4, a2
		ble		@blackLine
@out:
@noDraw:
		movem.l	(sp)+,d3-d7/a2-a4
	}
}

void ZFrameRect (
	Rect *r )
{
	asm 68000 {
		movem.l	d3-d7, -(sp)
		move.l	r, a0
		move.w	OFFSET(Rect, left) (a0), d3
		move.w	OFFSET(Rect, top) (a0), d4
		move.w	OFFSET(Rect, right) (a0), d5
		move.w	OFFSET(Rect, bottom) (a0), d6

		cmp.w	d3, d5			/* check to see if rect has zero thickness */
		ble		@noDraw			/* horizontally or vertically */
		cmp.w	d4, d6
		ble		@noDraw

		subq.w	#1, d5			/* Coords for line routines need to be */
		subq.w	#1, d6			/* adjusted so frame is drawn on INSIDE */
								/* of passed rectangle, as in QuickDraw */
				
		/* ZHLine(left, right, top) */
		
		move.w	d4, -(sp)
		move.w	d5, -(sp)
		move.w	d3, -(sp)
		bsr		ZHLine
		addq.l	#6, sp
		
		/* ZHLine(left, right, bottom) */
		
		move.w	d6, -(sp)
		move.w	d5, -(sp)
		move.w	d3, -(sp)
		bsr		ZHLine
		addq.l	#6, sp		
		
		/* If pattern is either black or white, use ZVSLine for speed */
		
		move.l	zCurBitMap, a0
		movea.l	zCurPat, a0
		cmpa.l	zBlack, a0
		bne		@whiteTest
		move.w	#false, d7
		bra		@sLine
@whiteTest:
		cmpa.l	zWhite, a0
		bne		@patLine
		move.w	#true, d7
		
@sLine:
		/* ZVSLine(top, bottom, left, value, skipFactor) */

		move.w	#0, -(sp)
		move.w	d7, -(sp)
		move.w	d3, -(sp)
		move.w	d6, -(sp)
		move.w	d4, -(sp)
		bsr		ZVSLine
		lea		10(sp), sp
		
		/* ZVSLine(top, bottom, right, value, skipFactor) */
		
		move.w	#0, -(sp)
		move.w	d7, -(sp)
		move.w	d5, -(sp)
		move.w	d6, -(sp)
		move.w	d4, -(sp)
		bsr		ZVSLine
		lea		10(sp), sp
		bra		@out

@patLine:
		/* ZVLine(top, bottom, left) */
		
		move.w	d3, -(sp)
		move.w	d6, -(sp)
		move.w	d4, -(sp)
		bsr		ZVLine
		addq.l	#6, sp
		
		/* ZVLine(top, bottom, right) */
		
		move.w	d5, -(sp)
		move.w	d6, -(sp)
		move.w	d4, -(sp)
		bsr		ZVLine
		addq.l	#6, sp
@noDraw:
@out:
		movem.l	(sp)+, d3-d7
	}
}

void ZFillRect (
	Rect *r )
{
	asm 68000 {
	
		movem.l	d3-d7/a2-a4, -(sp)
		move.l	r, a0
		move.w	OFFSET(Rect, left) (a0), d0
		move.w	OFFSET(Rect, top) (a0), d1
		move.w	OFFSET(Rect, right) (a0), d2
		move.w	OFFSET(Rect, bottom) (a0), d3

		/*	d0 = left
			d1 = top
			d2 = right
			d3 = bottom */

		cmp.w	d0, d2			/* if x1 <= x0, rect is invisible */
		ble		@noDraw			/* or invalid */
		cmp.w	d1, d3			/* if y1 <= y0, rect is invisible	*/
		ble		@noDraw			/* or invalid */
		
		/* rect is valid and possibly visible; check to see if any of it */
		/* falls within the clipping rectangle */
		
		move.l	zCurBitMap, a0	/* a0 = address of current ZBitMap record */
		sub.w	OFFSET(ZBitMap, xOrig) (a0), d0			/* too far right */
		cmp.w	OFFSET(ZBitMap, clipRect.right) (a0), d0		
		bge		@noDraw
	
		sub.w	OFFSET(ZBitMap, yOrig) (a0), d1			/* too far down */
		cmp.w	OFFSET(ZBitMap, clipRect.bottom) (a0), d1
		bge		@noDraw

		sub.w	OFFSET(ZBitMap, xOrig) (a0), d2			/* too far left */
		cmp.w	OFFSET(ZBitMap, clipRect.left) (a0), d2
		ble		@noDraw

		sub.w	OFFSET(ZBitMap, yOrig) (a0), d3			/* too far up */
		cmp.w	OFFSET(ZBitMap, clipRect.top) (a0), d3
		ble		@noDraw
		
		/* at least some is visible; now clip it */
				
		cmp.w	OFFSET(ZBitMap, clipRect.left) (a0), d0
		bge		@leftOK
		move.w	OFFSET(ZBitMap, clipRect.left) (a0), d0
@leftOK:
		cmp.w	OFFSET(ZBitMap, clipRect.top) (a0), d1
		bge		@topOK
		move.w	OFFSET(ZBitMap, clipRect.top) (a0), d1
@topOK:
		cmp.w	OFFSET(ZBitMap, clipRect.right) (a0), d2
		ble		@rightOK
		move.w	OFFSET(ZBitMap, clipRect.right) (a0), d2
@rightOK:
		cmp.w	OFFSET(ZBitMap, clipRect.bottom) (a0), d3
		ble		@rectOK
		move.w	OFFSET(ZBitMap, clipRect.bottom) (a0), d3

		/* rect is valid and clipped */
		/* now convert from boundary lines to first/last pixels */

@rectOK:
		subq.w	#1, d2
		subq.w	#1, d3
		
		/* division by 32 via masking & shifting */
		/* to obtain begin/end addr & bit nos. */
		
		move.w	d0, d4
		and.w	#0x001F, d4		/* d4.w = beginBit */
		and.w	#0xFFE0, d0
		lsr.w	#3, d0			/* d0.w = number of bytes to add to */
								/* row address for beginLong */

		move.w	d2, d5
		and.w	#0x001F, d5		/* d5.w = endBit */
		and.w	#0xFFE0, d2
		lsr.w	#3, d2			/* d2.w = number of bytes to add to */
								/* row address for endLong */

		move.l	OFFSET(ZBitMap, theLines) (a0), a1
		clr.l	d7
		move.w	d1, d7			/* a1 = address of theLines array */
		lsl.l	#2, d7			/* d7.l = index into theLines */
		move.l	0(a1,d7.l), a2
		adda.w	d0, a2			/* a2 = address of beginLong on first line */

		clr.l	d7
		move.w	d3, d7
		lsl.l	#2, d7
		move.l	0(a1,d7.l),	a4
		adda.w	d0, a4			/* a4 = address of beginLong on last line */
		
		/* retrieve begin/end masks and zCurPat address */
		
		lea		zMaskLongs, a1
		move.l	zCurPat, a3 
		and.w	#0x0007, d1		/* low 5 bits of top coord determine */
								/* index into pattern */
		lsl.w	#2, d1			/* valid indices: 0, 4, 8, 12... */
		move.w	d2, d6			/* a3 = address of pattern */
		sub.w	d0, d6			/* d6 = difference (bytes) between */
		bgt		@notEqual		/* beginLong and endLong */
		
		/* begin and end long are the same */	

		lsl.w	#7, d4
		lsl.w	#2, d5			/* array index: zMaskLongs[begin][end] */
		add.w	d4, d5
		move.l	0(a1, d5.w), d5		/* d5 holds mask */
		move.l	d5, d6
		not.l	d6
@back1:
		move.l	0(a3, d1.w), d7		/* retrieve pattern long */
		and.l	d5, d7
		and.l	d6, (a2)
		or.l	d7, (a2)
		addq.w	#4, d1
		cmp.w	#128, d1
		bne		@down1
		clr.w	d1
@down1:
		add.w	OFFSET(ZBitMap, rowBytes) (a0), a2
		cmpa.l	a4, a2
		ble		@back1
		bra		@out
		
@notEqual:		
		cmp.w	#4, d6
		bgt		@doHoriz
		
		/* begin and end longs are adjacent */

		lsl.w	#7, d4				
		move.l	0x7c(a1, d4.w), d4	/* d4 holds mask for left long */
		move.l	d4, d2
		not.l	d2
		lsl.w	#2, d5				/*    -- zMaskLongs[beginLong][31] */	
		move.l	0(a1, d5.w), d5		/* d5 holds mask for right long */
									/*    -- zMaskLongs[0][endLong] */
		move.l	d5, d3
		not.l	d3
		move.w	OFFSET(ZBitMap, rowBytes) (a0), d6
		subq.w	#4, d6				/* one long less than one line */
									/* added each time */
@back2:
		move.l	0(a3, d1.w), d7		/* retrieve pattern long */
		and.l	d4, d7
		and.l	d2, (a2)
		or.l	d7, (a2)+
		move.l	0(a3, d1.w), d7		/* retrieve clean pattern long */
		and.l	d5, d7
		and.l	d3, (a2)
		or.l	d7, (a2)
		addq.w	#4, d1
		cmp.w	#128, d1
		bne		@down2
		clr.w	d1
@down2:
		adda.w	d6, a2
		cmpa.l	a4, a2
		ble		@back2
		bra		@out

		/* begin and end longs are separated by one or more longs */
		
@doHoriz:
		lsl.w	#7, d4				/* d4 holds mask for left long */
		move.l	0x7c(a1, d4.w), d4	/*    -- zMaskLongs[beginLong][31] */				
		lsl.w	#2, d5				/* d5 holds mask for right long */
		move.l	0(a1, d5.w), d5		/*    -- zMaskLongs[0][endLong] */

		subq.w	#4, d6				/* get difference in terms */
		lsr.w	#1, d6				/* of instructions (for jmp); */
		lea		@finish, a1			/* two bytes per long to be moved */
		suba.w	d6, a1
		move.w	OFFSET(ZBitMap, rowBytes) (a0), d6
		sub.w	d2, d6
		add.w	d0, d6
		move.l	d4, d0
		not.l	d0
		move.l	d5, d3
		not.l	d3
@back3:
		move.l	0(a3, d1.w), d7		/* retrieve pattern long */
		and.l	d4, d7
		and.l	d0, (a2)
		or.l	d7, (a2)+
		move.l	0(a3, d1.w), d7		/* retrieve clean pattern long */
		jmp		(a1)

		move.l	d7, (a2)+			/* 50 longs (1600 pixels, the max) */
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+

		move.l	d7, (a2)+			/* 40 longs */
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+

		move.l	d7, (a2)+			/* 30 longs */
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+

		move.l	d7, (a2)+			/* 20 longs */
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+

		move.l	d7, (a2)+			/* 10 longs */	
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
@finish:
		and.l	d5, d7
		and.l	d3, (a2)
		or.l	d7, (a2)
		addq.w	#4, d1
		cmp.w	#128, d1
		bne		@down3
		clr.w	d1
@down3:	
		adda.w	d6, a2
		cmpa.l	a4, a2
		bgt		@out
		bra		@back3
@out:
@noDraw:
		movem.l	(sp)+, d3-d7/a2-a4
	}

}

/*
	Function: ZCopyLongs

	This routine performs no error checking and no clipping.
	Be careful -- it can easily clobber memory.
	All coordinates are absolute (origin = 0, 0) for both
	the source rectangle and the destination coordinates.
*/

void ZCopyLongs (
	ZBitMap *srcBits, ZBitMap *dstBits,
	Rect *srcRectAbs,
	short xDestAbs, short yDestAbs )
{
	short lineLenBytes;

	asm 68000 {
		movem.l	d3-d7/a2-a4, -(sp)
		clr.l	d1
		clr.l	d3
		move.l	srcRectAbs, a0
		move.w	OFFSET(Rect, left) (a0), d0
		move.w	OFFSET(Rect, top) (a0), d1
		move.w	OFFSET(Rect, right) (a0), d2
		move.w	OFFSET(Rect, bottom) (a0), d3
		

		/*	d0 = left
			d1 = top
			d2 = right
			d3 = bottom */

		cmp.w	d1, d3
		ble		@noMove
		move.l	srcBits, a0
		move.l	dstBits, a1

		/* convert from boundary lines to first/last pixels */
		subq.w	#1, d2
		subq.w	#1, d3
		
		/* division by 32 via shifting & masking */
		/* to obtain begin/end addr */
	

		and.w	#0xFFE0, d0
		lsr.w	#3, d0			/* d0.w = number of bytes to add to */
								/* row address for beginLong (src) */
		and.w	#0xFFE0, d2
		lsr.w	#3, d2			/* d2.w = number of bytes to add to */
								/* row address for endLong (src) */

		move.w	d2, lineLenBytes
		sub.w	d0, lineLenBytes

		move.w	xDestAbs, d5	/* d5.w = number of bytes to add to */
		and.w	#0xFFE0, d5
		lsr.w	#3, d5			/* row address for beginLong (dest) */

		move.l	OFFSET(ZBitMap, theLines) (a0), a3
		lsl.l	#2, d1				/* d1.w = index into theLines */
		move.l	0(a3,d1.l), a2
		adda.w	d0, a2				/* a2 = address of beginLong */
									/* on first src line */
		lsl.l	#2, d3
		move.l	0(a3,d3.l),	a3
		adda.w	d0, a3				/* d4 = address of beginLong */
		move.l	a3, d4				/* on last src line */
									
		move.l	OFFSET(ZBitMap, theLines) (a1), a4
		clr.l	d7
		move.w	yDestAbs, d7
		lsl.l	#2, d7				/* d7.w = index into theLines */
		move.l	0(a4,d7.l), a4
		adda.w	d5, a4				/* a4 = address of beginLong */
									/* on first dest line */
				
		move.w	d2, d6				/* d6 is the difference between begin */
		sub.w	d0, d6				/* and end long (bytes) */
		addq.w	#4, d6				/* One more long must be moved than */
									/* the difference */
									/* Get difference in terms of */
		lsr.w	#1, d6				/* of instructions (for jmp); */
		lea		@finish, a3			/* two bytes per long to be moved */
		suba.w	d6, a3
		
		/* calculate how much should be added to both source and */
		/* destination addresses after each line is moved */
		
		move.w	OFFSET(ZBitMap, rowBytes) (a0), d6
		sub.w	d2, d6
		add.w	d0, d6			/* add d6 to source address */
		
		move.w	OFFSET(ZBitMap, rowBytes) (a1), d7
		move.w	lineLenBytes, d1
		add.w	d5, d1
		sub.w	d1, d7
		add.w	d5, d7
@back3:
		jmp		(a3)
		move.l	(a2)+, (a4)+			/* 50 longs (1600 pixels, the max) */
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+

		move.l	(a2)+, (a4)+			/* 40 longs */
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+

		move.l	(a2)+, (a4)+			/* 30 longs */
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+

		move.l	(a2)+, (a4)+			/* 20 longs */
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+

		move.l	(a2)+, (a4)+			/* 10 longs */	
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2), (a4)
@finish:
		adda.w	d6, a2
		cmpa.l	d4, a2
		bgt		@out
		adda.w	d7, a4
		bra		@back3
@out:
@noMove:
		movem.l	(sp)+, d3-d7/a2-a4
	}
}

void ZClearLongs (
	Rect *rectAbs )
{
	asm 68000 {
		movem.l	d3-d7/a2-a4, -(sp)
		move.l	rectAbs, a0
		clr.l	d1
		clr.l	d3
		move.w	OFFSET(Rect, left) (a0), d0
		move.w	OFFSET(Rect, top) (a0), d1
		move.w	OFFSET(Rect, right) (a0), d2
		move.w	OFFSET(Rect, bottom) (a0), d3

		/*	d0 = left
			d1 = top
			d2 = right
			d3 = bottom */

		cmp.w	d1, d3
		ble		@noMove

		/* convert from boundary lines to first/last pixels */
		subq.w	#1, d2
		subq.w	#1, d3
		
		/* division by 32 via shifting & masking */
		/* to obtain begin/end addr */
	
		and.w	#0xFFE0, d0
		lsr.w	#3, d0			/* d0.w = number of bytes to add to */
								/* row address for beginLong (src) */
		and.w	#0xFFE0, d2
		lsr.w	#3, d2			/* d2.w = number of bytes to add to */
								/* row address for endLong (src) */

		move.l	zCurBitMap, a0
		move.l	OFFSET(ZBitMap, theLines) (a0), a3
		lsl.l	#2, d1				/* d1.w = index into theLines */
		move.l	0(a3,d1.l), a2
		adda.w	d0, a2				/* a2 = address of beginLong */
									/* on first line */
		lsl.l	#2, d3
		move.l	0(a3,d3.l),	a4
		adda.w	d0, a4				/* a4 = address of beginLong */
									/* on last line */
									
		move.w	d2, d6				/* d6 is the difference between begin */
		sub.w	d0, d6				/* and end long (bytes) */
		addq.w	#4, d6				/* One more long must be moved than */
									/* the difference */
									/* Get difference in terms of */
		lsr.w	#1, d6				/* of instructions (for jmp); */
		lea		@finish, a3			/* two bytes per long to be moved */
		suba.w	d6, a3
		
		/* calculate how much should be added to */
		/* address after each line is moved */
		
		move.w	OFFSET(ZBitMap, rowBytes) (a0), d6
		sub.w	d2, d6
		add.w	d0, d6				/* add d6 to address */
		
		move.l	#0xFFFFFFFF, d7
@back3:
		jmp		(a3)
		move.l	d7, (a2)+			/* 50 longs (1600 pixels, the max) */
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+

		move.l	d7, (a2)+			/* 40 longs */
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+

		move.l	d7, (a2)+			/* 30 longs */
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+

		move.l	d7, (a2)+			/* 20 longs */
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+

		move.l	d7, (a2)+			/* 10 longs */	
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)+
		move.l	d7, (a2)
@finish:
		adda.w	d6, a2
		cmpa.l	a4, a2
		bgt		@out
		bra		@back3
@out:
@noMove:
		movem.l	(sp)+, d3-d7/a2-a4
	}
}