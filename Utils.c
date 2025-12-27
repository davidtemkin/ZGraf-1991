/*
 * ZGraf - Utils.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include <Traps.h>
#include <Menus.h>
#include "JumpTable.h"
#include <stdio.h>
#include "Exceptions.h"

#include "CGraf.h"
#include "CDepth.h"
#include "CExpl.h"
#include "Anim.h"
#include "Utils.h"

unsigned long rSeed = 0;
Fract cosTable[360], sinTable[360];
long gOldExit;

void InitUtils()
{
	short i;
	float temp;
	rSeed = TickCount();

	for (i = 0; i< 360; i++)
		{
		temp = 2 * 3.141601625 * (i - 180) / 360;
		cosTable[i] = FracSin(X2Fix(temp));;
		sinTable[i] = FracCos(X2Fix(temp));
		}

	{
		char s[100];
		long x;
	
		// Symantec�s ANSI library can fail when memory is low.
		// Calling it for the first time seems to force 
		// an allocation -- better to initialize it here when
		// there�s definitely enough memory.
	
		sprintf(s, "%ld", x);
	}
	
	InstallPatch();
}

void InitToolbox ()
{
	InitGraf(&qd.thePort);
	InitCursor();
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	MaxApplZone();
	
	/* Initialize random number generator */
	
	GetDateTime((unsigned long *) &qd.randSeed);
}

void ForgetPtr (
	void *ptr )
{
	if (ptr) {
		ZDisposePtr((Ptr) ptr);
		FailMemError();
		}
}

Boolean TrapAvailable (
	short theTrap )
{
	TrapType tType;
	short    numToolBoxTraps;
	
				// first determine the trap type
				
	tType = (theTrap & 0x800) > 0 ? ToolTrap : OSTrap;
	
				// next find out how many traps there are
				
	if (NGetTrapAddress( _InitGraf, ToolTrap) == NGetTrapAddress( 0xAA6E, ToolTrap))
		numToolBoxTraps = 0x200;
	else
		numToolBoxTraps = 0x400;
	
				// check if the trap number is too big for the
				// current trap table
				
	if (tType == ToolTrap)
	{
		theTrap &= 0x7FF;
		if (theTrap >= numToolBoxTraps)
			theTrap = _Unimplemented;
	}
	
				// the trap is implemented if its address is
				// different from the unimplemented trap
				
	return (NGetTrapAddress( theTrap, tType) != 
			NGetTrapAddress(_Unimplemented, ToolTrap));
}
	
Boolean IsKeyPressed (
	unsigned short k )

{
	unsigned char km[16];

	asm {
		pea		km
		GetKeys
		clr.l (sp)+
		}

	return ((km[k>>3] >> (k&7)) & 1);
}

void CalcVectors(short angle, Fixed velocity, Fixed* xVel, Fixed* yVel)
{
  *xVel = FracMul(cosTable[angle], velocity);
  *yVel = FracMul(sinTable[angle], velocity);
}

long cstrlen (
	register char *s )
{
	register long count = 0;
	
	while (*s++)
		count++;

	return count;
}

void PrintBinaryLong (
	long l )
{
	int b;
	
	/*
		Prints an ASCII representation of the binary contents of the
		passed long, from highest to lowest significance.
	*/
	
	for (b = 0; b < 32; b++) {
		if (BitTst((Ptr) &l, b))
			printf("*");
		else
			printf(".");
		}
	printf("\n");
}

void PrintBinaryByte (
	char c )
{
	int i;
	for (i=0; i<8; i++) {
		if (BitTst(&c,i))
			printf("*");
		else
			printf(".");
		}
	printf("\n");
}

void PrintRect (
	Rect *r )
{
	printf("%d %d %d %d\n", r->left, r->top, r->right, r->bottom);
}

