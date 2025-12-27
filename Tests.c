/*
 * ZGraf - Tests.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include "Inheritance.h"

#include "Anim.h"
#include "Tests.h"
//#include <Stdio.h>

long HLineTest (
	Boolean qd,
	Boolean shade )
{
	short y;
	long start, end;
	Rect r;
	short i;
	
	zCurPat = &(*zShades)[0];
	ZSetRect(&r, 0, 0, zCurBitMap->xBound, zCurBitMap->yBound);
	ZFillRect(&r);
	start = Ticks;
	ZSetShade(10);
	if (qd) {
		for (i = 0; i < 20; i++) {		/* QuickDraw */
			for (y=0; y<zCurBitMap->yBound; y++) {
				if (shade)
					PenPat(qdShades[y%11]);
				MoveTo(0,y);
				Line(y,0); 
				}
			}
		}
	else {
		for (i = 0; i < 20; i++) { 		/* ZGraf */
			for (y=0; y<zCurBitMap->yBound; y++) {
				if (shade)
					zCurPat = &(*zShades)[y%11];
				ZHLine(0,y,y);
				}
			}
		}
	end = Ticks;
	return (end - start);
}

long VLineTest (
	Boolean qd,
	Boolean shade )
{
	short y;
	long start, end;
	Rect r;
	short i;
	
	ZSetShade(0);
	ZSetRect(&r, 0, 0, zCurBitMap->xBound, zCurBitMap->yBound);
	ZFillRect(&r);
	ZSetShade(10);
	start = Ticks;
	if (qd) {
		for (i = 0; i < 20; i++) {		/* QuickDraw */
			for (y=0; y<zCurBitMap->yBound; y++) {
				if (shade)
					PenPat((void *) &qdShades[y%11]);
				MoveTo(y,0);
				Line(0,y); 
				}
			}
		}
	else {
		for (i = 0; i < 20; i++) { 		/* ZGraf */
			for (y=0; y<zCurBitMap->yBound; y++) {
				if (shade) {
					zCurPat = &(*zShades)[y%11];
					ZVLine(0, y, y);
					}
				else
					ZVSLine(0,y,y,true,0);
				}
			}
		}
	end = Ticks;
	return (end - start);
}

long RectTest (
	Boolean qd,
	short size )
{
	Rect r;
	long i;
	long begin, end;
	short hVel, vVel;

	ZSetRect(&r, 0, 0, zCurBitMap->xBound, zCurBitMap->yBound);
	zCurPat = &(*zShades)[0];
	ZFillRect(&r);

	r.left = r.top = 0;
	r.right = r.bottom = size;
	hVel = 5;
	vVel = 3;
	ZSetShade(10);
	begin = Ticks;
	if (qd) {
		for (i = 0; i<5000; i++) {
			r.left += hVel; r.right += hVel;
			r.top += vVel; r. bottom += vVel;
			if ((r.left < 0) || (r.right > zCurBitMap->xBound)) hVel *= -1;
			if ((r.top < 0) || (r.bottom > zCurBitMap->yBound)) vVel *= -1;
			FillRect(&r, (ConstPatternParam) &qdShades[i%11]);
			}
		}
	else { /* ZGRAF */
		for (i = 0; i<5000; i++) {
			r.left += hVel; r.right += hVel;
			r.top += vVel; r. bottom += vVel;
			if ((r.left < 0) || (r.right > zCurBitMap->xBound)) hVel *= -1;
			if ((r.top < 0) || (r.bottom > zCurBitMap->yBound)) vVel *= -1;
			zCurPat = &(*zShades)[i%11];
			ZFillRect(&r);
			}
		}
	end = Ticks;
	return (end - begin);
}

long CopyTest (
	Boolean qd )
{
	ZBitMap *offScreen, *holder;
	Rect r;
	long t;
	short i;
	
	offScreen = ZNewBitMap(320, 320);
	holder = zCurBitMap;
	ZSetBitMap(offScreen);
	ZSetShade(0);
	ZSETRECT(r, 0, 0, offScreen->xBound, offScreen->yBound);
	ZFillRect(&r);
	ZSetShade(10);
	PenSize(2, 2);
	MoveTo(0, 0);
	LineTo(320, 320);
	MoveTo(320, 0);
	LineTo(0, 320);
	ZSetBitMap(holder);
	t = Ticks;
	if (!qd) {	/* ZGraf */
		for (i=0;i<5000;i++)
			ZCopyLongs(offScreen, zCurBitMap, &r, 0, 0);
		}
	else {
		for (i=0;i<5000;i++)
			CopyBits(&offScreen->qdPort.portBits,
				&zCurBitMap->qdPort.portBits,
				&r, &r, srcCopy, NIL);
		}
	ZFreeBitMap(offScreen);
	
	return(Ticks - t);
}


void VerticalBenchmark ()
{
	long vqdshade, vzshade, vqd, vz;

	vqdshade = VLineTest(true, true);
	vzshade = VLineTest(false, true);
	vqd = VLineTest(true, false);
	vz = VLineTest(false, false);

	printf("THE SURVEY SAYS...\n\n");
	printf("Shaded vertical lines:\n");
	printf("QD: %ld\t\tZGRAF: %ld\n", vqdshade, vzshade);
	printf("White vertical lines:\n");
	printf("QD: %ld\t\tZGRAF: %ld\n", vqd, vz);
	
}

