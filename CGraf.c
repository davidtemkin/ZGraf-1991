/*
 * ZGraf - CGraf.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include "ZGraf.h"
#include "CGraf.h"

RGBColor rgbBlack, rgbGreen, rgbRed, rgbAmber; 
ColorSpec depthColors[4];
CMaskTable *cGreenMasks, *cRedMasks, *cAmberMasks;
Boolean fakeColorDisp;
CTabHandle systemCTab;
GDHandle saveGDev, myGDev;
short reqList[257];

void CInitGraf (
	Boolean isColorMonitor )
{
	short begin, end;
	short bit, i;
	Handle dithPats;
	
	cGreenMasks = cRedMasks = cAmberMasks = nil;
	zShades = nil;
	zPage1BitMap = zPage2BitMap = nil;

	if (gConfig.gameScreenDepth == 4)
		ZInitFourBitTable();
	else if (gConfig.gameScreenDepth == 8)
		ZInitEightBitTable();
	
	systemCTab = nil;
	FailNIL(systemCTab = (CTabHandle) NewHandle(0));
	
	FailNIL(cGreenMasks = (CMaskTable *) ZNewPtr(sizeof(CMaskTable)));
	FailNIL(cRedMasks = (CMaskTable *) ZNewPtr(sizeof(CMaskTable)));
	FailNIL(cAmberMasks = (CMaskTable *) ZNewPtr(sizeof(CMaskTable)));
	
	/*
		Full mask longs are as follows (highest to lowest order bits):

		Green:	01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01
		Red:	10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10
		Amber:	11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11

	*/
		
	for (begin = 0; begin < 16; begin++) {
		for (end = 0; end < 16; end++) {
			(*cGreenMasks)[begin][end] = 0L;
			for (bit = begin; bit <= end; bit++)
				ZSetBit(&((*cGreenMasks)[begin][end]), bit * 2 + 1);
			(*cRedMasks)[begin][end] =
				(*cGreenMasks)[begin][end] << 1;
			(*cAmberMasks)[begin][end] =
				(*cGreenMasks)[begin][end] | (*cRedMasks)[begin][end];
			}
		}

	FailNIL(zShades = (ZPatTable *) ZNewPtr(sizeof(ZPatTable)));

	for (i=0; i<11; i++) {
		unsigned short *p;
		unsigned char b;
		unsigned short word;
		short line, bit;

		GetIndPattern((void *) &qdShades[i], 2000, i+1);
		FailResError();
		for (line = 0; line < 8; line++) {
			p = (unsigned short *) &(*zShades)[i][line];
			b = qdShades[i][line];
			word = 0;
			for (bit = 0; bit < 8; bit++)
				if (!BitTst(&b, bit)) {
					BitSet(&word, bit * 2);
					BitSet(&word, bit * 2 + 1);
					}
			*p = word; *(p+1) = word;
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

	dithPats = nil;
	FailNILRes(dithPats = GetResource('dith', 129));
	DetachResource(dithPats);
	HLock(dithPats);
	zDitherShades = (ZPatTable (*)[8]) *dithPats;
	zCurDitherIndex = 0;
	zCurDitherTable = &(*zDitherShades)[zCurDitherIndex];

	// Uh, yeah...
	
	/*
		Initialize the screen bitmap.
	*/
	
	FailNIL(zPage1BitMap = (ZBitMap *) ZNewPtr(sizeof(ZBitMap)));

	if (isColorMonitor) {
		PixMapHandle pmap;
		OSErr err;

		fakeColorDisp = false;
		saveGDev = gConfig.gdev;
		myGDev = NewGDevice((**saveGDev).gdRefNum, (**saveGDev).gdMode);
		SetGDevice(myGDev);
		
		pmap = (**myGDev).gdPMap;
		zPage1BitMap->baseAddr = (long *) (**pmap).baseAddr;	/* might want to use GetPixBaseAddr */
		zPage1BitMap->rowBytes = (**pmap).rowBytes & 0x3FFF;	/* mask out status bits */
		zPage1BitMap->xBound = (**pmap).bounds.right - (**pmap).bounds.left;
		zPage1BitMap->yBound = (**pmap).bounds.bottom - (**pmap).bounds.top;
		zPage1BitMap->qdRowBytes = (**pmap).rowBytes;
		CInitBitMap(zPage1BitMap);
		
		/* Handle driver errors -- FIX */

		videoParam.ioCompletion = NIL;
		videoParam.ioCRefNum = (**myGDev).gdRefNum;
		*(void **)(&videoParam.csParam) = &modeRecord;
		videoParam.csCode = cscGetMode;	
		err = PBStatus((ParmBlkPtr) &videoParam, 0);
		videoParam.csCode = cscGetPageCnt;
		err = PBStatus((ParmBlkPtr) &videoParam, 0);
		if (modeRecord.csPage > 1) {
			FailNIL(zPage2BitMap = (ZBitMap *) ZNewPtr(sizeof(ZBitMap)));
			modeRecord.csPage = 1;
			videoParam.csCode = cscGetBaseAddr;
			err = PBStatus((ParmBlkPtr) &videoParam, 0);
			zPage2BitMap->baseAddr = (long *) modeRecord.csBaseAddr;
			zPage2BitMap->rowBytes = zPage1BitMap->rowBytes;
			zPage2BitMap->xBound = zPage1BitMap->xBound;
			zPage2BitMap->yBound = zPage1BitMap->yBound;
			zPage2BitMap->qdRowBytes = zPage1BitMap->qdRowBytes;
			CInitBitMap(zPage2BitMap);
			}
		else
			zPage2BitMap = NIL;
		}
	else {
		fakeColorDisp = true;
		zPage1BitMap->rowBytes = qd.screenBits.bounds.right / 8;
		zPage1BitMap->xBound = qd.screenBits.bounds.right / 2;
		zPage1BitMap->yBound = qd.screenBits.bounds.bottom;
		zPage1BitMap->baseAddr = (long *) qd.screenBits.baseAddr;
		zPage1BitMap->qdRowBytes = 0;
		CInitBitMap(zPage1BitMap);
		zPage2BitMap = NIL;
		}

	ZSetBitMap(zPage1BitMap);
	ZSetShade(10);

	/*
		Assign colors for 3D display. NOTE: Equally bright shades of
		red and green seem to result in a much brighter green.
	*/

	PercentColor(&rgbBlack,	0,		0,		0);
	PercentColor(&rgbRed,	100,	0,		0);
	PercentColor(&rgbGreen,	0,		55,		0);
	PercentColor(&rgbAmber,	95,		52,		0);

}

void CPrepareDevice ()
{
	if (!gConfig.isGameCLUTInstalled) {
		short i;
	
		reqList[0] = 3;
		for (i=0; i<4; i++)
			reqList[i+1] = i;
		SaveEntries(0, systemCTab, (ReqListRec *) reqList);
		for (i=0; i<4; i++)
			ProtectEntry(i, false);
		
		depthColors[0].rgb = rgbBlack;
		depthColors[1].rgb = rgbGreen;
		depthColors[2].rgb = rgbRed;
		depthColors[3].rgb = rgbAmber;
		for (i=0; i<4; i++)
			depthColors[i].value = i;
		SetEntries(0, 3, depthColors);
		
		for (i=0; i<4; i++)
			ProtectEntry(i, true);
		
		gConfig.isGameCLUTInstalled = true;
	
		}
}

void CCleanUpDevice (
	long inhibitExceptions )
{
	/*
		Restore the old colors.
		Should only be called after CPrepareDevice.
	*/
	
	
	if (gConfig.isGameCLUTInstalled) {
		SetGDevice(saveGDev);
		RestoreEntries((CTabHandle) systemCTab, NIL, (ReqListRec *) reqList);
		ShowCursor(); ShowCursor();
		gConfig.isGameCLUTInstalled = false;
		}

}

void CInitBitMap (
	ZBitMap *theBitMap )
{	
	GrafPort *holdPort;
	PixMapHandle qdpm = nil;			/* Handle to QuickDraw shadow PixMap */
	short lineNo;
	long lineAddr;
	long tSlot;

	theBitMap->theLines = nil;

	ZSetRect(&theBitMap->clipRect,
		0, 0, theBitMap->xBound, theBitMap->yBound);
	theBitMap->xOrig = theBitMap->yOrig = 0;
	theBitMap->maskPtr = (long *) cAmberMasks;
	theBitMap->fullMask = (*cAmberMasks)[0][15];

	FailNIL(theBitMap->theLines = (long *((*)[])) ZNewPtr(4*(theBitMap->yBound)));
	lineAddr = (long) theBitMap->baseAddr;
	for (lineNo = 0; lineNo < theBitMap->yBound; lineNo++) {
		(*theBitMap->theLines)[lineNo] = (long *) lineAddr;
		lineAddr += theBitMap->rowBytes;
		}

	/*
		This assumes the current gDevice is already set up.
		Now set up QuickDraw stuff. (Note that for screen bitmaps, a separate
		qdRowBytes holds the rowBytes to preserve QuickDraw flags).
	*/

	holdPort = qd.thePort;
	OpenCPort((CGrafPtr) &theBitMap->qdPort);
	FailNIL(qdpm = NewPixMap());
	(**qdpm).baseAddr = (Ptr) theBitMap->baseAddr;
	if (theBitMap->qdRowBytes)
		(**qdpm).rowBytes = theBitMap->qdRowBytes;
	else
		(**qdpm).rowBytes = theBitMap->rowBytes;
	(**qdpm).pixelSize = 2;
	SetRect(&(**qdpm).bounds, 0, 0,
		theBitMap->rowBytes * 8, theBitMap->yBound);
	SetPortPix(qdpm);
	
	SetRect(&theBitMap->qdPort.portRect, 0, 0,
		theBitMap->rowBytes * 4, theBitMap->yBound);
	RectRgn(theBitMap->qdPort.clipRgn, &theBitMap->qdPort.portRect); 
	RectRgn(theBitMap->qdPort.visRgn, &theBitMap->qdPort.portRect); 
	RGBForeColor(&rgbBlack);
	RGBBackColor(&rgbAmber);
	
	SetPort(holdPort);
}

ZBitMap *CNewBitMap (
	short xBound, short yBound )
{
	long needMem;
	short rowLongs;
	ZBitMap *newBitMap = nil;

	if ((xBound > 0) && (yBound > 0)) {
		FailNIL(newBitMap = (ZBitMap *) ZNewPtr((sizeof(ZBitMap))));
		newBitMap->baseAddr = nil;
		
		newBitMap->yBound = yBound;
		rowLongs = (xBound - 1) / 16 + 1;
		newBitMap->rowBytes = rowLongs * 4;
		newBitMap->xBound = xBound;
		needMem = (long) newBitMap->rowBytes * (long) newBitMap->yBound;
		FailNIL(newBitMap->baseAddr = (long *) ZNewPtr(needMem));
		newBitMap->qdRowBytes = 0;
		CInitBitMap(newBitMap);
		return (newBitMap);
		}
	else
		return (NIL);
}

ZBitMap *CDupBitMap (
	ZBitMap *srcBitMap )
{
	ZBitMap *holdBM, *newBitMap = nil;

	holdBM = zCurBitMap;

	FailNIL(newBitMap = (ZBitMap *) ZNewPtr(sizeof(ZBitMap)));
	newBitMap->baseAddr = srcBitMap->baseAddr;
	newBitMap->xBound = srcBitMap->xBound;
	newBitMap->yBound = srcBitMap->yBound;
	newBitMap->rowBytes = srcBitMap->rowBytes;
	newBitMap->maskPtr = srcBitMap->maskPtr;
	newBitMap->fullMask = srcBitMap->fullMask;
	newBitMap->bitDepth = srcBitMap->bitDepth;
	newBitMap->qdRowBytes = srcBitMap->qdRowBytes;

	CInitBitMap(newBitMap);
	ZSetBitMap(newBitMap);
	ZSetOrigin(srcBitMap->xOrig, srcBitMap->yOrig);
	ZSetAbsClipRect(&srcBitMap->clipRect);
	ZSetBitMap(holdBM);
	
	return (newBitMap);
}

void CFreeBitMap (
	ZBitMap *bitMap )
{
	ClosePort(&bitMap->qdPort);
	ForgetPtr(bitMap->baseAddr);
	ForgetPtr(bitMap->theLines);
	ForgetPtr(bitMap);
}

void CSetColor (
	short color )
{
	switch(color) {
		case kRed:
			zCurBitMap->maskPtr = (long *) cRedMasks;
			zCurBitMap->fullMask = (*cRedMasks)[0][15];
			break;	
		case kGreen:
			zCurBitMap->maskPtr = (long *) cGreenMasks;
			zCurBitMap->fullMask = (*cGreenMasks)[0][15];
			break;
		case kAmber:
			zCurBitMap->maskPtr = (long *) cAmberMasks;
			zCurBitMap->fullMask = (*cAmberMasks)[0][15];
			break;
		}
}

short CGetPixel (
	short x, short y )
{
	/*
	
		Returns the value of the pixel (0 to 3) which is below
		and to the right of the point (x, y). This routine does NOT
		take into account the current color mask -- it simply returns
		the absolute value of the specified two-bit pixel. The green bit
		is the lowest order bit; the red bit is the next higher order
		bit. When both red and green bits are set, the pixel is amber.
		
		When the requested pixel falls outside the bounds of the current
		bitmap, the result of this routine is undefined.
		
	*/

	asm 68000 {
		movem.l	d3/d7, -(sp)
		move.w	x, d0
		move.w	y, d1

		/*	d0 = x
			d1 = y */

		/*
			Check to see if the pixel falls within the clipping rectangle.
			If not, the result is undefined.
		*/
		
		/* a0 = address of current ZBitMap record */
		
		move.l	zCurBitMap, a0
		sub.w	OFFSET(ZBitMap, xOrig) (a0), d0			/* too far right */
		cmp.w	OFFSET(ZBitMap, xBound) (a0), d0		
		bge		@out
		
		tst.w	d0										/* too far left */
		bmi		@out

		sub.w	OFFSET(ZBitMap, yOrig) (a0), d1			/* too far down */
		cmp.w	OFFSET(ZBitMap, yBound) (a0), d1
		bge		@out

		tst.w	d1										/* too far up */
		bmi		@out

		/*
			Coordinates are already pixel numbers -- no adjustment needed
			Now divide by 8 to obtain word & bit nos.
		*/
		
		move.w	d0, d2
		and.w	#0x0007, d2		/* d2.w = pixel no. within word */
		lsl.w	#1, d2			/* low order bit = 14 - (2 * pixelNum) */
		move.w	#14, d3			
		sub.w	d2, d3			/* d3.w = low order bit no. of pixel */
		and.w	#0xFFF8, d0
		lsr.w	#2, d0			/* d0.w = number of bytes to add to */
								/* row address for word */

		move.l	OFFSET(ZBitMap, theLines) (a0), a1
								/* a1 = address of theLines array */
		lsl.w	#2, d1			/* d1.w = index into theLines */
		move.l	0(a1,d1.w), a1
		adda.w	d0, a1			/* a1 = address of word */

		move.w	(a1), d0
		move.w	#3, d2			/* Extract the pixel bits and put */
		lsl.w	d3, d2			/* them into the two low-order bits */
		and.w	d2, d0			/* of d0, which is the return value */
		lsr.w	d3, d0
@out:
		movem.l	(sp)+, d3/d7
	}

}

void CSetPixel (
	short x, short y,
	short value )
{
	/*
	
		Sets the pixel which is below and to the right of the point (x, y).
		Like other CGraf routines, CSetPixel takes into account the color
		mask and ORs the pixel into the bitmap. For example, if an existing
		pixel is red and the current mask is green, calling CSetPixel with
		value = true will result in that pixel being turned amber.
		
	*/
	
	asm 68000 {
		movem.l	d3, -(sp)
		move.w	x, d0
		move.w	y, d1

		/*
			Check to see if the pixel falls within the clipping rectangle.
			Otherwise nothing is drawn.
		*/
		
		/* a0 = address of current ZBitMap record */
		
		move.l	zCurBitMap, a0
		sub.w	OFFSET(ZBitMap, xOrig) (a0), d0			/* too far right */
		cmp.w	OFFSET(ZBitMap, clipRect.right) (a0), d0		
		bge		@noDraw
		
		cmp.w	OFFSET(ZBitMap, clipRect.left) (a0), d0	/* too far left */
		blt		@noDraw

		sub.w	OFFSET(ZBitMap, yOrig) (a0), d1			/* too far down */
		cmp.w	OFFSET(ZBitMap, clipRect.bottom) (a0), d1
		bge		@noDraw

		cmp.w	OFFSET(ZBitMap, clipRect.top) (a0), d1	/* too far up */
		blt		@noDraw
				
		/*
			Coordinates are already pixel numbers -- no adjustment needed
			Now divide by 8 to obtain word & pixel nos.
		*/

		move.w	d0, d2
		and.w	#0x0007, d2		/* d2.w = pixel no. within word */
		lsl.w	#1, d2			/* low order bit = 14 - (2 * pixelNum) */
		move.w	#14, d3			
		sub.w	d2, d3			/* d3.w = low order bit no. of pixel */
		and.w	#0xFFF8, d0
		lsr.w	#2, d0			/* d0.w = number of bytes to add to */
								/* row address for word */

		move.l	OFFSET(ZBitMap, theLines) (a0), a1
								/* a1 = address of theLines array */
		lsl.w	#2, d1			/* d1.w = index into theLines */
		move.l	0(a1,d1.w), a1
		adda.w	d0, a1			/* a1 = address of byte */

		move.w	OFFSET(ZBitMap, fullMask) (a0), d2
		and.w	#3, d2
		lsl.w	d3, d2			/* move low-order bits into proper position */
		tst.w	value
		bne		@pixelOn
@pixelOff:
		not.w	d2
		and.w	d2, (a1)
		bra		@out
@pixelOn:
		or.w	d2, (a1)
@noDraw:
@out:
		movem.l	(sp)+, d3
	}
}

void CHLine (
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
 								/* boundary to right (as per QD lines ) */
 	
		cmp.w	d0, d2			/* if x1 <= x0, line is invisible */
		ble		@noDraw			/* or invalid */
		
		/* The line is valid and possibly visible; check to see */
		/* if any of it falls within the clipping rectangle */
		
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
		
		/* division by 16 via masking & shifting */
		/* to obtain begin/end addr & pixel nos. */
		
		move.w	d0, d4
		and.w	#0x000F, d4		/* d4.w = beginPixel */
		and.w	#0xFFF0, d0
		lsr.w	#2, d0			/* d0.w = number of bytes to add to */
								/* row address for beginLong */
		move.w	d2, d5
		and.w	#0x000F, d5		/* d5.w = endPixel */
		and.w	#0xFFF0, d2
		lsr.w	#2, d2			/* d2.w = number of bytes to add to */
								/* row address for endLong */

		move.l	OFFSET(ZBitMap, theLines) (a0), a1
		move.l	d1, d7			/* a1 = address of theLines array */
		lsl.w	#2, d7			/* d7.w = index into theLines */
		move.l	0(a1,d7.w), a2
		adda.w	d0, a2			/* a2 = address of beginLong */
		
		/* retrieve begin/end masks and zCurPat address */
		
		move.l	OFFSET(ZBitMap, maskPtr) (a0), a1
		move.l	zCurPat, a3 
		and.w	#0x0007, d1		/* low 5 bits of vert coord determine */
								/* index into pattern */
		lsl.w	#2, d1			/* valid indices: 0, 4, 8, 12... */
		move.w	d2, d6			/* a3 = address of pattern */
		sub.w	d0, d6			/* d6 = difference (bytes) between */
		bgt		@notEqual		/* beginLong and endLong */
		
		/* begin and end long are the same */	

		lsl.w	#6, d4
		lsl.w	#2, d5			/* get zMaskLongs[beginPixel][endPixel] */
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

		lsl.w	#6, d4
		move.l	60(a1, d4.w), d4	/* d4 holds mask for left long */
		move.l	d4, d2				/*    -- zMaskLongs[beginPixel][15] */
		not.l	d2
		lsl.w	#2, d5	
		move.l	0(a1, d5.w), d5		/* d5 holds mask for right long */
									/*    -- zMaskLongs[0][endPixel] */
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
		lsl.w	#6, d4				/* d4 holds mask for left long */
		move.l	60(a1, d4.w), d4	/*    -- zMaskLongs[beginLong][31] */				
		lsl.w	#2, d5				/* d5 holds mask for right long */
		move.l	0(a1, d5.w), d5		/*    -- zMaskLongs[0][endLong] */

		subq.w	#4, d6				/* get difference in terms */
									/* of instructions (for jmp); */
		lea		@finish, a1			/* two bytes per long to be moved */
		suba.w	d6, a1
		move.l	d4, d0
		not.l	d0
		move.l	d5, d3
		not.l	d3
		move.l	0(a3, d1.w), d2		/* retrieve pattern long */
		move.l	OFFSET(ZBitMap, fullMask) (a0), d6
		not.l	d6					/* d6 holds inverse of mask -- */
									/* these are the bits to leave alone */
		move.l	d2, d7				/* put a copy in d7 */
		and.l	d4, d7
		and.l	d0, (a2)
		or.l	d7, (a2)+
		move.l	d2, d7				/* get clean pattern long */
		and.l	OFFSET(ZBitMap, fullMask) (a0), d7
 		jmp		(a1)				

		and.l	d6, (a2)	/* 100 longs (1600 pixels, the max) */
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+

		and.l	d6, (a2)	/* 80 longs */
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+

		and.l	d6, (a2)	/* 70 longs */
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+

		and.l	d6, (a2)	/* 60 longs */
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+

		and.l	d6, (a2)	/* 50 longs */
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+

		and.l	d6, (a2)	/* 40 longs */
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+

		and.l	d6, (a2)	/* 30 longs */
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+

		and.l	d6, (a2)	/* 20 longs */
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+

		and.l	d6, (a2)	/* 10 longs */
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
		and.l	d6, (a2)
		or.l	d7, (a2)+
@finish:
		and.l	d5, d7
		and.l	d3, (a2)
		or.l	d7, (a2)+
@out:
@noDraw:
		movem.l	(sp)+,d3-d7/a2-a4
	}
}