ZBitMap *LoadPaintFile (
	char *fileName )
{
	ZBitMap *bm;
	long headerSize = kPaintHeaderSize;
	void *headerBuf, *readBuf;
	char vName[256];
	short vRefNum, fRefNum;
	short scanLine;
	void *srcPtr, *dstPtr;
	long fileSize;
	OSErr err;

	/*
		Reads a MacPaint file into a ZBitMap structure.
		Returns NIL if memory can't be allocated.
	*/

	bm = ZNewBitMap(kPaintWidth, kPaintHeight);

	err = GetVol((StringPtr) vName, &vRefNum);
	err = FSOpen((StringPtr) CtoPstr(fileName), vRefNum, &fRefNum);
	err = GetEOF(fRefNum, &fileSize);
	headerBuf = (void *) ZNewPtr(512L);
	readBuf = (void *) ZNewPtr(fileSize);
	err = SetFPos(fRefNum, fsFromStart, 0L);
	err = FSRead(fRefNum, &headerSize, headerBuf);
	err = FSRead(fRefNum, &fileSize, readBuf);
	
	srcPtr = readBuf;
	dstPtr = bm->baseAddr;
	for (scanLine = 1; scanLine <= kPaintHeight; scanLine++)
		UnpackBits((Ptr *) &srcPtr, (Ptr *) &dstPtr, kPaintRowBytes);
	DisposPtr(headerBuf);
	DisposPtr(readBuf);

	return(bm);
}

void PercentColor(
	RGBColor *c,
	short redPercent, short greenPercent, short bluePercent )
{
	double tRed, tGreen, tBlue;
	
	tRed = (double) 65535 * redPercent / (double) 100;
	c->red = (unsigned short) tRed;
	tGreen = (double) 65535 * greenPercent / (double) 100;
	c->green = (unsigned short) tGreen;
	tBlue = (double) 65535 * bluePercent / (double) 100;
	c->blue = (unsigned short) tBlue;
}

void PrintColor(
	RGBColor *c )
{

	printf("Red: $%X (%u), Green: $%X (%u), Blue: $%X (%u)\n",
		c->red, c->red, c->green, c->green, c->blue, c->blue);

}

void InputColor (
	RGBColor *c )
{
	short redPercent, greenPercent, bluePercent;

	printf("Red percent: ");
	scanf("%d", &redPercent);
	printf("Green percent: ");
	scanf("%d", &greenPercent);
	printf("Blue percent: ");
	scanf("%d", &bluePercent);
	PercentColor(c, redPercent, greenPercent, bluePercent);
}

/*
	Function: AdjustAddress
	
	On some Mac II-family machines, the address of the screen
	buffer is a 32-bit address. When 24-bit addressing is active,
	it is not possible to access this buffer through this address.
*/

long *AdjustAddress  (
	long *addr32Bit )
{
	long addr = (long) addr32Bit;
	long a, b, c, d;
	
	/*
		A 32-bit screen address of the form:
			Fsxx xxxx
		is equivalent to a 24-bit address of the form:
			  sx xxxx
		where s is the slot no. of the video card. The low-order
		five digits of the 24-bit address are the same as the
		last five digits of the 32-bit address.
		
		This method can also be used to extract the slot number
		of the current video device when installing a VBL task
		for that display.

		I believe it is not neccessary to transform 32-bit addresses
		into 24-bit addresses when the computer is running in 32-bit
		mode. Incidentally, the whole scheme is likely to break on
		some future hardware or operating system release.

	*/
	
	if (MMU32Bit == false32b) {
		a = (addr & 0xF0000000) >> 28;
		b = (addr & 0x0F000000) >> 24;
		if ((a == 0xF) && ((b >= 0x9) && (b <= 0xE))) {
			addr = addr & 0x000FFFFF;
			b = b << 20;
			addr = addr | b;
			}
		}	
	return ((long *) addr);
}

Fixed FRand (Fixed low, Fixed high )
{
	/*
		Produces random numbers between low and high -- 
		including low but not including high.
	*/
#if 0
	long num, range, rand;
	range = (long) (F2S(high) - F2S(low));
	num = (long) (Random() & 0x7FFF);
	rand =  (range * num) / 32767;
	return (low + S2F(rand));
#else
	return S2F(SRand(F2S(low), F2S(high)));
#endif
}

