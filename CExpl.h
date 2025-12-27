/*
 * ZGraf - CExpl.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#define _H_CExpl

#ifndef _H_ZExpl
	#include "ZExpl.h"
#endif

extern void CEDHLine(Expl *e, Fixed left, Fixed right, Fixed y, Fixed z);
extern void CEDVLine(Expl *e, Fixed top, Fixed bottom, Fixed x, Fixed z);
extern void CEDVSLine(Expl *e, Fixed top, Fixed bottom, Fixed x, Fixed z,
	short color, short skipFactor);
extern void CEDFrameRect(Expl *e, FRect *r, Fixed z);
extern void CEDFillRect(Expl *e, FRect *r, Fixed z);
extern void CEDSetPixel(Expl *e, Fixed x, Fixed y, Fixed z, short color);

