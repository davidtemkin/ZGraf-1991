/*
 * ZGraf - ZExpandPix.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include "ZExpandPix.h"

long ((*zFourBitTable)[]);			/* Pointers to arrays of longs */
long ((*zEightBitTable)[]);

void ZInitFourBitTable ()
{
	zFourBitTable = nil;
	
	FailNIL(zFourBitTable = (long ((*)[])) ZNewPtr(262144L));

	asm 68020 {
		move.l	zFourBitTable, a0
		clr.l	d0					/* d0: source word */
		clr.l	d1					/* d1: dest long */
									/* d3: scratch */
	@back1:							
		bfextu	d0{16:2}, d3
		bfins	d3, d1{2:2}
		bfextu	d0{18:2}, d3
		bfins	d3, d1{6:2}
		bfextu	d0{20:2}, d3
		bfins	d3, d1{10:2}
		bfextu	d0{22:2}, d3
		bfins	d3, d1{14:2}
		bfextu	d0{24:2}, d3
		bfins	d3, d1{18:2}
		bfextu	d0{26:2}, d3
		bfins	d3, d1{22:2}
		bfextu	d0{28:2}, d3
		bfins	d3, d1{26:2}
		bfextu	d0{30:2}, d3
		bfins	d3, d1{30:2}	
		move.l	d1, (a0)+
		add.w	#1, d0
		tst.w	d0
		bne		@back1
		}
}

void ZInitEightBitTable ()
{
	zEightBitTable = nil;

	FailNIL(zEightBitTable = (long ((*)[])) ZNewPtr(1024L));

	asm 68020 {
		move.l	zEightBitTable, a0
		clr.l	d0					/* d0: source word */
		clr.l	d1					/* d1: dest long */
									/* d3: scratch */
	@back1:							
		bfextu	d0{24:2}, d3
		bfins	d3, d1{6:2}
		bfextu	d0{26:2}, d3
		bfins	d3, d1{14:2}
		bfextu	d0{28:2}, d3
		bfins	d3, d1{22:2}
		bfextu	d0{30:2}, d3
		bfins	d3, d1{30:2}
		move.l	d1, (a0)+
		add.b	#1, d0
		tst.b	d0
		bne		@back1
		}
}

