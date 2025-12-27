/*
 * ZGraf - ZDepth.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "ZGraf.h"

#define _H_ZDepth

/* Datatypes */

typedef struct {		/* Rect in a plane */
	Fixed top, left, bottom, right;
	} FRect;

typedef struct {		/* Analogous to QuickDraw Point */
	Fixed x, y, z;
	} Pos;

typedef struct {		/* Analogous to QuickDraw Rect */
	Fixed left, top, front, right, bottom, back;
	} Box;

/* Constants */

#define MINSHORT (short) 0x8000
#define MAXSHORT (short) 0x7FFF
#define HALFMINSHORT (short) 0xC000
#define HALFMAXSHORT (short) 0x3FFF

#define MINLONG (long) 0x80000000
#define MAXLONG (long) 0x7FFFFFFF
#define HALFMINLONG (long) 0xC0000000
#define HALFMAXLONG (long) 0x3FFFFFFF

#define MINFIXED MINLONG
#define MAXFIXED MAXLONG
#define HALFMINFIXED HALFMINLONG
#define HALFMAXFIXED HALFMAXLONG

#define f1 (long) 0x00010000
#define f2 (long) 0x00020000
#define f5 (long) 0x00050000
#define f10 (long) 0x000A0000
#define f20 (long) 0x00140000
#define f25 (long) 0x00190000
#define f30 (long) 0x001E0000
#define f40 (long) 0x00280000
#define f50 (long) 0x00320000
#define f100 (long) 0x00640000
#define f200 (long) 0x00C80000
#define f300 (long) 0x012C0000
#define f400 (long) 0x01900000
#define f500 (long) 0x01F40000
#define f600 (long) 0x02580000
#define f700 (long) 0x02BC0000
#define f800 (long) 0x03200000
#define f900 (long) 0x03840000
#define f1000 (long) 0x03E80000
#define f1100 (long) 0x044C0000
#define f1200 (long) 0x04B00000

/* Application globals */

extern short xEyePos, yEyePos, zEyePos;
extern short eyeToScreen, eyeOffset, origOffset;
extern short halfEyeOffset;
extern short zClosest, zHalfClosest;
extern short xSepClosest, xHalfSepClosest;
extern FRect tunnelRect;

/* Function prototypes */

//extern Fixed S2F(short s);

#define S2F(x) ((long) ((long) x << 16))
//extern Fixed S2F(short s);
extern short F2S(Fixed f);

extern void SetFRect(FRect *r, Fixed left, Fixed top,
	Fixed right, Fixed bottom);
extern void OffsetFRect(FRect *r, Fixed xOff, Fixed yOff);
extern void SetBox(Box *b, Fixed left, Fixed top, Fixed front,
	Fixed right, Fixed bottom, Fixed back);

extern void DInit(short eDist, short eOff, short oOff);
extern void DHLine(Fixed left, Fixed right, Fixed y, Fixed z);
extern void DVLine(Fixed top, Fixed bottom, Fixed x, Fixed z);
extern void DVSLine(Fixed top, Fixed bottom, Fixed x, Fixed z,
	short color, short skipFactor);
extern void DFrameRect(FRect *r, Fixed z);
extern void DFillRect(FRect *r, Fixed z);
extern void	DSetPixel(Fixed x, Fixed y, Fixed z, short color);
extern void DTunnelRect(Fixed z);

/* Macros */

#define F2X(f) Fix2X(f)
#define X2F(x) X2Fix(x)

#define SETPOS(pos, a, b, c) \
	{ pos.x = a; pos.y = b; pos.z = c }
	
#define SETBOX(box, x1, y1, z1, x2, y2, z2) \
	{ box.left = x1; box.top = y1; box.front = z1; \
	  box.right = x2; box.bottom = y2; box.back = z2; }
