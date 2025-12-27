/*
 * ZGraf - CDepth.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "CGraf.h"
#include "ZDepth.h"

#define _H_CDepth

/* Application globals */

extern ZBitMap *leftEyeBM, *rightEyeBM;

/* Function prototypes */

extern void CDHLine(Fixed left, Fixed right, Fixed y, Fixed z);
extern void CDVLine(Fixed top, Fixed bottom, Fixed x, Fixed z);
extern void CDVSLine(Fixed top, Fixed bottom, Fixed x, Fixed z,
	short color, short skipFactor);
extern void CDFrameRect(FRect *r, Fixed z);
extern void CDFillRect(FRect *r, Fixed z);
extern void	CDSetPixel(Fixed x, Fixed y, Fixed z, short color);
extern void CDTunnelRect(Fixed z);