void CVLine (
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
			
		addq.w	#1, d3			/* As per QD lines, includes end pixels */

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
		/* to obtain begin/end addr & pixel nos. */
		
		move.w	d0, d7
		and.w	#0x0007, d7		/* d4.w = pixel no. within word */
		lsl.w	#1, d7			/* low order bit = 14 - (2 * pixelNum) */
		move.w	#14, d4			
		sub.w	d7, d4			/* d4.w = low order bit no. of pixel */
		and.w	#0xFFF8, d0
		lsr.w	#2, d0			/* d0.w = number of bytes to add to */
								/* row address for beginWord */

		move.l	OFFSET(ZBitMap, theLines) (a0), a1
		move.l	d1, d7			/* a1 = address of theLines array */
		lsl.w	#2, d7			/* d7.w = index into theLines */
		move.l	0(a1,d7.w), a2
		adda.w	d0, a2			/* a2 = address of beginWord on first line */
		lsl.w	#2, d3
		move.l	0(a1,d3.w),	a4
		adda.w	d0, a4			/* a4 = address of beginWord on last line */
		
		/* Retrieve pattern address */
		
		move.l	zCurPat, a3 
		and.w	#0x0007, d1		/* low 5 bits of top coord determine */
								/* starting index into pattern */
		lsl.w	#2, d1			/* valid indices: 0, 4, 8, 12... */
								/* a3 = address of pattern */
		move.w	#128, d6		/* d6 = overflowed index into pattern */
		/*
			Since the low word of the pattern long is always the same as 
			the high word, we will always use the high word.
		*/

		move.w	OFFSET(ZBitMap, fullMask) (a0), d2
		and.w	#3, d2
		lsl.w	d4, d2		/* move low-order bits into proper position */
		move.w	d2, d4
		not.w	d4
		
		/*
			d2 contains the mask for each word of the line being drawn.
			(only bits which are on can be set). d4 contains its inverse
			(the bits which are to be left alone in each word are set).
		*/
		
		move.w	OFFSET(ZBitMap, rowBytes) (a0), a1
@back:
		move.w	0(a3, d1.w), d3		/* d3 holds pattern word */
		and.w	d2, d3 
		and.w	d4, (a2)
		or.w	d3, (a2)
		addq.w	#4, d1
		cmp.w	d6, d1
		bne		@down
		clr.w	d1
@down:
		adda.w	a1, a2
		cmpa.l	a4, a2
		ble		@back
@out:
@noDraw:
		movem.l	(sp)+,d3-d7/a2-a4
	}
}

