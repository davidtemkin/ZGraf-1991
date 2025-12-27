/*
 * ZGraf - Vis.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include "Inheritance.h"

#include "Vis.h"
#include "Anim.h"

#define CLASS		Vis
#define SUPERCLASS	Root

#define methodDefinitions														\
			definesMethod (IVis);												\
			definesMethod (Process);											\
			definesMethod (Hilite);												\
			definesMethod (Draw);												\
			definesMethod (DrawRadar);											\
			definesMethod (CheckBounds);										\
			definesMethod (AdjustSize);											\
			definesMethod (Collided);											\



void IVisClass (
	VisClass *classPtr,
	short	  shapeResID )
{
	IRootClass((RootClass *) classPtr);

	classPtr->stdThick = S2F(30);
	
	if (shapeResID > 0)
		{

		Rect r;
		FRect fixedR;
		short sizeFactor;
		register short i;
		SShapeRect* sRects;
		ShapeRecHdl classShape;

		classShape = nil;
		classPtr->stdFRects = nil;
		FailNIL(classShape = (ShapeRecHdl)GetResource('zOBJ',shapeResID)); 
		MoveHHi((Handle)classShape);
		HLock((Handle)classShape);
		
		sizeFactor = (*classShape)->sSize;
		i = (*classShape)->sCount;
		sRects = &(*classShape)->sRects;
		classPtr->stdRectCount = i;

		FailNIL(classPtr->stdFRects = (FShapeRect*) ZNewPtr(sizeof(FShapeRect) * i));

		for (i = 0; i < (*classShape)->sCount; i++)
			{
			r = sRects[i].rRect;
			
			r.top 		*= sizeFactor;
			r.left 		*= sizeFactor;
			r.bottom 	*= sizeFactor;
			r.right 	*= sizeFactor;
			
			SetFRect(&fixedR , S2F(r.left), S2F(r.top), S2F(r.right), S2F(r.bottom));
			
			// If the rect is collidable then add it to the stdExtent for this class
			//if (sRects[i].rCollidable)
				{
				if (fixedR.top < classPtr->stdExtent.top) 
					classPtr->stdExtent.top = fixedR.top;
				if (fixedR.left < classPtr->stdExtent.left) 
					classPtr->stdExtent.left = fixedR.left;
				if (fixedR.bottom > classPtr->stdExtent.bottom) 
					classPtr->stdExtent.bottom = fixedR.bottom;
				if (fixedR.right > classPtr->stdExtent.right) 
					classPtr->stdExtent.right = fixedR.right;
				}
				
			classPtr->stdFRects[i].rFRect = fixedR;
			classPtr->stdFRects[i].rShade = sRects[i].rShade;
			if (classPtr->stdFRects[i].rShade > 127) 
				classPtr->stdFRects[i].rShade -= 254;
			classPtr->stdFRects[i].rFrame = sRects[i].rFrame > 0;
			classPtr->stdFRects[i].rCollidable = sRects[i].rCollidable > 0;
			classPtr->stdFRects[i].rColor = sRects[i].rColor;
			}
		HUnlock((Handle)classShape);
		ReleaseResource((Handle)classShape);
		}

	#define NORMAL	
	#ifdef NORMAL
		classPtr->stdExplIncr = S2F(125);
		classPtr->stdExplSteps = 25;
		classPtr->stdFormIncr = S2F(200);
		classPtr->stdFormSteps = 15;
	#else	
		classPtr->stdExplIncr = S2F(25);
		classPtr->stdExplSteps = 300;
		classPtr->stdFormIncr = S2F(25);
		classPtr->stdFormSteps = 300;
	#endif

}

defineMethod (IVis) (
	Vis *self,
	Fixed x, Fixed y, Fixed zFromPlayer,
	FRect *extent,
	Fixed thick,
	short checkCollide,
	Fixed xVel, Fixed yVel, Fixed zVel )
{
	var(x) = x;
	var(y) = y;
	var(zFromPlayer) = zFromPlayer;
	var(expl) = classVar(stdForm);
	var(extent) = *extent;
	var(thick) = thick;
	var(checkCollide) = checkCollide;
	var(isForm) = true;
	var(xVel) = xVel;
	var(yVel) = yVel;
	var(zVel) = zVel;
	var(fInstanceFRects) = classVar(stdFRects);
}

defineMethod (Process) (
	Vis *self )
{
	if (var(isForm)) 
		{
		var(isForm) = !IncrExpl(&var(expl));
		}
	else if (var(isExpl)) 
		{
		var(pleaseRemove) = IncrExpl(&var(expl));
		}
		
	if (var(xVel) || var(yVel))
		{
		var(x) += var(xVel);
		var(y) += var(yVel);
		(method(CheckBounds));
		}
	(message(theTunnel, MoveObject), self);
}

defineMethod (Hilite) (
	Vis *self,
	short hilite )
{
	var(hilite) = hilite;
}

//#define SHOWEXTENT
#define CALCSHADE(s) (s > 0 ? (&(*zShades)[s]) : &(*zCurDitherTable)[-s])
defineMethod (Draw) (
	Vis *self )
{
	register short i;
	short numRects = classVar(stdRectCount);
	FShapeRect sRect;
#ifdef SHOWEXTENT
	FRect fr;
	SHADE(zWhite);
	fr = classVar(stdExtent);
	SHADE(zWhite);
	DFrameRect(&fr, 0L);
	SetFRect(&fr , -f100, -f100, f100, f100);
	DFrameRect(&fr, 0L);
#endif
 	i = 0;
	
	do {
		switch (10 - (numRects - i) % 10) {
			case 10 :
				{
				sRect = var(fInstanceFRects)[i];		
				zCurPat = CALCSHADE(sRect.rShade);
				if (sRect.rFrame)
					if (var(isExpl) || var(isForm))
						EDFrameRect(&var(expl), &(sRect.rFRect), 0L);
					else
						DFrameRect(&(sRect.rFRect), 0L);
				else
					if (var(isExpl) || var(isForm))
						EDFillRect(&var(expl), &(sRect.rFRect), 0L);
					else
						DFillRect(&(sRect.rFRect), 0L);
				i++;
				}
			case 1 :
				{
				sRect = var(fInstanceFRects)[i];		
				zCurPat = CALCSHADE(sRect.rShade);
				if (sRect.rFrame)
					if (var(isExpl) || var(isForm))
						EDFrameRect(&var(expl), &(sRect.rFRect), 0L);
					else
						DFrameRect(&(sRect.rFRect), 0L);
				else
					if (var(isExpl) || var(isForm))
						EDFillRect(&var(expl), &(sRect.rFRect), 0L);
					else
						DFillRect(&(sRect.rFRect), 0L);
				i++;
				}
			case 2 :
				{
				sRect = var(fInstanceFRects)[i];		
				zCurPat = CALCSHADE(sRect.rShade);
				if (sRect.rFrame)
					if (var(isExpl) || var(isForm))
						EDFrameRect(&var(expl), &(sRect.rFRect), 0L);
					else
						DFrameRect(&(sRect.rFRect), 0L);
				else
					if (var(isExpl) || var(isForm))
						EDFillRect(&var(expl), &(sRect.rFRect), 0L);
					else
						DFillRect(&(sRect.rFRect), 0L);
				i++;
				}
			case 3 :
				{
				sRect = var(fInstanceFRects)[i];		
				zCurPat = CALCSHADE(sRect.rShade);
				if (sRect.rFrame)
					if (var(isExpl) || var(isForm))
						EDFrameRect(&var(expl), &(sRect.rFRect), 0L);
					else
						DFrameRect(&(sRect.rFRect), 0L);
				else
					if (var(isExpl) || var(isForm))
						EDFillRect(&var(expl), &(sRect.rFRect), 0L);
					else
						DFillRect(&(sRect.rFRect), 0L);
				i++;
				}
			case 4 :
				{
				sRect = var(fInstanceFRects)[i];		
				zCurPat = CALCSHADE(sRect.rShade);
				if (sRect.rFrame)
					if (var(isExpl) || var(isForm))
						EDFrameRect(&var(expl), &(sRect.rFRect), 0L);
					else
						DFrameRect(&(sRect.rFRect), 0L);
				else
					if (var(isExpl) || var(isForm))
						EDFillRect(&var(expl), &(sRect.rFRect), 0L);
					else
						DFillRect(&(sRect.rFRect), 0L);
				i++;
				}
			case 5 :
				{
				sRect = var(fInstanceFRects)[i];		
				zCurPat = CALCSHADE(sRect.rShade);
				if (sRect.rFrame)
					if (var(isExpl) || var(isForm))
						EDFrameRect(&var(expl), &(sRect.rFRect), 0L);
					else
						DFrameRect(&(sRect.rFRect), 0L);
				else
					if (var(isExpl) || var(isForm))
						EDFillRect(&var(expl), &(sRect.rFRect), 0L);
					else
						DFillRect(&(sRect.rFRect), 0L);
				i++;
				}
			case 6 :
				{
				sRect = var(fInstanceFRects)[i];		
				zCurPat = CALCSHADE(sRect.rShade);
				if (sRect.rFrame)
					if (var(isExpl) || var(isForm))
						EDFrameRect(&var(expl), &(sRect.rFRect), 0L);
					else
						DFrameRect(&(sRect.rFRect), 0L);
				else
					if (var(isExpl) || var(isForm))
						EDFillRect(&var(expl), &(sRect.rFRect), 0L);
					else
						DFillRect(&(sRect.rFRect), 0L);
				i++;
				}
			case 7 :
				{
				sRect = var(fInstanceFRects)[i];		
				zCurPat = CALCSHADE(sRect.rShade);
				if (sRect.rFrame)
					if (var(isExpl) || var(isForm))
						EDFrameRect(&var(expl), &(sRect.rFRect), 0L);
					else
						DFrameRect(&(sRect.rFRect), 0L);
				else
					if (var(isExpl) || var(isForm))
						EDFillRect(&var(expl), &(sRect.rFRect), 0L);
					else
						DFillRect(&(sRect.rFRect), 0L);
				i++;
				}
			case 8 :
				{
				sRect = var(fInstanceFRects)[i];		
				zCurPat = CALCSHADE(sRect.rShade);
				if (sRect.rFrame)
					if (var(isExpl) || var(isForm))
						EDFrameRect(&var(expl), &(sRect.rFRect), 0L);
					else
						DFrameRect(&(sRect.rFRect), 0L);
				else
					if (var(isExpl) || var(isForm))
						EDFillRect(&var(expl), &(sRect.rFRect), 0L);
					else
						DFillRect(&(sRect.rFRect), 0L);
				i++;
				}
			case 9 :
				{
				sRect = var(fInstanceFRects)[i];		
				zCurPat = CALCSHADE(sRect.rShade);
				if (sRect.rFrame)
					if (var(isExpl) || var(isForm))
						EDFrameRect(&var(expl), &(sRect.rFRect), 0L);
					else
						DFrameRect(&(sRect.rFRect), 0L);
				else
					if (var(isExpl) || var(isForm))
						EDFillRect(&var(expl), &(sRect.rFRect), 0L);
					else
						DFillRect(&(sRect.rFRect), 0L);
				i++;
				}
			}
		} while (numRects - i);
}

defineMethod (DrawRadar) (
	Vis *self )
{
	short xScreen, yScreen;
	Rect r;
	
	xScreen = F2S(var(x)) / xRadarDiv;
	yScreen = F2S(var(zFromPlayer)) / yRadarDiv;
	
	if (!var(hilite) && !var(isExpl)) 
		{
		ZSetPixel(xScreen, yScreen, true);
		}
	else if (var(hilite)) 
		{
		ZSETRECT(r, xScreen, yScreen, xScreen + 2, yScreen + 2);
		zCurPat = zWhite;
		ZFillRect(&r);
		}
}

defineMethod (Collided) (
	Vis *self,
	Vis *hitObj )
{
}

defineMethod (CheckBounds) (
	Vis *self )
{
	// At the very least we only have to check two of these. 
	// If an object is touching the left wall, it certainly won't
	// be touching the right wall too.
        
	if (var(x) + classVar(stdExtent).right > tunnelRect.right) 
		{
		var(x) = tunnelRect.right - classVar(stdExtent).right;
		var(xVel) = -var(xVel);
		}
	else
		if ((var(x) + classVar(stdExtent).left) < tunnelRect.left) 
			{
			var(x) = tunnelRect.left - classVar(stdExtent).left;
			var(xVel) = -var(xVel);
			}
		if ((var(y) + classVar(stdExtent).bottom) > tunnelRect.bottom) 
			{
			var(y) = tunnelRect.bottom - classVar(stdExtent).bottom;
			var(yVel) = -var(yVel);
			}
		else
			if ((var(y) + classVar(stdExtent).top) < tunnelRect.top) 
				{
				var(y) = tunnelRect.top - classVar(stdExtent).top;
				var(yVel) = -var(yVel);
				}
}

defineMethod (AdjustSize) (
	Vis *self )
{
	// Call this method after the object has been moved.

	var(collideExtent) = var(extent);
	var(collideThick) = var(thick);
	
	if (var(xVel) < 0)
		var(collideExtent).right += var(xVel);
	else if (var(xVel) > 0)
		var(collideExtent).left -= var(xVel);
	if (var(yVel) < 0)
		var(collideExtent).bottom += var(yVel);
	else if (var(yVel) > 0)
		var(collideExtent).top -= var(yVel);

	// Note that the handling of z-movement is not accurate

	if (var(zVel) < 0)
		var(collideThick) -= var(zVel);
	else if (var(zVel) > 0)
		var(collideThick) += var(zVel);
		
}

defineClassObject;
