/*
 * ZGraf - CDisplay.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include "Inheritance.h"

#include "Tunnel.h"
#include "Anim.h"
#include "CDisplay.h"
#include "CDepth.h"


#define CLASS		CDisplay
#define SUPERCLASS	Display

#define methodDefinitions													\
			definesMethod (ICDisplay);										\
			definesMethod (InitGraphics);									\
			definesMethod (InitBitMaps);									\
			definesMethod (BlankScreen);									\
			definesMethod (DrawInitialStatus);								\
			definesMethod (DrawTarget);										\
			definesMethod (ShowAbsRect);									\
			definesMethod (ShowStatus);										\
			definesMethod (ShowBuffers);									\
			definesMethod (CleanUp);										\

void ICDisplayClass (
	CDisplayClass *classPtr )
{
	IDisplayClass((DisplayClass *) classPtr);
}

defineMethod (ICDisplay) (
	CDisplay *self )
{
	var(useColor) = kColor;
	
	(method(IDisplay));
}

defineMethod (InitGraphics) (
	CDisplay *self )
{
	if (var(useColor) == kFakeColor)
		CInitGraf(false);
	else
		CInitGraf(true);
	DInit(50, 140, 0);
	
	/* 400, 140, 10 */

	SwapGraphicsFunctions();
}

defineMethod (InitBitMaps) (
	CDisplay *self )
{
	(inherited(InitBitMaps));

	var(leftViewBM) = ZDupBitMap(var(renderViewBM));
	var(leftViewBM)->xOrig += origOffset / 2;
	ZSetBitMap(var(leftViewBM));
	CSetColor(kGreen);
	
	var(rightViewBM) = ZDupBitMap(var(renderViewBM));
	var(rightViewBM)->xOrig -= origOffset / 2;
	ZSetBitMap(var(rightViewBM));
	CSetColor(kRed);

	ZSetBitMap(var(renderRadarBM));
	CSetColor(kAmber);
	
	ZSetBitMap(var(renderStatusBM));
	RGBForeColor(&rgbAmber);
	RGBBackColor(&rgbBlack);
	TextMode(srcBic);
	CSetColor(kAmber);

	leftEyeBM = var(leftViewBM);
	rightEyeBM = var(rightViewBM);
}

defineMethod (BlankScreen) (
	CDisplay *self )
{
	WindowPtr wPtr;
	Rect winRect;

	/* Set up the screen */

	winRect = var(fullScreen);
	if (var(useColor) == kFakeColor)
		winRect.right *= 2;

	wPtr = NewWindow((WindowPeek) &var(gameWindow), &winRect,
		(StringPtr) "\pNo Title", true,
		dBoxProc, (WindowPtr) -1, false, 0L);

	CPrepareDevice();
	HideCursor(); HideCursor();
	ZSetBitMap(zPage1BitMap);
	if (gConfig.req32BitSwitch) asm {
		moveq	#1, d0
		_SwapMMUMode
		}
	SHADE(zBlack);
	ZFillRect(&var(fullScreen));
	BITS(var(renderFullBM));
	ZFillRect(&var(fullScreen));
	if (gConfig.req32BitSwitch) asm {
		clr.l	d0
		_SwapMMUMode
		}
}

defineMethod (DrawInitialStatus) (
	CDisplay *self )
{
	PicHandle statusPict = nil;
	ZBitMap *tmp;
	Rect r;

	ZSetBitMap(var(renderFullBM));
	RGBForeColor(&rgbBlack);
	RGBBackColor(&rgbAmber);

	tmp = ZNewBitMap(var(statusRect).right - var(statusRect).left,
		var(statusRect).bottom - var(statusRect).top);
	FailNILRes(statusPict = (PicHandle) GetResource('PICT', kSidePict));
	ZSetBitMap(tmp);
	ZSetRect(&r, 0, 0, var(statusRect).right - var(statusRect).left,
		var(statusRect).bottom - var(statusRect).top);
	SHADE(zWhite);
	ZFillRect(&r);
	DrawPicture(statusPict, &r);

	/* Show it */
	
	if (gConfig.req32BitSwitch) asm {
		moveq	#1, d0
		_SwapMMUMode
		}
	ZCopyLongs(tmp, var(renderFullBM), &r, var(statusRect).left,
		var(statusRect.top));

	(method(ShowAbsRect), &var(fullScreen), 0, 0);

	if (gConfig.req32BitSwitch) asm {
		clr.l	d0
		_SwapMMUMode
		}

	ZFreeBitMap(tmp);
	ReleaseResource((Handle) statusPict);
}

defineMethod (DrawTarget) (
	CDisplay *self )
{
	short holdOrig;
   return;
	BITS(var(leftViewBM));
		holdOrig = zCurBitMap->xOrig;
		zCurBitMap->xOrig -= var(halfTargetOffset);
	(method(RenderTarget));
		zCurBitMap->xOrig = holdOrig;
		
	BITS(var(rightViewBM));
		holdOrig = zCurBitMap->xOrig;
		zCurBitMap->xOrig += var(halfTargetOffset);
	(method(RenderTarget));
		zCurBitMap->xOrig = holdOrig;
}


defineMethod (ShowAbsRect) (
	CDisplay *self,
	Rect *absUpdateRect,
	short xDst,
	short yDst )
{
	// Note that this method does not switch addressing modes.

	switch (gConfig.gameScreenDepth) {
		case 2:
			ZCopyLongs(var(renderFullBM), zPage1BitMap, absUpdateRect, 
				xDst, yDst);
			break;
		case 4:
			ZExpand2To4(var(renderFullBM), zPage1BitMap, absUpdateRect, 
				xDst, yDst);
			break;
		case 8:
			ZExpand2To8(var(renderFullBM), zPage1BitMap, absUpdateRect, 
				xDst, yDst);
			break;
		}
}

defineMethod (ShowStatus) (
	CDisplay *self,
	ZBitMap *srcStatus,
	ZBitMap *dstStatus )
{
	// The srcStatus and dstStatus parameters are ignored here since 
	//   color displays don�t support page flipping.

	(method(ShowAbsRect), &var(statusRect), var(statusRect).left,
		var(statusRect).top);
}

defineMethod (ShowBuffers) (
	CDisplay *self )
{
	OSErr err;
	short oldTicks;

	if (gConfig.useVBL)
		(method(WaitForVBL));

	if (gConfig.req32BitSwitch) asm {
		moveq	#1, d0
		_SwapMMUMode
		}
		
	(method(ShowAbsRect), &var(gameRect), var(gameRect).left,
			var(gameRect).top);

/*
	(method(ShowAbsRect), &var(viewRect),
		var(viewRect).left, var(viewRect).top);
	(method(ShowAbsRect), &var(radarCopyRect),
		var(radarCopyRect).left, var(radarCopyRect).top);
	(method(ShowStatus), var(renderStatusBM), zPage1BitMap);
*/
		
	if (gConfig.req32BitSwitch) asm {
		clr.l	d0
		_SwapMMUMode
		}
}


defineMethod (CleanUp) (
	CDisplay *self,
	long inhibitExceptions )
{
	(inherited(CleanUp), inhibitExceptions);

	DoFlushIfRequired();
	CCleanUpDevice(inhibitExceptions);
}

defineClassObject;
