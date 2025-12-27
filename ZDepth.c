/*
 * ZGraf - ZDepth.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include "ZDepth.h"

short xEyePos, yEyePos, zEyePos;
short eyeToScreen, eyeOffset, origOffset;
short halfEyeOffset;
FRect tunnelRect;

void DInit (
	short eyeDist,
	short eyeOff,
	short origOff )
{
	eyeToScreen = eyeDist;
	eyeOffset = eyeOff;
	origOffset = origOff;
	
	halfEyeOffset = eyeOffset / 2;
}

/*

-- don't think this does any good

Fixed S2F (
	short s )
{
	asm 68000 {
		clr.l	d0
		move.w	s, d0
		swap	d0
		}
}
*/

short F2S (
	Fixed f )
{
	asm 68000 {
		move.w	f, d0		// Extracts hi-order word
		}
}

void SetFRect (
	FRect *r,
	Fixed left, Fixed top,
	Fixed right, Fixed bottom)
{
	asm 68000 {
		move.l	r, a0
		move.l	left, OFFSET(FRect, left) (a0)
		move.l	top, OFFSET(FRect, top) (a0)
		move.l	right, OFFSET(FRect, right) (a0)
		move.l	bottom, OFFSET(FRect, bottom) (a0)
		}
}

void OffsetFRect (
	FRect *r,
	Fixed xOff,
	Fixed yOff )
{
	r->left += xOff;
	r->right += xOff;
	r->top += yOff;
	r->bottom += yOff;
}

void SetBox (
	Box *b,
	Fixed left, Fixed top, Fixed front,
	Fixed right, Fixed bottom, Fixed back )
{
	b->left = left;
	b->top = top;
	b->front = front;
	b->right = right;
	b->bottom = bottom;
	b->back = back;
}

void DHLine (
	Fixed left, Fixed right,
	Fixed y,
	Fixed z )
{
	asm 68000 {
		movem.l	d3, -(sp)
		move.w	left, d0		/* Extract only high order word of */
		move.w	right, d2		/* coordinates */
		move.w	y, d1
		move.w	z, d3

		sub.w	zEyePos, d3
		
		sub.w	xEyePos, d0
		muls	eyeToScreen, d0
		divs	d3, d0					/* d0.w = leftDraw */
		
		sub.w	xEyePos, d2
		muls	eyeToScreen, d2
		divs	d3, d2					/* d2.w = rightDraw */
		
		sub.w	yEyePos, d1
		muls	eyeToScreen, d1
		divs	d3, d1					/* d1.w = yDraw */
		
		move.w	d1, -(sp)
		move.w	d2, -(sp)
		move.w	d0, -(sp)
		bsr		ZHLine
		addq.l	#6, sp
		movem.l	(sp)+, d3
		}
}

void DVLine (
	Fixed top, Fixed bottom,
	Fixed x,
	Fixed z )
{
	asm 68000 {
		movem.l	d3, -(sp)
		move.w	x, d0
		move.w	top, d1
		move.w	bottom, d3
		move.w	z, d2
		
		sub.w	zEyePos, d2

		sub.w	yEyePos, d1
		muls	eyeToScreen, d1
		divs	d2, d1					/* d1.w = topDraw */
		
		sub.w	yEyePos, d3
		muls	eyeToScreen, d3
		divs	d2, d3					/* d3.w = bottomDraw */
		
		sub.w	xEyePos, d0
		muls	eyeToScreen, d0
		divs	d2, d0					/* d0.w = xDraw */
		
		move.w	d0, -(sp)
		move.w	d3, -(sp)
		move.w	d1, -(sp)
		bsr		ZVLine
		addq.l	#6, sp
		movem.l	(sp), d3
		}
}

void DVSLine (
	Fixed top, Fixed bottom,
	Fixed x,
	Fixed z,
	short color,
	short skipFactor)
{
	asm 68000 {
		movem.l	d3, -(sp)
		move.w	x, d0
		move.w	top, d1
		move.w	bottom, d3
		move.w	z, d2
		
		sub.w	zEyePos, d2

		sub.w	yEyePos, d1
		muls	eyeToScreen, d1
		divs	d2, d1					/* d1.w = topDraw */
		
		sub.w	yEyePos, d3
		muls	eyeToScreen, d3
		divs	d2, d3					/* d3.w = bottomDraw */
		
		sub.w	xEyePos, d0
		muls	eyeToScreen, d0
		divs	d2, d0					/* d0.w = xDraw */
		
		move.w	skipFactor, -(sp)
		move.w	color, -(sp)
		move.w	d0, -(sp)
		move.w	d3, -(sp)
		move.w	d1, -(sp)
		bsr		ZVSLine
		lea		10(sp), sp				/* clean up stack */
		movem.l	(sp)+, d3
		}
}

