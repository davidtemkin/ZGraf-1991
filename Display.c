/*
 * ZGraf - Display.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include <Retrace.h>

#include "Inheritance.h"

#include "Anim.h"
#include "Display.h"

#define CLASS		Display
#define SUPERCLASS	Root

#define methodDefinitions													\
			definesMethod (IDisplay);										\
			definesMethod (InitGraphics);									\
			definesMethod (InitRects);										\
			definesMethod (InitBitMaps);									\
			definesMethod (BlankScreen);									\
			definesMethod (DrawInitialStatus);								\
			definesMethod (ProcessEvent);									\
			definesMethod (RenderTarget);									\
			definesMethod (DrawTarget);										\
			definesMethod (ClearBuffers);									\
			definesMethod (WaitForVBL);										\
			definesMethod (ShowAbsRect);									\
			definesMethod (ShowStatus);										\
			definesMethod (ShowBuffers);									\
			definesMethod (CleanUp);										\
			definesMethod (Free);											\


void IDisplayClass (
	DisplayClass *classPtr )
{
	IRootClass((RootClass *) classPtr);
}

defineMethod (IDisplay) (
	Display *self )
{
	(method(IRoot));

	var(useFullScreen) = true;
	var(useVideoSync) = gConfig.useVBL;
	var(hasSlots) = gConfig.hasColorQD;
	
	(method(InitGraphics));

	if (gConfig.useOrigAltScreen)
		var(useFlipping) = true;
	else
		var(useFlipping) = false;

	(method(InitRects));
	(method(InitBitMaps));
	(method(BlankScreen));
	
	var(frameNo) = 0L;
}

defineMethod (InitGraphics) (
	Display *self )
{
	/* Set up ZGraf and ZDepth data structures */
	
	ZInitGraf();
	DInit(50, 0, 0);
}

defineMethod (InitRects) (
	Display *self )
{
	short statusDist;

	/* Calculate screen rectangles */

	ZSetRect(&var(compactScreen), 0, 0, kCompactWidth, kCompactHeight);
	ZSetRect(&var(fullScreen), 0, 0,
		zPage1BitMap->xBound, zPage1BitMap->yBound);
	if (var(useFullScreen))
		var(gameRect) = var(fullScreen);
	else
		var(gameRect) = var(compactScreen);

//	OffsetRect(&var(gameRect), (var(fullScreen).right - var(gameRect).right) / 2,
//		(var(fullScreen).bottom - var(gameRect.bottom)) / 2);

	statusDist = (var(gameRect).bottom - kStatusHeight) / 2;
	ZSetRect(&var(statusRect),
		var(gameRect).right - kStatusWidth,
		var(gameRect).top + statusDist,
		var(gameRect).right,
		var(gameRect).top + statusDist + kStatusHeight);
	ZSetRect(&var(radarRect),
		var(statusRect).left + kRadarFrame,
		var(statusRect).top + kRadarFrame,
		var(statusRect).left + kRadarFrame + kRadarWidth,
		var(statusRect).bottom - kRadarFrame);
	ZSetRect(&var(radarCopyRect),
		var(statusRect).left,
		var(statusRect).top + kRadarFrame,
		var(statusRect).left + 32,
		var(statusRect).bottom - kRadarFrame);
	ZSetRect(&var(viewRect),
		var(gameRect).left,
		var(gameRect).top,
		var(statusRect).left,
		var(gameRect).bottom);
	var(xViewSize) = var(viewRect).right -
		var(viewRect).left;
	var(yViewSize) = var(viewRect).bottom -
		var(viewRect).top;
}

