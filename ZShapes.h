/*
 * ZGraf - ZShapes.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#define _H_ZShapes
#include "ZGraf.h"

/*
	The shape datatype consists of a series of ZBitMaps representing a
	set of primary images along with the same number of mask images,
	if the shape includes masks. The largest number of such images
	is 16 shapes and 16 masks. A given pair of shape and mask images
	corresponds to the appropriate bitshifted image for that shape.
	
	If the shape includes a mask, then only the pixels in the shape which
	correnspond to black (on) pixels in the mask will be drawn when
	the shape is drawn.
	
	When a shape is created, the caller specifies which shifts to 
	precalculate. If all possible shifts are desired, kAllShifts should
	be passed. Otherwise, the caller should set the appropriate bits in
	the whichShifts parameter such that only the desired shifts have the
	corresponding bit �on.� For example, if the shape will only be drawn
	at absolute horizontal coordinate 18, requiring shift 2 (shift = 
	xCoord % 16), then only bit two of the whichShifts should be set.
	In this context, bits are numbered starting with 0 for the high-order
	bit and 15 with the lowest order bit (that is, pixel numbers within
	a word, not 68000 bit numbers).
	
	Shifts are cached. If a request is given to draw a shape in a position
	which requires a non-existent bit shift, that image will be created
	and entered into the shape table. Subsequent requests to draw that 
	shape at the same bitshift will be much more efficient.

*/

/* Constants */

#define kAllShifts	0xFFFF
#define kNoShifts	0x0000

/* Datatypes */

typedef struct {
	short xBound, yBound;
	PicHandle shapePict;			/* Shape PICT resource ID */
	PicHandle maskPict;				/* Mask PICT resource ID */
	ZBitMap *shapes[16];			/* any of these which hold NIL indicate */
									/* that there is currently no image for */
									/* that bit shift  -- shift 0 is for */
									/* the leftmost coordinate in a word */
	ZBitMap *masks[16];
	} ZShape;
	
/* Function prototypes */

ZShape *ZNewShape(short pictID,
	short xBound, short yBound, short whichShifts);
ZShape *ZNewMaskShape(short pictID, short maskID,
	short xBound, short yBound, short whichShifts);
void ZNewShift(ZShape *shape, short whichShift);
void ZDrawShape(ZShape *shape, Rect *srcRect, short xDest, short yDest);
void ZDrawMaskShape(ZShape *shape, Rect *srcRect, short xDest, short yDest);
void ZFreeShape(ZShape *shape);
