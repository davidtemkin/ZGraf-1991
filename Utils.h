/*
 * ZGraf - Utils.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "ZGraf.h"

	// Efficient C versions of HiWord
	//   and LoWord Traps

#define		HiShort(longNum)	(short)(((longNum) >> 16) & 0xFFFF)
#define		LoShort(longNum)	(short)((longNum) & 0xFFFF)

	// Macros for extracting the first
	//   and last 8 bits of a short

#define		HiByte(shortNum)	(((shortNum) >> 8) & 0x00FF)
#define		LoByte(shortNum)	((shortNum) & 0x00FF)

	// Macro for absolute value
	//   x may be any numeric type

#define		Abs(x)			((x) < 0 ? -(x) : (x))

	// Macros for maximum and minimum
	//   of two numbers

#define		Max(x, y)		((x) > (y) ? (x) : (y))
#define		Min(x, y)		((x) < (y) ? (x) : (y))

	// turn on QuickDraw's hilite mode with
	// bclr #7,0x938
						
void SetHiliteMode(void) = {0x08B8, 0x0007, 0x0938};

// globals

extern char MMU32Bit : 0xCB2;
extern long gOldExit;

// Constants

#define kPaintWidth 576
#define kPaintHeight 720
#define kPaintHeaderSize 512
#define kPaintRowBytes 72

#define GetMBarHeight() (* (short*) 0x0BAA)

#define QD32TRAP 0xAB03
#define PRIV_TRAP 0xA198

// Functions

void InitUtils();
void InitToolbox(void);
void ForgetPtr(void *ptr);
Boolean TrapAvailable(short theTrap);
Boolean IsKeyPressed(unsigned short k);
void CalcVectors(short angle, Fixed velocity, Fixed* xVel, Fixed* yVel);
long cstrlen(char *);
void PrintBinaryLong(long l);
void PrintBinaryByte(char c);
void PrintRect(Rect *r);
ZBitMap *LoadPaintFile(char *fileName);
void SetUpDevice(void);
void PercentColor(RGBColor *c,
	short redPercent, short greenPercent, short bluePercent);
void PrintColor(RGBColor *c);
void InputColor(RGBColor *c);
long *AdjustAddress(long *addr32Bit);
Fixed FRand(Fixed low, Fixed high);
short SRand(short low, short high);
void SwapCalls(void *didCall, void *nowCall);
void SwapGraphicsFunctions(void);
void DoFlushIfRequired(void);

void ErrorAlert(short errNo, long errMsgIndex);
void CopyPString( ConstStr255Param srcString, Str255 destString);
void PositionDialog(ResType theType, short theID);

void InstallPatch(void);