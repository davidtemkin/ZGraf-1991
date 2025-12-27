/*
 * ZGraf - ZExpl.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include "ZExpl.h"

void InitExpl (
	Expl *e,
	FRect *extent,
	Fixed xHit, Fixed yHit,
	Fixed distIncr,
	short lastStep )
{
	Fixed xQuarter, yQuarter;
	Fixed look;
	Fixed xOffset, yOffset;
	short i;
	
	xQuarter = (extent->right - extent->left) >> 2;
	yQuarter = (extent->bottom - extent->top) >> 2;


	if (xHit >= extent->right) {
		e->xHitLine = 4;
		xOffset = xQuarter;
		}		
	else if (xHit <= extent->left) {
		xOffset = xQuarter;
		e->xHitLine = 0;
		}
	else {
		i = 0;
		look = extent->left;
		while (xHit > (look + xQuarter) && i <= 4) {
			look += xQuarter;
			i++;
			}
		e->xHitLine = i + 1;
		xOffset = xHit - look;
		}

	if (yHit >= extent->bottom) {
		e->yHitLine = 4;
		yOffset = yQuarter;
		}		
	else if (yHit <= extent->top) {
		yOffset = yQuarter;
		e->yHitLine = 0;
		}
	else {
		i = 0;
		look = extent->top;
		while (yHit > (look + yQuarter) && i <= 4) {
			look += yQuarter;
			i++;
			}
		e->yHitLine = i + 1;
		yOffset = yHit - look;
		}


	e->xLine[0] = extent->left;
	e->yLine[0] = extent->top;
	e->xLine[1] = extent->left + xOffset;
	e->yLine[1] = extent->top + yOffset;
	for (i = 2; i <= 5; i++) {
		e->xLine[i] = e->xLine[i-1] + xQuarter;
		e->yLine[i] = e->yLine[i-1] + yQuarter;
		}

	e->distIncr = distIncr;
	e->lastStep = lastStep;
	e->step = 0;
	for (i=0; i <= 4; i++)
		e->xOffset[i] = e->yOffset[i] = 0L;
}

void InitForm (
	Expl *e,
	FRect *extent,
	Fixed distIncr,
	short lastStep )
{
	Fixed xQuarter, yQuarter;
	short i;
	
	xQuarter = (extent->right - extent->left) >> 2;
	yQuarter = (extent->bottom - extent->top) >> 2;

	e->xHitLine = 3;
	e->yHitLine = 3;


	e->xLine[0] = extent->left;
	e->yLine[0] = extent->top;
	e->xLine[1] = extent->left;
	e->yLine[1] = extent->top;
	
	for (i = 2; i <= 5; i++) {
		e->xLine[i] = e->xLine[i-1] + xQuarter;
		e->yLine[i] = e->yLine[i-1] + yQuarter;
		}	
	
	e->distIncr = -distIncr;
	e->lastStep = lastStep;
	e->step = 0;


	e->xOffset[3] = FixMul(S2F(lastStep), distIncr);
	e->yOffset[3] = e->xOffset[3];
	e->xOffset[2] = e->yOffset[2] = -e->xOffset[3];
	
	
	e->xOffset[4] = FixMul(S2F(2), e->xOffset[3]);
	e->yOffset[4] = e->xOffset[4];
	e->xOffset[1] = e->yOffset[1] = -e->xOffset[4];
	
	e->xOffset[0] = e->yOffset[0] = e->xOffset[1];
}

Boolean IncrExpl (
	Expl *ex )
{
	register short i;
	register Expl *e = ex;
	register Fixed incr;
	
	incr = e->distIncr;
	for (i = e->xHitLine; i <= 4; i++) {
		e->xOffset[i] += incr;
		incr += incr;
		}
		
	incr = e->distIncr;
	for (i = e->xHitLine-1; i >= 0; i--) {
		e->xOffset[i] -= incr;
		incr += incr;
		}
		
	incr = e->distIncr;
	for (i = e->yHitLine; i <= 4; i++) {
		e->yOffset[i] += incr;
		incr += incr;
		}
		
	incr = e->distIncr;
	for (i = e->yHitLine-1; i >= 0; i--) {
		e->yOffset[i] -= incr;
		incr += incr;
		}
		
	e->step++;
	if (e->step >= e->lastStep)
		return(true);
	else
		return(false);
}

void EDHLine (
	Expl *e,
	Fixed left, Fixed right,
	Fixed y,
	Fixed z )
{
	Fixed leftDraw, rightDraw, yDraw;
	short xl, leftL;
	short yL;
	Boolean doneHoriz;

	leftL = 0;
	yL = 0;
	while (left > e->xLine[leftL + 1] && leftL <= 4)
		leftL++;
	while (y > e->yLine[yL + 1] && yL <= 4)
		yL++;
	
	/*
		Go from left to right & draw each segment
	*/
	
	yDraw = y + e->yOffset[yL];
	xl = leftL;
	leftDraw = left + e->xOffset[leftL];
	doneHoriz = false;
	while (!doneHoriz && xl <= 4) {
		if (right <= e->xLine[xl + 1]) {
			rightDraw = right + e->xOffset[xl];
			DHLine(leftDraw, rightDraw, yDraw, z);
			doneHoriz = true;
			}
		else {
			rightDraw = e->xLine[xl + 1] + e->xOffset[xl];
			DHLine(leftDraw, rightDraw, yDraw, z);
			xl++;
			leftDraw = e->xLine[xl] + e->xOffset[xl];
			}
		}
}