void ScreenCopy ()
{
	Rect r;
	short x, y;
	long t;
	ZBitMap *offScreen;
	
	if (zCurBitMap->baseAddr != zPage1BitMap->baseAddr) {
		ZSetRect(&r, 0,0,zCurBitMap->xBound, zCurBitMap->yBound);
		ZCopyLongs(zCurBitMap, zPage1BitMap, &r, 0,0);
	}
}

void FullBenchmark (
	short onScreen )
{
	ZBitMap *offScreen;
	long hqdshade, hzshade, hqd, hz;
	long vqdshade, vzshade, vqd, vz;
	long r20qd, r20z;
	long r50qd, r50z;
	long r100qd, r100z;
	long r200qd, r200z;
	long r300qd, r300z;
	long copyqd, copyz;

	if (!onScreen) {
		offScreen = ZNewBitMap(zPage1BitMap->xBound,
			zPage1BitMap->yBound);
		ZSetBitMap(offScreen);
	}
	
	hqdshade = HLineTest(true, true);
	ScreenCopy();
	hzshade = HLineTest(false, true);
	ScreenCopy();
	hqd = HLineTest(true, false);
	ScreenCopy();
	hz = HLineTest(false, false);
	ScreenCopy();
	
	vqdshade = VLineTest(true, true);
	ScreenCopy();
	vzshade = VLineTest(false, true);
	ScreenCopy();
	vqd = VLineTest(true, false);
	ScreenCopy();
	vz = VLineTest(false, false);
	ScreenCopy();

	r20qd = RectTest(true, 20);
	ScreenCopy();
	r20z = RectTest(false, 20);
	ScreenCopy();
	
	r50qd = RectTest(true, 50);
	ScreenCopy();
	r50z = RectTest(false, 50);
	ScreenCopy();
	
	r100qd = RectTest(true, 100);
	ScreenCopy();
	r100z = RectTest(false, 100);
	ScreenCopy();
	
	r200qd = RectTest(true, 200);
	ScreenCopy();
	r200z = RectTest(false, 200);
	ScreenCopy();
	
	r300qd = RectTest(true, 300);
	ScreenCopy();
	r300z = RectTest(false, 300);
	ScreenCopy();
	
	copyqd = CopyTest(true);
	copyz = CopyTest(false);
	
	if (!onScreen)
		ZSetBitMap(zPage1BitMap);

	printf("Shaded horizontal lines:\n");
	printf("QD: %ld      ZGRAF: %ld\n", hqdshade, hzshade);
	printf("White horizontal lines:\n");
	printf("QD: %ld      ZGRAF: %ld\n", hqd, hz);
	printf("Shaded vertical lines:\n");
	printf("QD: %ld      ZGRAF: %ld\n", vqdshade, vzshade);
	printf("White vertical lines:\n");
	printf("QD: %ld      ZGRAF: %ld\n", vqd, vz);

	printf("\n20 x 20 shaded rectangles:\n");
	printf("QD: %ld      ZGRAF: %ld\n", r20qd, r20z);
	printf("50 x 50 shaded rectangles:\n");
	printf("QD: %ld      ZGRAF: %ld\n", r50qd, r50z);
	printf("100 x 100 shaded rectangles:\n");
	printf("QD: %ld      ZGRAF: %ld\n", r100qd, r100z);
	printf("200 x 200 shaded rectangles:\n");
	printf("QD: %ld      ZGRAF: %ld\n", r200qd, r200z);
	printf("300 x 300 shaded rectangles:\n");
	printf("QD: %ld      ZGRAF: %ld\n", r300qd, r300z);

	printf("\nCopyBits:\n");
	printf("QD: %ld      ZGRAF: %ld\n", copyqd, copyz);
}

void PixelTest ()
{
	register short x;
	register short y;
	
	for (y = 0; y<zCurBitMap->yBound; y++)
		for (x=0; x<zCurBitMap->xBound; x++)
			ZSetPixel(x, y, !(ZGetPixel(x,y)));
	ScreenCopy();

}


void Scroller (
	short dist )
{
	Rect r;
	long t;
	ZBitMap *screenCopy;
	short y, i;

	
	theDisplay = newInstance(Display);

	/* Initialize the display */

	(message(theDisplay, IDisplay));

	screenCopy = ZNewBitMap(zPage1BitMap->xBound,
		zPage1BitMap->yBound);
	ZSetRect(&r, 0, 0, zPage1BitMap->xBound, zPage1BitMap->yBound);
	ZCopyLongs(zPage1BitMap, screenCopy,&r, 0, 0);
	y = 0;
	r.left = 0; r.right = zPage1BitMap->xBound;
	for (i = 0; i < 2000; i ++) {
		syncTask.isVBL = false;		/* Reset VBL indicator */
		while(!syncTask.isVBL)
			;						/* Wait for vertical blank */
		r.top = zPage1BitMap->yBound - y;
		r.bottom = zPage1BitMap->yBound;
		ZCopyLongs(screenCopy, zPage1BitMap,&r, 0, 0);
		r.top = 0;
		r.bottom = zPage1BitMap->yBound - y;
		ZCopyLongs(screenCopy, zPage1BitMap, &r, 0, y);
		y -= dist;
		if (y > zPage1BitMap->yBound)
			y = 0;
		if (y < 0)
			y = zPage1BitMap->yBound;
		}
		ZSetRect(&r, 0, 0, zPage1BitMap->xBound, zPage1BitMap->yBound);
		ZCopyLongs(screenCopy, zPage1BitMap,&r, 0, 0);
		ZFreeBitMap(screenCopy);
		ZSetBitMap(zPage1BitMap);
	(message(theDisplay, CleanUp), (long) false);

}
