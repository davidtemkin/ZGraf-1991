/*
 * ZGraf - Misc.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#define _H_Misc

#define true			1
#define false			0
#define abs(a)			((a > 0) ? (a) : -(a))

#define TRUE 1
#define FALSE 0
#define NIL (void *) 0L
#define abs(a) ((a > 0) ? (a) : -(a))

#define outOfMemory 1
#define badPtr 2
#define otherError 3

#define QD32TRAP 0xAB03
#define PRIV_TRAP 0xA198

typedef short ErrorCode;

extern void InitToolbox(void);
extern Boolean TrapAvailable(short theTrap);
extern void HandleError(ErrorCode err);
extern Boolean IsKeyPressed(unsigned short k);