void EDVLine (
	Expl *e,
	Fixed top, Fixed bottom,
	Fixed x,
	Fixed z )
{
	Fixed topDraw, bottomDraw, xDraw;
	short yl, xL;
	Boolean doneVert;

	xL = 0;
	yl = 0;
	while (x > e->xLine[xL + 1] && xL <= 4)
		xL++;
	while (top > e->yLine[yl + 1] && yl <= 4)
		yl++;
	
	/*
		Go from top to bottom & draw each segment
	*/
	
	xDraw = x + e->xOffset[xL];
	topDraw = top + e->yOffset[yl];
	doneVert = false;
	while (!doneVert && yl <= 4) {
		if (bottom <= e->yLine[yl + 1]) {
			bottomDraw = bottom + e->yOffset[yl];
			DVLine(topDraw, bottomDraw, xDraw, z);
			doneVert = true;
			}
		else {
			bottomDraw = e->yLine[yl + 1] + e->yOffset[yl];
			DVLine(topDraw, bottomDraw, xDraw, z);
			yl++;
			topDraw = e->yLine[yl] + e->yOffset[yl];
			}
		}
}

void EDVSLine (
	Expl *e,
	Fixed top, Fixed bottom,
	Fixed x,
	Fixed z,
	short color,
	short skipFactor )
{
	Fixed topDraw, bottomDraw, xDraw;
	short yl, xL;
	Boolean doneVert;

	xL = 0;
	yl = 0;
	while (x > e->xLine[xL + 1] && xL <= 4)
		xL++;
	while (top > e->yLine[yl + 1] && yl <= 4)
		yl++;

	/*
		Go from top to bottom & draw each segment
	*/

	xDraw = x + e->xOffset[xL];
	topDraw = top + e->yOffset[yl];
	doneVert = false;
	while (!doneVert && yl <= 4) {
		if (bottom <= e->yLine[yl + 1]) {
			bottomDraw = bottom + e->yOffset[yl];
			DVSLine(topDraw, bottomDraw, xDraw, z, color, skipFactor);
			doneVert = true;
			}
		else {
			bottomDraw = e->yLine[yl + 1] + e->yOffset[yl];
			DVSLine(topDraw, bottomDraw, xDraw, z, color, skipFactor);
			yl++;
			topDraw = e->yLine[yl] + e->yOffset[yl];
			}
		}
}

void EDFrameRect (
	Expl *e,
	FRect *r,
	Fixed z )
{
	Boolean useVSLine;
	short color;

	if (zCurPat == zBlack) {
		useVSLine = true;
		color = false;
		}
	else if (zCurPat == zWhite) {
		useVSLine = true;
		color = true;
		}
	else
		useVSLine = false;
		
	EDHLine(e, r->left, r->right, r->left, z);
	EDHLine(e, r->left, r->right, r->right, z);

	if (useVSLine) {
		EDVSLine(e, r->top, r->bottom, r->left, z, color, 0);
		EDVSLine(e, r->top, r->bottom, r->right, z, color, 0);
		}
	else {
		EDVLine(e, r->top, r->bottom, r->left, z);
		EDVLine(e, r->top, r->bottom, r->right, z);
		}
}

void EDFillRect (
	Expl *e,
	FRect *r,
	Fixed z )
{
	FRect drawRect;
	short xl, yl;
	short leftL, topL;
	Boolean doneHoriz, doneVert;

	leftL = 0;
	topL = 0;
	while (r->left > e->xLine[leftL + 1] && leftL <= 4)
		leftL++;
	while (r->top > e->yLine[topL + 1] && topL <= 4)
		topL++;
	
	/*
		Go from top to bottom and draw each row
		consecutively from left to right
	*/
	
	yl = topL;
	drawRect.top = r->top + e->yOffset[topL];
	doneVert = false;
	while (!doneVert && yl <= 4) {
		if (r->bottom <= e->yLine[yl + 1]) {
			drawRect.bottom = r->bottom + e->yOffset[yl];
			doneVert = true;
			}
		else
			drawRect.bottom = e->yLine[yl + 1] + e->yOffset[yl];
		xl = leftL;
		drawRect.left = r->left + e->xOffset[leftL];
		doneHoriz = false;
		while (!doneHoriz && xl <= 4) {
			if (r->right <= e->xLine[xl + 1]) {
				drawRect.right = r->right + e->xOffset[xl];
				DFillRect(&drawRect, z);
				doneHoriz = true;
				}
			else {
				drawRect.right = e->xLine[xl + 1] + e->xOffset[xl];
				DFillRect(&drawRect, z);
				xl++;
				drawRect.left = e->xLine[xl] + e->xOffset[xl];
				}
			}
		yl++;
		drawRect.top = e->yLine[yl] + e->yOffset[yl];
		}
}

void EDSetPixel (
	Expl *e,
	Fixed x, Fixed y,
	Fixed z,
	short color )
{
	short xl, yl;

	xl = 0;
	yl = 0;
	while (x > e->xLine[xl + 1] && xl <= 4)
		xl++;
	while (y > e->yLine[yl + 1] && yl <= 4)
		yl++;
	DSetPixel(x + e->xOffset[xl], y + e->yOffset[yl], z, color);

}