void ZExpand2To4 (
	ZBitMap *srcBits, ZBitMap *dstBits,
	Rect *srcRectAbs,
	short xDestAbs, short yDestAbs )
{
	short srcLineBytes, dstLineBytes;

	/*
		Warning: This routine, like ZCopyLongs, performs no error 
		checking and no clipping. All coordinates passed to this
		routine, including srcRectAbs, xDestAbs, and yDestAbs,
		need to be specified in absolute (global coordinates).

		This routine only executes on the 68020 and above.

	*/

	asm 68020 {
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
		/* in source bitmap */
		
		subq.w	#1, d2
		subq.w	#1, d3
		
		/* division by 16 via shifting & masking */
		/* to obtain begin/end addr in source bitmap */

		and.w	#0xFFF0, d0
		lsr.w	#2, d0			/* d0.w = number of bytes to add to */
								/* row address for beginLong (src) */
		and.w	#0xFFF0, d2
		lsr.w	#2, d2			/* d2.w = number of bytes to add to */
								/* row address for endLong (src) */

		/* Moving twice as many bytes into dst as from src */

		move.w	d2, srcLineBytes
		sub.w	d0, srcLineBytes
		add.w	#4, srcLineBytes	/* Move one more long than difference */
		
		move.w	srcLineBytes, d7
		lsl.w	#1, d7				/* Lines are twice as long in dest. */
		move.w	d7, dstLineBytes

		move.w	xDestAbs, d5		/* d5.w = number of bytes to add to */
		and.w	#0xFFF8, d5			/* row address for beginLong (dest) */
		lsr.w	#1, d5

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

		move.w	srcLineBytes, d6	
		move.w	d6, d7
		lsl.w	#1, d6				/* 12 bytes executed per 4 bytes moved */
		add.w	d7, d6				/* from src bitmap -- so multiply */
		lea		@finish, a3			/* bytes moved by 3 to get bytes to */
		suba.w	d6, a3				/* execute per line */
		
		/* calculate how much should be added to both source and */
		/* destination addresses after each line is moved */
		
		move.w	OFFSET(ZBitMap, rowBytes) (a0), d6
		sub.w	srcLineBytes, d6					/* add d6 to src address */

		move.w	OFFSET(ZBitMap, rowBytes) (a1), d7
		sub.w	dstLineBytes, d7					/* add d7 to dst addr */

		clr.l	d1					/* d1 used as scratch register */
		move.l	zFourBitTable, a0	/* a0 points to base of translation tbl */

@back3:
		jmp		(a3)
		
		move.w	(a2)+, d1				/* 100 longs moved from src */
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		
		move.w	(a2)+, d1				/* 90 longs moved from src */
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		
		move.w	(a2)+, d1				/* 80 longs moved from src */
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		
		move.w	(a2)+, d1				/* 70 longs moved from src */
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		
		move.w	(a2)+, d1				/* 60 longs moved from src */
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		
		move.w	(a2)+, d1				/* 50 longs moved from src */
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		
		move.w	(a2)+, d1				/* 40 longs moved from src */
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		
		move.w	(a2)+, d1				/* 30 longs moved from src */
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+		
		
		move.w	(a2)+, d1				/* 20 longs moved from src */
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+

		move.w	(a2)+, d1				/* 10 longs moved from src */
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
		move.w	(a2)+, d1
		move.l	(a0, d1.l*4), (a4)+
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

void ZExpand2To8 (
	ZBitMap *srcBits, ZBitMap *dstBits,
	Rect *srcRectAbs,
	short xDestAbs, short yDestAbs )
{
	short srcLineBytes, dstLineBytes;

	/*
		Warning: This routine, like ZCopyLongs, performs no error 
		checking and no clipping. All coordinates passed to this
		routine, including srcRectAbs, xDestAbs, and yDestAbs,
		need to be specified in absolute (global coordinates).

		This routine only executes on the 68020 and above.

	*/

	asm 68020 {
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
		/* in source bitmap */
		
		subq.w	#1, d2
		subq.w	#1, d3
		
		/* division by 16 via shifting & masking */
		/* to obtain begin/end addr in source bitmap */

		and.w	#0xFFF0, d0
		lsr.w	#2, d0			/* d0.w = number of bytes to add to */
								/* row address for beginLong (src) */
		and.w	#0xFFF0, d2
		lsr.w	#2, d2			/* d2.w = number of bytes to add to */
								/* row address for endLong (src) */

		/* Moving four times as many bytes into dst as from src */

		move.w	d2, srcLineBytes
		sub.w	d0, srcLineBytes
		add.w	#4, srcLineBytes	/* Move one more long than difference */
		
		move.w	srcLineBytes, d7
		lsl.w	#2, d7				/* Lines are four times as long in dst */
		move.w	d7, dstLineBytes

		move.w	xDestAbs, d5		/* d5.w = number of bytes to add to */
		and.w	#0xFFFC, d5			/* row address for beginLong (dest) */

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

		/* calculate how much should be added to both source and */
		/* destination addresses after each line is moved */
		
		move.w	OFFSET(ZBitMap, rowBytes) (a0), d6
		sub.w	srcLineBytes, d6					/* add d6 to src address */

		move.w	OFFSET(ZBitMap, rowBytes) (a1), d7
		sub.w	dstLineBytes, d7					/* add d7 to dst addr */

		clr.l	d1					/* d1 used as scratch register */
		move.l	zEightBitTable, a0	/* a0 points to base of translation */
									/*   table */

		move.w	srcLineBytes, d2
		lsr.w	#2, d2				/* d2 holds number of LONGS to be moved */
		sub.w	#1, d2				/* from src per line, less one */
		move.w	d2, d3				/* d3 holds a copy of this number */

@back:								/* This moves one long from source */
		move.b	(a2)+, d1
		move.l	(a0, d1.w*4), (a4)+
		move.b	(a2)+, d1
		move.l	(a0, d1.w*4), (a4)+
		move.b	(a2)+, d1
		move.l	(a0, d1.w*4), (a4)+
		move.b	(a2)+, d1
		move.l	(a0, d1.w*4), (a4)+
		dbra.w	d2, @back

@lineDone:
		adda.w	d6, a2
		cmpa.l	d4, a2
		bgt		@out
		adda.w	d7, a4
		move.w	d3, d2
		bra		@back

@out:
		movem.l	(sp)+, d3-d7/a2-a4
	}
}