defineMethod (InitBitMaps) (
	Display *self )
{
	short fontID;

	/* Set up game bitmaps */
	
	GetFNum("\pB Frutiger Bold", &fontID);

	if (var(useFlipping)) {
		var(full1BM) = zPage1BitMap;
		var(full2BM) = zPage2BitMap;
		var(renderFullBM) = zPage2BitMap;

		var(view1BM) = ZDupBitMap(zPage1BitMap);
		ZSetBitMap(var(view1BM));
		ZSetOrigin(-(var(viewRect).left + var(xViewSize) / 2),
			-(var(viewRect).top + var(yViewSize) / 2));
		ZSetAbsClipRect(&var(viewRect));
		
		var(view2BM) = ZDupBitMap(zPage2BitMap);
		ZSetBitMap(var(view2BM));
		ZSetOrigin(-(var(viewRect).left + var(xViewSize) / 2),
			-(var(viewRect).top + var(yViewSize) / 2));
		ZSetAbsClipRect(&var(viewRect));		
		
		var(radar1BM) = ZDupBitMap(zPage1BitMap);
		ZSetBitMap(var(radar1BM));
		ZSetOrigin(-(var(radarRect).left + kRadarWidth / 2),
			-(var(radarRect).top + kRadarHeight / 2));
		ZSetAbsClipRect(&var(radarRect));

		var(radar2BM) = ZDupBitMap(zPage2BitMap);
		ZSetBitMap(var(radar2BM));
		ZSetOrigin(-(var(radarRect).left + kRadarWidth / 2),
			-(var(radarRect).top + kRadarHeight / 2));
		ZSetAbsClipRect(&var(radarRect));

		var(status1BM) = ZDupBitMap(zPage1BitMap);
		ZSetBitMap(var(status1BM));
		TextFont(fontID);
		TextMode(srcXor);
		ZSetOrigin(-var(statusRect).left, -var(statusRect).top);
		ZSetAbsClipRect(&var(statusRect));
		
		var(status2BM) = ZDupBitMap(zPage2BitMap);
		ZSetBitMap(var(status2BM));
		TextFont(fontID);
		TextMode(srcXor);
		ZSetOrigin(-var(statusRect).left, -var(statusRect).top);
		ZSetAbsClipRect(&var(statusRect));
		
		var(renderViewBM) = var(view2BM);
		var(renderRadarBM) = var(radar2BM);
		var(renderStatusBM) = var(status2BM);
		}
	else {
		var(renderFullBM) = ZNewBitMap(zPage1BitMap->xBound,
			zPage1BitMap->yBound);

		var(renderViewBM) = ZDupBitMap(var(renderFullBM));
		ZSetBitMap(var(renderViewBM));
		ZSetOrigin(-(var(viewRect).left + var(xViewSize) / 2),
			-(var(viewRect).top + var(yViewSize) / 2));
		ZSetAbsClipRect(&var(viewRect));

		var(renderRadarBM) = ZDupBitMap(var(renderFullBM));
		ZSetBitMap(var(renderRadarBM));
		ZSetOrigin(-(var(radarRect).left + kRadarWidth / 2),
			-(var(radarRect).top + kRadarHeight / 2));
		ZSetAbsClipRect(&var(radarRect));

		var(renderStatusBM) = ZDupBitMap(var(renderFullBM));
		ZSetBitMap(var(renderStatusBM));
		TextFont(fontID);
		TextMode(srcXor);
		ZSetOrigin(-var(statusRect).left, -var(statusRect).top);
		ZSetAbsClipRect(&var(statusRect));
		}
}