void CVSLine (
	short top, short bottom,
	short x,
	short value,
	short skipFactor )
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
		/* division by 16 via masking & shifting */
		/* to obtain begin/end addr & pixel nos. */
		
		move.w	d0, d7
		and.w	#0x0007, d7		/* d4.w = pixel no. within word */
		lsl.w	#1, d7			/* low order bit = 14 - (2 * pixelNum) */
		move.w	#14, d4			
		sub.w	d7, d4			/* d4.w = low order bit no. of pixel */
		and.w	#0xFFF8, d0
		lsr.w	#2, d0			/* d0.w = number of bytes to add to */
								/* row address for word */

		move.l	OFFSET(ZBitMap, theLines) (a0), a1
		move.l	d1, d7			/* a1 = address of theLines array */
		lsl.w	#2, d7			/* d7.w = index into theLines */
		move.l	0(a1, d7.w), a2
		adda.w	d0, a2			/* a2 = address of word on first line */
		lsl.w	#2, d3
		move.l	0(a1, d3.w), a4
		adda.w	d0, a4			/* a4 = address of word on last line */
		
		move.w	OFFSET(ZBitMap, fullMask) (a0), d2
		and.w	#3, d2
		lsl.w	d4, d2		/* move low-order bits into proper position */
		move.w	d2, d6
		not.w	d6
		
		move.w	OFFSET(ZBitMap, rowBytes) (a0), d4
		move.w	skipFactor, d5
		lsl.w	d5, d4
		tst.w	value
		beq		@blackLine