short SRand (
	short low, short high )
{
	double num, range;

	/*
		Produces random numbers between low and high -- 
		including low but not including high.
	*/
#if 0
	range = (double) (high - low);
	num = ((double) Random() + 32767.0) * range / ((double) 65536);
	return ((short) num + low);
#else
	rSeed = rSeed * 5709421UL + 1UL;
	
	return ((rSeed >> 16UL) % (high - low)) + low;
#endif
}

void SwapCalls (
	void *didCall, void *nowCall )
{
	LoadedJTEntry holdCall;

	/*
		NOTE:
	
		In order for this to work, the functions in question
		must be located in a different segment from the function 
		which calls this function.
		
	*/

	holdCall = *(LoadedJTEntry *) didCall;
	*(LoadedJTEntry *) didCall = *(LoadedJTEntry *) nowCall;
	*(LoadedJTEntry *) nowCall = holdCall;

}

#define _CacheFlushTrap		0xA0BD

void DoFlushIfRequired ()
{

#if 0

	// Copied out of TCL. This uses a "private" trap.
	//   Probably not the right thing, but this is what they use...

	if (TrapAvailable( _CacheFlushTrap)) {
		asm { dc.w _CacheFlushTrap }
		}
#else

	// This is the one that I wrote based on the docs.

	if (TrapAvailable(_HWPriv)) {
		FlushDataCache();
		FlushInstructionCache();
		}
#endif

}

void SwapGraphicsFunctions ()
{

	// Code which swaps jump table entries is self-modifying code.
	// Fix for 68040.

	DoFlushIfRequired();

	SwapCalls((void *) &ZInitBitMap, (void *) &CInitBitMap);
	SwapCalls((void *) &ZNewBitMap, (void *) &CNewBitMap);
	SwapCalls((void *) &ZDupBitMap, (void *) &CDupBitMap);
	SwapCalls((void *) &ZFreeBitMap, (void *) &CFreeBitMap);
	SwapCalls((void *) &ZGetPixel, (void *) &CGetPixel);
	SwapCalls((void *) &ZSetPixel, (void *) &CSetPixel);
	SwapCalls((void *) &ZHLine, (void *) &CHLine);
	SwapCalls((void *) &ZVLine, (void *) &CVLine);
	SwapCalls((void *) &ZVSLine, (void *) &CVSLine);
	SwapCalls((void *) &ZFrameRect, (void *) &CFrameRect);
	SwapCalls((void *) &ZFillRect, (void *) &CFillRect);
	SwapCalls((void *) &ZCopyLongs, (void *) &CCopyLongs);
	SwapCalls((void *) &ZClearLongs, (void *) &CClearLongs);
	
	SwapCalls((void *) &DHLine, (void *) &CDHLine);
	SwapCalls((void *) &DVLine, (void *) &CDVLine);
	SwapCalls((void *) &DVSLine, (void *) &CDVSLine);
	SwapCalls((void *) &DFrameRect, (void *) &CDFrameRect);
	SwapCalls((void *) &DFillRect, (void *) &CDFillRect);
	SwapCalls((void *) &DSetPixel, (void *) &CDSetPixel);
	SwapCalls((void *) &DTunnelRect, (void *) &CDTunnelRect);

	SwapCalls((void *) &EDHLine, (void *) &CEDHLine);
	SwapCalls((void *) &EDVLine, (void *) &CEDVLine);
	SwapCalls((void *) &EDVSLine, (void *) &CEDVSLine);
	SwapCalls((void *) &EDFrameRect, (void *) &CEDFrameRect);
	SwapCalls((void *) &EDFillRect, (void *) &CEDFillRect);
	SwapCalls((void *) &EDSetPixel, (void *) &CEDSetPixel);
	
	DoFlushIfRequired();
}

void CopyPString(
	ConstStr255Param srcString,
	Str255 destString )
{
	BlockMove(srcString, destString, srcString[0] + 1L);
}

/******************************************************************************

 	TCL: PositionDialog
 
		Center the bounding box of a dialog or alert in the upper third
		of the screen.  This is the preferred location according to the
		Human Interface Guidelines.

 ******************************************************************************/