defineMethod (BlankScreen) (
	Display *self )
{
	WindowPtr wPtr;

	/* Set up the screen */
	
	wPtr = NewWindow((WindowPeek) &var(gameWindow), &var(fullScreen),
		(StringPtr) "\pNo Title", true,
		dBoxProc, (WindowPtr) -1, false, 0L);
		
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
	Display *self )
{
	Rect r;
	PicHandle statusPict = nil;
	ZBitMap *tmp;
	OSErr err;

	// Get things in sync so page 1 is visible

	if (var(useFlipping)) {
		if (!var(page1Visible)) {
			var(renderViewBM) = var(view2BM);
			var(renderRadarBM) = var(radar2BM);
			var(renderFullBM) = var(full2BM);
			asm { bset.b #vPage2,vBase+vBufA }
			var(page1Visible) = true;
			}
		}
	
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


defineMethod (ProcessEvent) (
	Display *self,
	EventRecord *event )
{
}

defineMethod (RenderTarget) (
	Display *self )
{
	SHADE(zWhite);
	ZHLine(-20, -6, -1);
	ZHLine(-20, -6, 0);
	ZHLine(5, 19, -1);
	ZHLine(5, 19, 0);

	ZVSLine(-20, -6, -1, true, 0);
	ZVSLine(-20, -6, 0, true, 0);
	ZVSLine(5, 20, -1, true, 0);
	ZVSLine(5, 20, 0, true, 0);

	SHADE(zBlack);
	ZHLine(-20, -6, -2);
	ZHLine(-20, -6, 1);
	ZHLine(5, 19, -2);
	ZHLine(5, 19, 1);

	ZVSLine(-20, -6, -2, false, 0);
	ZVSLine(-20, -6, 1, false, 0);
	ZVSLine(5, 19, -2, false, 0);
	ZVSLine(5, 19, 1, false, 0);
}

defineMethod (DrawTarget) (
	Display *self )
{
	return;
	BITS(var(renderViewBM));
	(method(RenderTarget));	
}


defineMethod (WaitForVBL) (
	Display *self )
{
	syncTask.isVBL = false;		/* Reset VBL indicator */
	
	if (gConfig.useOrigAltScreen)
		syncTask.doFlip = 1 + var(page1Visible);

	while(!syncTask.isVBL)
		;						/* Wait for vertical blank */

}

defineMethod (ClearBuffers) (
	Display *self )
{
	/* Blank the viewport */

	if (gConfig.req32BitSwitch && var(useFlipping)) asm {
		moveq	#1, d0
		_SwapMMUMode
		}

	BITS(var(renderFullBM));
	ZClearLongs(&var(viewRect));
	SHADE(zBlack);
	ZFillRect(&var(radarRect));

	if (gConfig.req32BitSwitch && var(useFlipping)) asm {
		clr.l	d0
		_SwapMMUMode
		}
}

defineMethod (ShowAbsRect) (
	Display *self,
	Rect *absUpdateRect,
	short xDst,
	short yDst )
{
	ZCopyLongs(var(renderFullBM), zPage1BitMap, absUpdateRect, 
		xDst, yDst);
}

defineMethod (ShowStatus) (
	Display *self,
	ZBitMap *srcStatus, 
	ZBitMap *dstStatus )
{
	ZCopyLongs(srcStatus, dstStatus, &var(statusRect), var(statusRect).left,
			var(statusRect).top);
}

defineMethod (ShowBuffers) (
	Display *self )
{
	OSErr err;

	if (gConfig.useVBL || gConfig.useOrigAltScreen)
		(method(WaitForVBL));

	if (var(useFlipping)) {
		if (var(page1Visible)) {
		
			// Page 2 has just been shown
		
			(method(ShowStatus), var(status2BM), var(status1BM));
			var(renderViewBM) = var(view1BM);
			var(renderRadarBM) = var(radar1BM);
			var(renderFullBM) = var(full1BM);
			var(renderStatusBM) = var(status1BM);
			var(page1Visible) = false;
			}
		else {
		
			// Page 1 has just been shown
		
			(method(ShowStatus), var(status1BM), var(status2BM));
			var(renderViewBM) = var(view2BM);
			var(renderRadarBM) = var(radar2BM);
			var(renderFullBM) = var(full2BM);
			var(renderStatusBM) = var(status2BM);
			var(page1Visible) = true;
			}
		}

	else {		/* no page flipping */
	
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
}

defineMethod (CleanUp) (
	Display *self,
	long inhibitExceptions )
{
	OSErr err;

	if (gConfig.req32BitSwitch) {
	
		// if 32-bit addressing has been left on (for whatever reason),
		//   revert to 24-bit as part of the cleanup process.
	
		if (GetMMUMode() == true32b) {
			asm {
				clr.l	d0
				_SwapMMUMode
				}
			}
		}

	if (var(useFlipping)) {
		asm { bset.b #vPage2, vBase+vBufA }
		*(zPage2BitMap->baseAddr) = 'MINE';
		}

	if (gConfig.isSyncTaskInstalled) {
		if (var(hasSlots)) {
			err = SlotVRemove((QElem *) &syncTask, gConfig.slotNo);
			if (!inhibitExceptions)
				FailOSErr(err);
			}
		else {
			err = VRemove((QElem *) &syncTask);
			if (!inhibitExceptions)
				FailOSErr(err);
			}
		gConfig.isSyncTaskInstalled = false;
		}
	
	if (gConfig.isTimeTaskInstalled) {
		err = VRemove((QElem *) &timeTask);
		if (!inhibitExceptions)
			FailOSErr(err);
		gConfig.isTimeTaskInstalled = false;
		}

	FlushEvents(everyEvent, 0);
	ShowCursor(); ShowCursor();
}

defineMethod (Free) (
	Display *self )
{
	
	// Fix: some pointers are freed twice.
	// need a safe way to free pointers

	if (!var(useFlipping)) {
		ZFreeBitMap(var(renderViewBM));
		ZFreeBitMap(var(renderRadarBM));
		}
	
	(inherited(Free));
}

defineClassObject;