/*
 * ZGraf - CDepth.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"
#include "JumpTable.h"

#include "ZGraf.h"
#include "ZDepth.h"
#include "CGraf.h"

ZBitMap *leftEyeBM, *rightEyeBM;

void CDHLine (
	Fixed left, Fixed right,
	Fixed y,
	Fixed z )
{
	asm 68000 {
		movem.l	d3-d5, -(sp)
		move.w	left, d0		/* Extract only high order word of */
		move.w	right, d1		/* coordinates */
		move.w	y, d3
		move.w	z, d4
		move.w	xEyePos, d5
		
		sub.w	halfEyeOffset, d5
		sub.w	zEyePos, d4
		
		sub.w	yEyePos, d3
		muls	eyeToScreen, d3
		divs	d4, d3					/* d3.w = yDraw */

		sub.w	d5, d0
		muls	eyeToScreen, d0
		divs	d4, d0					/* d0.w = leftDraw */
		
		sub.w	d5, d1
		muls	eyeToScreen, d1
		divs	d4, d1					/* d1.w = rightDraw */
		
		/* Draw the image for the left eye */
		
		move.l	leftEyeBM, zCurBitMap
		
		move.w	d3, -(sp)
		move.w	d1, -(sp)
		move.w	d0, -(sp)
		bsr		CHLine
		addq.l	#6, sp

		move.w	left, d0
		move.w	right, d1
		add.w	eyeOffset, d5

		sub.w	d5, d0
		muls	eyeToScreen, d0
		divs	d4, d0					/* d0.w = leftDraw */
		
		sub.w	d5, d1
		muls	eyeToScreen, d1
		divs	d4, d1					/* d1.w = rightDraw */

		/* Draw the image for the right eye */
		
		move.l	rightEyeBM, zCurBitMap
		
		move.w	d3, -(sp)
		move.w	d1, -(sp)
		move.w	d0, -(sp)
		bsr		CHLine
		addq.l	#6, sp

		movem.l	(sp)+, d3-d5
		}
}

void CDVLine (
	Fixed top, Fixed bottom,
	Fixed x,
	Fixed z )
{
	asm 68000 {
		movem.l	d3-d6, -(sp)
		move.w	x, d0
		move.w	top, d3
		move.w	bottom, d4
		move.w	z, d5
		move.w	xEyePos, d6
		
		sub.w	halfEyeOffset, d6
		sub.w	zEyePos, d5

		sub.w	yEyePos, d3
		muls	eyeToScreen, d3
		divs	d5, d3					/* d3.w = topDraw */
		
		sub.w	yEyePos, d4
		muls	eyeToScreen, d4
		divs	d5, d4					/* d4.w = bottomDraw */
		
		sub.w	d6, d0
		muls	eyeToScreen, d0
		divs	d5, d0					/* d0.w = xDraw */
		
		/* Draw the image for the left eye */
		
		move.l	leftEyeBM, zCurBitMap
		
		move.w	d0, -(sp)
		move.w	d4, -(sp)
		move.w	d3, -(sp)
		bsr		CVLine
		addq.l	#6, sp
		
		move.w	x, d0
		add.w	eyeOffset, d6
		
		sub.w	d6, d0
		muls	eyeToScreen, d0
		divs	d5, d0					/* d0.w = xDraw */
		
		/* Draw the image for the right eye */
		
		move.l	rightEyeBM, zCurBitMap
		
		move.w	d0, -(sp)
		move.w	d4, -(sp)
		move.w	d3, -(sp)
		bsr		CVLine
		addq.l	#6, sp
		
		movem.l	(sp)+, d3-d6
		}
}

void CDVSLine (
	Fixed top, Fixed bottom,
	Fixed x,
	Fixed z,
	short color,
	short skipFactor )
{
	asm 68000 {
		movem.l	d3-d6, -(sp)
		move.w	x, d0
		move.w	top, d3
		move.w	bottom, d4
		move.w	z, d5
		move.w	xEyePos, d6
		
		sub.w	halfEyeOffset, d6
		sub.w	zEyePos, d5

		sub.w	yEyePos, d3
		muls	eyeToScreen, d3
		divs	d5, d3					/* d3.w = topDraw */
		
		sub.w	yEyePos, d4
		muls	eyeToScreen, d4
		divs	d5, d4					/* d4.w = bottomDraw */
		
		sub.w	d6, d0
		muls	eyeToScreen, d0
		divs	d5, d0					/* d0.w = xDraw */

		/* Draw the image for the left eye */

		move.l	leftEyeBM, zCurBitMap
		
		move.w	skipFactor, -(sp)
		move.w	color, -(sp)
		move.w	d0, -(sp)
		move.w	d4, -(sp)
		move.w	d3, -(sp)
		bsr		CVSLine
		lea		10(sp), sp				/* clean up stack */

		/* Draw the image for the right eye */
		
		move.w	x, d0
		add.w	eyeOffset, d6

		sub.w	d6, d0
		muls	eyeToScreen, d0
		divs	d5, d0					/* d0.w = xDraw */
		
		move.l	rightEyeBM, zCurBitMap
		
		move.w	skipFactor, -(sp)
		move.w	color, -(sp)
		move.w	d0, -(sp)
		move.w	d4, -(sp)
		move.w	d3, -(sp)
		bsr		CVSLine
		lea		10(sp), sp				/* clean up stack */
		
		movem.l	(sp)+, d3-d6
		}
}