@whiteLine:
		or.w	d2, (a2)
		adda.w	d4, a2
		cmpa.l	a4, a2
		ble		@whiteLine
		bra		@out

@blackLine:
		and.w	d6, (a2)
		adda.w	d4, a2
		cmpa.l	a4, a2
		ble		@blackLine
@out:
@noDraw:
		movem.l	(sp)+,d3-d7/a2-a4
	}
}

void CFrameRect (
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
				
		/* CHLine(left, right, top) */
		
		move.w	d4, -(sp)
		move.w	d5, -(sp)
		move.w	d3, -(sp)
		bsr		CHLine
		addq.l	#6, sp
		
		/* CHLine(left, right, bottom) */
		
		move.w	d6, -(sp)
		move.w	d5, -(sp)
		move.w	d3, -(sp)
		bsr		CHLine
		addq.l	#6, sp
		
		/* If pattern is either black or white, use CVSLine for speed */
		
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
		/* CVSLine(top, bottom, left, value, skipFactor) */

		move.w	#0, -(sp)
		move.w	d7, -(sp)
		move.w	d3, -(sp)
		move.w	d6, -(sp)
		move.w	d4, -(sp)
		bsr		CVSLine
		lea		10(sp), sp
		
		/* CVSLine(top, bottom, right, value, skipFactor) */
		
		move.w	#0, -(sp)
		move.w	d7, -(sp)
		move.w	d5, -(sp)
		move.w	d6, -(sp)
		move.w	d4, -(sp)
		bsr		CVSLine
		lea		10(sp), sp
		bra		@out

@patLine:
		/* CVLine(top, bottom, left) */
		
		move.w	d3, -(sp)
		move.w	d6, -(sp)
		move.w	d4, -(sp)
		bsr		CVLine
		addq.l	#6, sp
		
		/* CVLine(top, bottom, right) */
		
		move.w	d5, -(sp)
		move.w	d6, -(sp)
		move.w	d4, -(sp)
		bsr		CVLine
		addq.l	#6, sp
@noDraw:
@out:
		movem.l	(sp)+, d3-d7
	}
}