void	PositionDialog(
	ResType		theType,				/* Resource type, DLOG or ALRT		*/
	short		theID)					/* Res ID of DLOG or ALRT			*/
{
	Handle				theTemplate;	/* Handle to resource template		*/
	register Rect		*theRect;		/* Bounding box of dialog			*/
	register short		left;			/* Left side of centered rect		*/
	register short		top;			/* Top side of centered rect		*/
	
		/* The first field of the resource template for DLOG's and ALRT's */
		/* is its bounding box.  Get a pointer to this rectangle.  This   */
		/* handle dereferencing is safe since the remaining statements in */
		/* this function do not move memory (assignment and simple math). */

	theTemplate = GetResource(theType, theID);	// TCL 1.1.3 11/30/92 BF
	FailNILRes(theTemplate);
	theRect = (Rect*) *theTemplate;
	
										/* Center horizontally on screen	*/
	left = (qd.screenBits.bounds.right - (theRect->right - theRect->left)) / 2;

										/* Leave twice as much space below	*/
										/*   as above the rectangle			*/	
	top = (qd.screenBits.bounds.bottom - (theRect->bottom - theRect->top)) / 3;
										/* Don't put rect under menu bar	*/
	top = Max(top, GetMBarHeight() + 1);

	theRect->right += left - theRect->left;
	theRect->left = left;
	theRect->bottom += top - theRect->top;
	theRect->top = top;
}


/******************************************************************************

	TCL: ErrorAlert
 
 	Simplified from TCL version.
 
 	Displays a standard error alert, given an error number and an index into
 	  an STR# resource holding a list of error messages. The error alert includes
 	  the numeric error code as well as the message, a brief textual description
 	  of what went wrong.
 
 	If errMsgIndex is 0, only a generic message will be provided.
 
 ******************************************************************************/

#define kErrorAlert				300		// for now, very simple dialog
#define kGenericErrorStrID		300		// STR resource containing generic error string 
#define kErrorMsgListID			131		// STR# resource with error strings (see Exceptions.h)

void ErrorAlert (
	short errNum,
	long errMsgIndex )
{
	Str255	errStr;
	Str63 numStr;
	StringHandle strH;


	// FIX: None of this should be able to raise an exception.

	SafeCleanUp();

	if (errMsgIndex == 0) {

		// Use generic message if none is supplied

		strH = GetString(kGenericErrorStrID);
		CopyPString(*strH, errStr);
		}

	else {
	
		// mark string empty
	
		errStr[0] = 0;
		GetIndString(errStr, kErrorMsgListID, (short) errMsgIndex);

		if (errStr[0] == 0) {
			strH = GetString(kGenericErrorStrID);
			if (strH)
				CopyPString(*strH, errStr);
			else
				CopyPString("\pSorry, an error has occurred.", errStr);
			}
		}
	
	
	NumToString(errNum, numStr);
	ParamText(errStr, (ConstStr255Param) numStr, nil, nil);

	// avoid infinite recursion in error handling by specifically
	// testing if the ALRT and DITL resources we need are there.

  	if ((GetResource('ALRT', kErrorAlert) == nil) || (GetResource('DITL', kErrorAlert) == nil))
   		SafeQuit();	// nothing else we can do...

	PositionDialog('ALRT', kErrorAlert);
	InitCursor();
	StopAlert(kErrorAlert, nil);
}

/******************************************************************************


	Patches ExitToShell. This way, we can be sure that everything is
	properly cleaned up in case of an abnormal exit.

	Closes SoundKit, removes all VBLs, restores the display to normal.
	
 
 ******************************************************************************/

void ExitToShell_1 (void)
{
	SafeCleanUp();

	asm	{
		move.l	gOldExit,A0
		jmp		(A0)
		}
}

void InstallPatch ()
{	
	gOldExit = NGetTrapAddress(0x9F4, ToolTrap);
	NSetTrapAddress((long) ExitToShell_1, 0x9F4, ToolTrap);
}