void DFrameRect (
	FRect *r,
	Fixed z )
{
	Rect drawRect;
	
	asm 68000 {
		movem.l	d3-d4, -(sp)
		move.l	r, a0
		move.w	OFFSET(FRect, left) (a0), d0
		move.w	OFFSET(FRect, top) (a0), d1
		move.w	OFFSET(FRect, right) (a0), d2
		move.w	OFFSET(FRect, bottom) (a0), d3
		move.w	z, d4
		sub.w	zEyePos, d4
		
		sub.w	xEyePos, d0
		muls	eyeToScreen, d0
		divs	d4, d0
		move.w	d0, drawRect.left
		
		sub.w	yEyePos, d1
		muls	eyeToScreen, d1
		divs	d4, d1
		move.w	d1, drawRect.top
		
		sub.w	xEyePos, d2
		muls	eyeToScreen, d2
		divs	d4, d2
		move.w	d2, drawRect.right
		
		sub.w	yEyePos, d3
		muls	eyeToScreen, d3
		divs	d4, d3
		move.w	d3, drawRect.bottom
		
		pea		drawRect
		bsr		ZFrameRect
		clr.l	(sp)+
		movem.l	(sp)+, d3-d4
		}
}

void DFillRect (
	FRect *r,
	Fixed z )
{
	Rect drawRect;
	
	asm 68000 {
		movem.l	d3-d4, -(sp)
		move.l	r, a0
		move.w	OFFSET(FRect, left) (a0), d0
		move.w	OFFSET(FRect, top) (a0), d1
		move.w	OFFSET(FRect, right) (a0), d2
		move.w	OFFSET(FRect, bottom) (a0), d3
		move.w	z, d4
		sub.w	zEyePos, d4

		sub.w	xEyePos, d0
		muls	eyeToScreen, d0
		divs	d4, d0
		move.w	d0, drawRect.left

		sub.w	yEyePos, d1
		muls	eyeToScreen, d1
		divs	d4, d1
		move.w	d1, drawRect.top

		sub.w	xEyePos, d2
		muls	eyeToScreen, d2
		divs	d4, d2
		move.w	d2, drawRect.right

		sub.w	yEyePos, d3
		muls	eyeToScreen, d3
		divs	d4, d3
		move.w	d3, drawRect.bottom

		pea		drawRect
		bsr		ZFillRect
		clr.l	(sp)+
		movem.l	(sp)+, d3-d4
		}
}

void DSetPixel (
	Fixed x, Fixed y,
	Fixed z,
	short color )
{
	asm 68000 {
		move.w	x, d0
		move.w	y, d1
		move.w	z, d2
				
		sub.w	zEyePos, d2

		sub.w	xEyePos, d0
		muls	eyeToScreen, d0
		divs	d2, d0		
		
		sub.w	yEyePos, d1
		muls	eyeToScreen, d1
		divs	d2, d1
		
		move.w	color, -(sp)
		move.w	d1, -(sp)
		move.w	d0, -(sp)
		bsr		ZSetPixel
		addq.l	#6, sp
		}
}

void DTunnelRect (
	Fixed z )
{
	asm 68000 {
		movem.l	d3-d7, -(sp)
		move.w	z, d7				/* Only look at hi-order words. */
		sub.w	zEyePos, d7
		move.w	tunnelRect.left, d3
		move.w	tunnelRect.top, d4
		move.w	tunnelRect.right, d5
		move.w	tunnelRect.bottom, d6
		
		sub.w	xEyePos, d3
		muls	eyeToScreen, d3
		divs	d7, d3
				
		sub.w	yEyePos, d4
		muls	eyeToScreen, d4
		divs	d7, d4
		
		sub.w	xEyePos, d5
		muls	eyeToScreen, d5
		divs	d7, d5
		
		sub.w	yEyePos, d6
		muls	eyeToScreen, d6
		divs	d7, d6

		subq.w	#1, d5			/* Coords for line routines need to be */
		subq.w	#1, d6			/* adjusted so frame is drawn on INSIDE */
								/* of passed rectangle, as in QuickDraw */
								
		move.l	zCurBitMap, a0
		move.l	zMed, zCurPat

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
		
		/* Use ZVSLine with skipFactor = 1 for speed */
		/* ZVSLine(top, bottom, left, color, skipFactor) */

		move.w	#1, -(sp)
		move.w	#true, -(sp)
		move.w	d3, -(sp)
		move.w	d6, -(sp)
		move.w	d4, -(sp)
		bsr		ZVSLine
		lea		10(sp), sp
		
		/* ZVSLine(top, bottom, right, color, skipFactor) */
		
		move.w	#1, -(sp)
		move.w	#true, -(sp)
		move.w	d5, -(sp)
		move.w	d6, -(sp)
		move.w	d4, -(sp)
		bsr		ZVSLine
		lea		10(sp), sp
@noDraw:
@out:
		movem.l	(sp)+, d3-d7
	}
}