void CFillRect (
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
		
		/* division by 16 via masking & shifting */
		/* to obtain begin/end addr & pixel nos. */
		
		move.w	d0, d4
		and.w	#0x000F, d4		/* d4.w = begin pixel */
		and.w	#0xFFF0, d0
		lsr.w	#2, d0			/* d0.w = number of bytes to add to */
								/* row address for beginLong */

		move.w	d2, d5
		and.w	#0x000F, d5		/* d5.w = end pixel */
		and.w	#0xFFF0, d2
		lsr.w	#2, d2			/* d2.w = number of bytes to add to */
								/* row address for endLong */

		move.l	OFFSET(ZBitMap, theLines) (a0), a1
		move.l	d1, d7			/* a1 = address of theLines array */
		lsl.w	#2, d7			/* d7.w = index into theLines */
		move.l	0(a1,d7.w), a2
		adda.w	d0, a2			/* a2 = address of beginLong on first line */
		lsl.w	#2, d3
		move.l	0(a1,d3.w),	a4
		adda.w	d0, a4			/* a4 = address of beginLong on last line */
		
		/* retrieve begin/end masks and zCurPat address */
		
		move.l	OFFSET(ZBitMap, maskPtr) (a0), a1
		move.l	zCurPat, a3 
		and.w	#0x0007, d1		/* low 5 bits of top coord determine */
								/* index into pattern */
		lsl.w	#2, d1			/* valid indices: 0, 4, 8, 12... */
		move.w	d2, d6			/* a3 = address of pattern */
		sub.w	d0, d6			/* d6 = difference (bytes) between */
		bgt		@notEqual		/* beginLong and endLong */
		
		/* begin and end long are the same */	
		
		lsl.w	#6, d4
		lsl.w	#2, d5				/* array index: zMaskLongs[begin][end] */
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

		lsl.w	#6, d4				
		move.l	60(a1, d4.w), d4	/* d4 holds mask for left long */
		move.l	d4, d2				/*    -- zMaskLongs[beginPixel][15] */
		not.l	d2
		lsl.w	#2, d5	
		move.l	0(a1, d5.w), d5		/* d5 holds mask for right long */
									/*    -- zMaskLongs[0][endPixel] */
		move.l	d5, d3
		not.l	d3
		move.w	OFFSET(ZBitMap, rowBytes) (a0), d6
		subq.w	#4, d6
									/* one long less than one line */
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
		lsl.w	#6, d4				/* d4 holds mask for left long */
		move.l	60(a1, d4.w), d4	/*    -- zMaskLongs[beginLong][31] */				
		lsl.w	#2, d5				/* d5 holds mask for right long */
		move.l	0(a1, d5.w), d5		/*    -- zMaskLongs[0][endLong] */

		subq.w	#4, d6				/* get difference in terms */
									/* of instructions (for jmp); */
		lea		@finish, a1			/* four bytes per long to be moved */
		suba.w	d6, a1
		move.w	OFFSET(ZBitMap, rowBytes) (a0), d6
		sub.w	d2, d6
		add.w	d0, d6
		move.l	d4, d0
		not.l	d0
		move.l	d5, d3
		not.l	d3
		move.l	OFFSET(ZBitMap, fullMask) (a0), d2
		not.l	d2
@back3:
		move.l	0(a3, d1.w), d7		/* retrieve pattern long */
		and.l	d4, d7
		and.l	d0, (a2)
		or.l	d7, (a2)+
		move.l	0(a3, d1.w), d7		/* retrieve clean pattern long */
		and.l	OFFSET(ZBitMap, fullMask) (a0), d7
		jmp		(a1)

		and.l	d2, (a2)			/* 100 longs (1600 pixels, the max) */
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+

		and.l	d2, (a2)			/* 90 longs */
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+

		and.l	d2, (a2)			/* 80 longs */
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+

		and.l	d2, (a2)			/* 70 longs */
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+

		and.l	d2, (a2)			/* 60 longs */
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+

		and.l	d2, (a2)			/* 50 longs */
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+

		and.l	d2, (a2)			/* 40 longs */
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+

		and.l	d2, (a2)			/* 30 longs */
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+

		and.l	d2, (a2)			/* 20 longs */
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+

		and.l	d2, (a2)			/* 10 longs */
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
		and.l	d2, (a2)
		or.l	d7, (a2)+
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

void CCopyLongs(
	ZBitMap *srcBits, ZBitMap *dstBits,
	Rect *srcRectAbs,
	short xDestAbs, short yDestAbs )
{
	short lineLenBytes;

	/*
		Warning: CCopyLongs, like ZCopyLongs, performs no error 
		checking and no clipping. All coordinates passed to this
		routine, including srcRectAbs, xDestAbs, and yDestAbs,
		need to be specified in absolute (global coordinates).
	*/

	asm 68000 {
		movem.l	d3-d7/a2-a4, -(sp)
		move.l	srcRectAbs, a0
		move.w	OFFSET(Rect, left) (a0), d0
		move.w	OFFSET(Rect, top) (a0), d1
		move.w	OFFSET(Rect, right) (a0), d2
		move.w	OFFSET(Rect, bottom) (a0), d3
		

		/*	d0 = left
			d1 = top
			d2 = right
			d3 = bottom */

		move.l	srcBits, a0
		move.l	dstBits, a1

		/* convert from boundary lines to first/last pixels */
		subq.w	#1, d2
		subq.w	#1, d3
		
		/* division by 16 via shifting & masking */
		/* to obtain begin/end addr */

		and.w	#0xFFF0, d0
		lsr.w	#2, d0			/* d0.w = number of bytes to add to */
								/* row address for beginLong (src) */
		and.w	#0xFFF0, d2
		lsr.w	#2, d2			/* d2.w = number of bytes to add to */
								/* row address for endLong (src) */

		move.w	d2, lineLenBytes
		sub.w	d0, lineLenBytes

		move.w	xDestAbs, d5	/* d5.w = number of bytes to add to */
		and.w	#0xFFF0, d5		/* row address for beginLong (dest) */
		lsr.w	#2, d5	

		move.l	OFFSET(ZBitMap, theLines) (a0), a3
		lsl.w	#2, d1				/* d1.w = index into theLines */
		move.l	0(a3,d1.w), a2
		adda.w	d0, a2				/* a2 = address of beginLong */
									/* on first src line */
		lsl.w	#2, d3
		move.l	0(a3,d3.w),	a3
		adda.w	d0, a3				/* d4 = address of beginLong */
		move.l	a3, d4				/* on last src line */
									
		move.l	OFFSET(ZBitMap, theLines) (a1), a4
		move.w	yDestAbs, d7
		lsl.w	#2, d7				/* d7.w = index into theLines */
		move.l	0(a4,d7.w), a4
		adda.w	d5, a4				/* a4 = address of beginLong */
									/* on first dest line */
				
		move.w	d2, d6				/* d6 is the difference between begin */
		sub.w	d0, d6				/* and end long (bytes) */
		addq.w	#4, d6				/* One more long must be moved than */
									/* the difference */
		lsr #1, d6					/* two bytes executed per long moved */
		lea		@finish, a3
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
		
		move.l	(a2)+, (a4)+			/* 100 longs (1600 pixels) */
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+

		move.l	(a2)+, (a4)+			/* 90 longs */
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+

		move.l	(a2)+, (a4)+			/* 80 longs */
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+

		move.l	(a2)+, (a4)+			/* 70 longs */
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+

		move.l	(a2)+, (a4)+			/* 60 longs */
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+
		move.l	(a2)+, (a4)+

		move.l	(a2)+, (a4)+			/* 50 longs */
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
		movem.l	(sp)+, d3-d7/a2-a4
	}
}

void CClearLongs (
	Rect *rectAbs )
{

	/*
		Clears the specified rectangle from the current bitmap.
		Note: this routine assumes that pixels whose value is 0
		are black (clear), which is not the Apple convention.
		
		Warning: This routine performs no error checking and no
		clipping. The rectangle has to be in absolute (global)
		coordinates.
	*/

	asm 68000 {
		movem.l	d3-d7/a2-a4, -(sp)
		move.l	rectAbs, a0
		move.w	OFFSET(Rect, left) (a0), d0
		move.w	OFFSET(Rect, top) (a0), d1
		move.w	OFFSET(Rect, right) (a0), d2
		move.w	OFFSET(Rect, bottom) (a0), d3

		/*	d0 = left
			d1 = top
			d2 = right
			d3 = bottom */

		/* convert from boundary lines to first/last pixels */
		
		subq.w	#1, d2
		subq.w	#1, d3
		
		/* division by 16 via shifting & masking */
		/* to obtain begin/end addr */
	
		and.w	#0xFFF0, d0
		lsr.w	#2, d0			/* d0.w = number of bytes to add to */
								/* row address for beginLong (src) */
		and.w	#0xFFF0, d2
		lsr.w	#2, d2			/* d2.w = number of bytes to add to */
								/* row address for endLong (src) */

		move.l	zCurBitMap, a0
		move.l	OFFSET(ZBitMap, theLines) (a0), a3
		lsl.w	#2, d1				/* d1.w = index into theLines */
		move.l	0(a3,d1.w), a2
		adda.w	d0, a2				/* a2 = address of beginLong */
									/* on first line */
		lsl.w	#2, d3
		move.l	0(a3,d3.w),	a4
		adda.w	d0, a4				/* a4 = address of beginLong */
									/* on last line */
									
		move.w	d2, d6				/* d6 is the difference between begin */
		sub.w	d0, d6				/* and end long (bytes) */
		addq.w	#4, d6				/* One more long must be moved than */
									/* the difference */
		lsr.w	#1, d6
		lea		@finish, a3			/* Two bytes executed per long cleared */
		suba.w	d6, a3
		
		/* calculate how much should be added to */
		/* address after each line is moved */
		
		move.w	OFFSET(ZBitMap, rowBytes) (a0), d6
		sub.w	d2, d6
		add.w	d0, d6				/* add d6 to address */
	
@back3:
		jmp		(a3)

		clr.l	(a2)+	/* 100 Longs (1600 pixels) */
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+

		clr.l	(a2)+	/* 90 Longs */
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+

		clr.l	(a2)+	/* 80 Longs */
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+

		clr.l	(a2)+	/* 70 Longs */
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+

		clr.l	(a2)+	/* 60 Longs */
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+

		clr.l	(a2)+	/* 50 Longs */
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+

		clr.l	(a2)+	/* 40 Longs */
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+

		clr.l	(a2)+	/* 30 Longs */
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+

		clr.l	(a2)+	/* 20 Longs */
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+

		clr.l	(a2)+	/* 10 Longs */
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)+
		clr.l	(a2)
@finish:
		adda.w	d6, a2
		cmpa.l	a4, a2
		bgt		@out
		bra		@back3
@out:
		movem.l	(sp)+, d3-d7/a2-a4
	}
}