void CDFrameRect (
	FRect *r,
	Fixed z )
{
	Rect drawRect;
	
	asm 68000 {
		movem.l	d3-d6/a2, -(sp)
		move.l	r, a2
		move.w	OFFSET(FRect, left) (a2), d0
		move.w	OFFSET(FRect, right) (a2), d1
		move.w	OFFSET(FRect, top) (a2), d3
		move.w	OFFSET(FRect, bottom) (a2), d4
		move.w	z, d5
		move.w	xEyePos, d6
		
		sub.w	halfEyeOffset, d6
		sub.w	zEyePos, d5
		
		sub.w	d6, d0
		muls	eyeToScreen, d0
		divs	d5, d0
		move.w	d0, drawRect.left
		
		sub.w	d6, d1
		muls	eyeToScreen, d1
		divs	d5, d1
		move.w	d1, drawRect.right
		
		sub.w	yEyePos, d3
		muls	eyeToScreen, d3
		divs	d5, d3
		move.w	d3, drawRect.top
		
		sub.w	yEyePos, d4
		muls	eyeToScreen, d4
		divs	d5, d4
		move.w	d4, drawRect.bottom
		
		/* Draw image for left eye */
		
		move.l	leftEyeBM, zCurBitMap
		
		pea		drawRect
		bsr		CFrameRect
		clr.l	(sp)+
		
		/* Draw image for right eye */

		move.w	OFFSET(FRect, left) (a2), d0
		move.w	OFFSET(FRect, right) (a2), d1
		add.w	eyeOffset, d6

		sub.w	d6, d0
		muls	eyeToScreen, d0
		divs	d5, d0
		move.w	d0, drawRect.left

		sub.w	d6, d1
		muls	eyeToScreen, d1
		divs	d5, d1
		move.w	d1, drawRect.right

		move.l	rightEyeBM, zCurBitMap

		pea		drawRect
		bsr		CFrameRect
		clr.l	(sp)+
		
		movem.l	(sp)+, d3-d6/a2
		}
}

void CDFillRect (
	FRect *r,
	Fixed z )
{
	Rect drawRect;
	
	asm 68000 {
		movem.l	d3-d6/a2, -(sp)
		move.l	r, a2
		move.w	OFFSET(FRect, left) (a2), d0
		move.w	OFFSET(FRect, top) (a2), d3
		move.w	OFFSET(FRect, right) (a2), d1
		move.w	OFFSET(FRect, bottom) (a2), d4
		move.w	z, d5
		move.w	xEyePos, d6
		
		sub.w	halfEyeOffset, d6
		sub.w	zEyePos, d5
		
		sub.w	d6, d0
		muls	eyeToScreen, d0
		divs	d5, d0
		move.w	d0, drawRect.left
		
		sub.w	yEyePos, d3
		muls	eyeToScreen, d3
		divs	d5, d3
		move.w	d3, drawRect.top
		
		sub.w	d6, d1
		muls	eyeToScreen, d1
		divs	d5, d1
		move.w	d1, drawRect.right
		
		sub.w	yEyePos, d4
		muls	eyeToScreen, d4
		divs	d5, d4
		move.w	d4, drawRect.bottom
		
		/* Draw image for left eye */
		
		move.l	leftEyeBM, zCurBitMap
		
		pea		drawRect
		bsr		CFillRect
		clr.l	(sp)+
		
		/* Draw image for right eye */

		move.w	OFFSET(FRect, left) (a2), d0
		move.w	OFFSET(FRect, right) (a2), d1
		add.w	eyeOffset, d6

		sub.w	d6, d0
		muls	eyeToScreen, d0
		divs	d5, d0
		move.w	d0, drawRect.left

		sub.w	d6, d1
		muls	eyeToScreen, d1
		divs	d5, d1
		move.w	d1, drawRect.right

		move.l	rightEyeBM, zCurBitMap

		pea		drawRect
		bsr		CFillRect
		clr.l	(sp)+
		
		movem.l	(sp)+, d3-d6/a2
		}
}

