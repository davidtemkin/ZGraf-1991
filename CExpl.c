/*
 * ZGraf - CExpl.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include "CExpl.h"

void CEDHLine (
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
			CDHLine(leftDraw, rightDraw, yDraw, z);
			doneHoriz = true;
			}
		else {
			rightDraw = e->xLine[xl + 1] + e->xOffset[xl];
			CDHLine(leftDraw, rightDraw, yDraw, z);
			xl++;
			leftDraw = e->xLine[xl] + e->xOffset[xl];
			}
		}
}

void CEDVLine (
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
			CDVLine(topDraw, bottomDraw, xDraw, z);
			doneVert = true;
			}
		else {
			bottomDraw = e->yLine[yl + 1] + e->yOffset[yl];
			CDVLine(topDraw, bottomDraw, xDraw, z);
			yl++;
			topDraw = e->yLine[yl] + e->yOffset[yl];
			}
		}
}

void CEDVSLine (
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
			CDVSLine(topDraw, bottomDraw, xDraw, z, color, skipFactor);
			doneVert = true;
			}
		else {
			bottomDraw = e->yLine[yl + 1] + e->yOffset[yl];
			CDVSLine(topDraw, bottomDraw, xDraw, z, color, skipFactor);
			yl++;
			topDraw = e->yLine[yl] + e->yOffset[yl];
			}
		}
}

void CEDFrameRect (
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
		
	CEDHLine(e, r->left, r->right, r->left, z);
	CEDHLine(e, r->left, r->right, r->right, z);

	if (useVSLine) {
		CEDVSLine(e, r->top, r->bottom, r->left, z, color, 0);
		CEDVSLine(e, r->top, r->bottom, r->right, z, color, 0);
		}
	else {
		CEDVLine(e, r->top, r->bottom, r->left, z);
		CEDVLine(e, r->top, r->bottom, r->right, z);
		}
}

void CEDFillRect (
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
				CDFillRect(&drawRect, z);
				doneHoriz = true;
				}
			else {
				drawRect.right = e->xLine[xl + 1] + e->xOffset[xl];
				CDFillRect(&drawRect, z);
				xl++;
				drawRect.left = e->xLine[xl] + e->xOffset[xl];
				}
			}
		yl++;
		drawRect.top = e->yLine[yl] + e->yOffset[yl];
		}
}

void CEDSetPixel (
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
	CDSetPixel(x + e->xOffset[xl], y + e->yOffset[yl], z, color);

}