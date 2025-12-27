/*
 * ZGraf - ZShapes.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include "ZShapes.h"

ZShape *ZNewShape (
	short imageID,
	short xBound, short yBound,
	short whichShifts )
{
	ZShape *theShape;
	short bitNo;
	Rect drawRect, whiteRect;
	ZBitMap *holdBM;
	
	/*
		FIX: Add error checking.
	*/

	holdBM = zCurBitMap;

	theShape = (ZShape *) ZNewPtr(sizeof(ZShape));
	theShape->xBound = xBound;
	theShape->yBound = yBound;
	theShape->shapePict = (PicHandle) GetResource('PICT', imageID);
	theShape->maskPict = NIL;
	ZSetRect(&whiteRect, 0, 0, ((xBound + 15) / 32 + 1) * 32, yBound);
	for (bitNo = 0; bitNo < 16; bitNo++) {
		if (BitTst(&whichShifts, bitNo)) {
			ZSetRect(&drawRect, bitNo, 0, xBound + bitNo, yBound);
			theShape->shapes[bitNo] = ZNewBitMap(xBound + 15, yBound);
			ZSetBitMap(theShape->shapes[bitNo]);
			ZSetShade(10);					
			ZFillRect(&whiteRect);
			DrawPicture(theShape->shapePict, &drawRect);
/*
			if (maskID) {
				theShape->masks[bitNo] = ZNewBitMap(xBound + 15, yBound);
				ZSetBitMap(theShape->masks[bitNo]);
				ZSetShade(10);
				ZFillRect(&whiteRect);
				DrawPicture(theShape->maskPict, &drawRect);
				}
*/
			}
		else {
			theShape->shapes[bitNo] = NIL;
			theShape->masks[bitNo] = NIL;
			}
		}
	ZSetBitMap(holdBM);
	return(theShape);
}

ZShape *ZNewMaskShape (
	short pictID, short maskID,
	short xBound, short yBound,
	short whichShifts )
{
	// theShape->maskPict = (PicHandle) GetResource('PICT', maskID);
}

void ZNewShift (
	ZShape *shape,
	short whichShift )
{

}

void ZDrawShape (
	ZShape *shape,
	Rect *srcRect,
	short xDest, short yDest )
{

}

void ZDrawMaskShape (
	ZShape *shape,
	Rect *srcRect,
	short xDest, short yDest )
{

}

void ZFreeShape (
	ZShape *shape )
{
	short shiftNo;

	if (shape->maskPict) {
		ReleaseResource((Handle) shape->shapePict);
		ReleaseResource((Handle) shape->maskPict);
		for (shiftNo = 0; shiftNo < 16; shiftNo++) {
			if (shape->shapes[shiftNo]) {
				ZFreeBitMap(shape->shapes[shiftNo]);
				ZFreeBitMap(shape->masks[shiftNo]);
				}
			}
		}
	else {
		ReleaseResource((Handle) shape->shapePict);
		for (shiftNo = 0; shiftNo < 16; shiftNo++) {
			if (shape->shapes[shiftNo])
				ZFreeBitMap(shape->shapes[shiftNo]);
			}
		}
	ForgetPtr(shape);
}