void CDSetPixel (
	Fixed x, Fixed y,
	Fixed z,
	short color )
{
	asm 68000 {
		movem.l	d3-d5, -(sp)
		move.w	x, d0
		move.w	y, d3
		move.w	z, d4
		move.w	xEyePos, d5
				
		sub.w	halfEyeOffset, d5
		sub.w	zEyePos, d4

		sub.w	d5, d0
		muls	eyeToScreen, d0
		divs	d4, d0		
		
		sub.w	yEyePos, d3
		muls	eyeToScreen, d3
		divs	d4, d3

		/* Draw image for the left eye */

		move.l	leftEyeBM, zCurBitMap

		move.w	color, -(sp)
		move.w	d3, -(sp)
		move.w	d0, -(sp)
		bsr		CSetPixel
		addq.l	#6, sp
		
		move.w	x, d0
		add.w	eyeOffset, d5

		sub.w	d5, d0
		muls	eyeToScreen, d0
		divs	d4, d0		

		/* Draw image for the right eye */
		
		move.l	rightEyeBM, zCurBitMap

		move.w	color, -(sp)
		move.w	d3, -(sp)
		move.w	d0, -(sp)
		bsr		CSetPixel
		addq.l	#6, sp

		movem.l	(sp)+, d3-d5
		}
}

void CDTunnelRect (
	Fixed z )
{
	asm 68000 {
		movem.l	d3-d7, -(sp)
		move.w	tunnelRect.left, d3
		move.w	tunnelRect.top, d4
		move.w	tunnelRect.right, d5
		move.w	tunnelRect.bottom, d6
		move.w	z, d7
		move.w	xEyePos, d0

		sub.w	halfEyeOffset, d0
		sub.w	zEyePos, d7
		
		sub.w	d0, d3
		muls	eyeToScreen, d3
		divs	d7, d3
				
		sub.w	yEyePos, d4
		muls	eyeToScreen, d4
		divs	d7, d4
		
		sub.w	d0, d5
		muls	eyeToScreen, d5
		divs	d7, d5
		
		sub.w	yEyePos, d6
		muls	eyeToScreen, d6
		divs	d7, d6

		subq.w	#1, d5			/* Coords for line routines need to be */
		subq.w	#1, d6			/* adjusted so frame is drawn on INSIDE */
								/* of passed rectangle, as in QuickDraw */

		move.l	zMed, zCurPat
		
		/* Draw image for left eye */
		
		move.l	leftEyeBM, zCurBitMap

		move.w	d4, -(sp)		/* CHLine(left, right, top) */
		move.w	d5, -(sp)
		move.w	d3, -(sp)
		bsr		CHLine
		addq.l	#6, sp
		
		move.w	d6, -(sp)		/* CHLine(left, right, bottom) */
		move.w	d5, -(sp)
		move.w	d3, -(sp)
		bsr		CHLine
		addq.l	#6, sp
		
		move.w	#1, -(sp)		/* CVSLine(top, bottom, left, color, skipFactor) */
		move.w	#true, -(sp)
		move.w	d3, -(sp)
		move.w	d6, -(sp)
		move.w	d4, -(sp)
		bsr		CVSLine
		lea		10(sp), sp
		
		move.w	#1, -(sp)		/* CVSLine(top, bottom, right, color, skipFactor) */
		move.w	#true, -(sp)
		move.w	d5, -(sp)
		move.w	d6, -(sp)
		move.w	d4, -(sp)
		bsr		CVSLine
		lea		10(sp), sp

		/* Draw image for right eye */
		
		move.l	rightEyeBM, zCurBitMap
		move.w	tunnelRect.left, d3
		move.w	tunnelRect.right, d5
		move.w	xEyePos, d0

		add.w	eyeOffset, d0
		
		sub.w	d0, d3
		muls	eyeToScreen, d3
		divs	d7, d3

		sub.w	d0, d5
		muls	eyeToScreen, d5
		divs	d7, d5

		move.w	d4, -(sp)		/* CHLine(left, right, top) */
		move.w	d5, -(sp)
		move.w	d3, -(sp)
		bsr		CHLine
		addq.l	#6, sp
		
		move.w	d6, -(sp)		/* CHLine(left, right, bottom) */
		move.w	d5, -(sp)
		move.w	d3, -(sp)
		bsr		CHLine
		addq.l	#6, sp
		
		move.w	#1, -(sp)		/* CVSLine(top, bottom, left, color, skipFactor) */
		move.w	#true, -(sp)
		move.w	d3, -(sp)
		move.w	d6, -(sp)
		move.w	d4, -(sp)
		bsr		CVSLine
		lea		10(sp), sp
		
		move.w	#1, -(sp)		/* CVSLine(top, bottom, right, color, skipFactor) */
		move.w	#true, -(sp)
		move.w	d5, -(sp)
		move.w	d6, -(sp)
		move.w	d4, -(sp)
		bsr		CVSLine
		lea		10(sp), sp

		movem.l	(sp)+, d3-d7
	}
}

