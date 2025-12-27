/*
 * ZGraf - ZExpl.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#pragma once

#define _H_ZExpl

#ifndef _H_ZDepth
	#include "ZDepth.h"
#endif

/* Datatypes */

typedef struct {
	Fixed xLine[6], yLine[6];
	Fixed xOffset[5], yOffset[5];
	Fixed distIncr;
	short xHitLine, yHitLine;
	short step;
	short lastStep;
	} Expl;

/* Function prototypes */

extern void InitExpl(Expl *e, FRect *extent, Fixed xHit, Fixed yHit,
	Fixed distIncr, short endStep);
extern void InitForm(Expl *e, FRect *extent, Fixed distIncr, short lastStep);
extern Boolean IncrExpl(Expl *e);

extern void EDHLine(Expl *e, Fixed left, Fixed right, Fixed y, Fixed z);
extern void EDVLine(Expl *e, Fixed top, Fixed bottom, Fixed x, Fixed z);
extern void EDVSLine(Expl *e, Fixed top, Fixed bottom, Fixed x, Fixed z,
	short color, short skipFactor);
extern void EDFrameRect(Expl *e, FRect *r, Fixed z);
extern void EDFillRect(Expl *e, FRect *r, Fixed z);
extern void EDSetPixel(Expl *e, Fixed x, Fixed y, Fixed z, short color);

