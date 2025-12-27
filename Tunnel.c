/*
 * ZGraf - Tunnel.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include "Inheritance.h"

#include "Anim.h"
#include "Display.h"
#include "Cross.h"
#include "Invis.h"
#include "Target.h"
#include "Player.h"
#include "Tunnel.h"

#define CLASS		Tunnel
#define SUPERCLASS	Root

#define methodDefinitions													\
			definesMethod (ITunnel);										\
			definesMethod (AddObject);										\
			definesMethod (MoveObject);										\
			definesMethod (RemoveObject);									\
			definesMethod (CheckCollisions);								\
			definesMethod (DeepCollide);									\
			definesMethod (ProcessObjects);									\
			definesMethod (DrawObjects);									\
			definesMethod (HiliteNext);										\
			definesMethod (ClearAll);										\
			definesMethod (Distance);										\


Fixed Ueff;
Fixed Veff;

void ITunnelClass (
	TunnelClass *classPtr )
{
	IRootClass((RootClass *) classPtr);

	Ueff = X2Frac(0.960434);
	Veff = X2Frac(0.397825);
}

defineMethod (ITunnel) (
	register Tunnel *self,
	Box *tunBox, 
	Fixed rectGap,
	Fixed maxVisDist, 
	Fixed minVisDist,
	Fixed eyeToTarget )
{
	var(tunBox) = *tunBox;
	var(rectGap) = rectGap;
	var(farRectIncr) = 0L;
	var(maxVisDist) = maxVisDist;
	var(minVisDist) = minVisDist;
	var(eyeToTarget) = eyeToTarget;
	SetFRect(&tunnelRect, var(tunBox).left, var(tunBox).top,
		var(tunBox).right, var(tunBox).bottom);
		
	if (maxVisDist >= var(tunBox).back)
		maxVisDist = var(tunBox).back - 1;
	
	var(halfTunnel) = (Vis *) newInstance(Invis);
	(message((Invis *) var(halfTunnel), IInvis),
		0L, 0L, var(tunBox).back);

	var(player) = (Vis *) newInstance(Player);
	(message((Player *) var(player), IPlayer), 0L, 0L, 0L);

	var(target) = (Vis *) newInstance(Target);
	(message((Target *) var(target), ITarget), 0L, 0L,
		var(eyeToTarget));

	/* Precalculate some values used when rendering */

	objVar(theDisplay, halfTargetOffset) =
		halfEyeOffset * eyeToScreen / F2S(objVar(theTunnel, eyeToTarget));

	xRadarDiv = F2S(var(tunBox).right) / (kRadarWidth / 2);
	yRadarDiv = -(F2S(var(tunBox).back) / (kRadarHeight / 2));

	/*
		Set up the tunnel object list by establishing the player
		and the half-tunnel marker (these are the minimum required).
	
		The object list could be maintained with a list object,
		but it would be considerably less efficient (many more
		method calls). Instead, the list is maintained through
		member variables which are accessed directly.
	*/
	
	objVar(var(player), prev) = var(halfTunnel);
	objVar(var(player), next) = var(halfTunnel);
	objVar(var(player), zToNext) = var(tunBox.back);
	
	objVar(var(halfTunnel), prev) = var(player);
	objVar(var(halfTunnel), next) = var(player);
	objVar(var(halfTunnel), zToNext) = var(tunBox.back);

	/* Add the target */
	
	(method(AddObject), var(target));

	/* Temporary: for test play */
	
	var(hilited) = var(player);
	
}

defineMethod (AddObject) (
	register Tunnel *self,
	register Vis *obj )
{
	register Vis *hold;		/* This object has been passed */
	register Vis *look;		/* This object may need to be passed */
	Fixed zTravelled;	/* Distance moved, up to and including hold */
	Fixed zToTravel;	/* Total distance required to move from player */
	Fixed zOld;			/* Holder for distance between hold and look */

	(message(obj, AdjustSize));
	zTravelled = 0L;	
	zToTravel = objVar(obj, zFromPlayer);
	hold = var(player);
	if (zToTravel >= 0L) {		/* Object is in front of player */
		while (zToTravel > zTravelled + objVar(hold, zToNext)) {
			zTravelled += objVar(hold, zToNext);
			hold = objVar(hold, next);
			}
			
			/*	Insert obj between hold and objVar(hold, next) */
			
			zOld = objVar(hold, zToNext);
			look = objVar(hold, next);
			
			objVar(obj, prev) = hold;
			objVar(hold, next) = obj;
			objVar(obj, next) = look;
			objVar(look, prev) = obj;

			objVar(hold, zToNext) = zToTravel - zTravelled;
			objVar(obj, zToNext) = zOld - objVar(hold, zToNext);
		}

	else {						/* Object is in back of player */
		look = objVar(var(player), prev);
		while (zToTravel < zTravelled - objVar(look, zToNext)) {
			zTravelled -= objVar(look, zToNext);
			hold = look;
			look = objVar(look, prev);
			}
			
			zOld = objVar(look, zToNext);
			
			objVar(look, next) = obj;
			objVar(obj, prev) = look;
			objVar(obj, next) = hold;
			objVar(hold, prev) = obj;
			
			objVar(obj, zToNext) = zTravelled - zToTravel;
			objVar(look, zToNext) = zOld - objVar(obj, zToNext);
		}
}

defineMethod (MoveObject) (
	register Tunnel *self,
	register Vis *obj )
{
	Vis *hold, *look;
	Vis *oldNext, *oldPrev;
	register Fixed zTravelled;
	Fixed dz, zOld;

	/*
		This method should be called whenever an object�s 
		z coordinate is changed.
		Objects should not be moved to invalid locations.

		Checks to see if obj needs to move within the circular list,
		then moves it, as approprate. Also adjusts object-to-object
		spacing.
	*/
	
	
	dz = objVar(obj, zVel);
	(message(obj, AdjustSize));		// For collision detection
	if (!dz)
		return;
	
	if (dz > 0) {	/* Moving forward */
		if (dz > objVar(obj, zToNext)) {
			
			/* Disconnect obj from its old position */
			
			oldNext = objVar(obj, next);
			oldPrev = objVar(obj, prev);
			objVar(oldNext, prev) = oldPrev;
			objVar(oldPrev, next) = oldNext;
			objVar(oldPrev, zToNext) += objVar(obj, zToNext);

			/* Put obj in where it belongs */
	
			zTravelled = objVar(obj, zToNext);
			hold = objVar(obj, next);
			while (dz > zTravelled + objVar(hold, zToNext)) {
				zTravelled += objVar(hold, zToNext);
				hold = objVar(hold, next);
				}

			/*	Insert obj between hold and objVar(hold, next) */
			
			zOld = objVar(hold, zToNext);
			look = objVar(hold, next);
			
			objVar(obj, prev) = hold;
			objVar(hold, next) = obj;
			objVar(obj, next) = look;
			objVar(look, prev) = obj;

			objVar(hold, zToNext) = dz - zTravelled;
			objVar(obj, zToNext) = zOld - objVar(hold, zToNext);
			}
		else {

			/* Just change zToNext values */

			objVar(objVar(obj, prev), zToNext) += dz;
			objVar(obj, zToNext) -= dz;
			}
		}
	else {	/* Moving backwards */
	
		if (dz < -objVar(objVar(obj, prev), zToNext)) {

			/* Disconnect the object from its old position. */
	
			oldNext = objVar(obj, next);
			oldPrev = objVar(obj, prev);
			objVar(oldNext, prev) = oldPrev;
			objVar(oldPrev, next) = oldNext;
			zTravelled = -objVar(oldPrev, zToNext);
			objVar(oldPrev, zToNext) += objVar(obj, zToNext);
			
			/* Put obj in where it belongs */
	
			hold = oldPrev;
			look = objVar(hold, prev);
			while (dz < zTravelled - objVar(look, zToNext)) {
				zTravelled -= objVar(look, zToNext);
				hold = look;
				look = objVar(look, prev);
				}

			/*	Insert obj between hold and look */
			
			zOld = objVar(look, zToNext);
			
			objVar(look, next) = obj;
			objVar(obj, prev) = look;
			objVar(obj, next) = hold;
			objVar(hold, prev) = obj;
			
			objVar(obj, zToNext) = zTravelled - dz;
			objVar(look, zToNext) = zOld - objVar(obj, zToNext);
			}
		else {

			/* Just change zToNext values */

			objVar(objVar(obj, prev), zToNext) += dz;
			objVar(obj, zToNext) -= dz;
			}
		}
}

defineMethod (RemoveObject) (
	Tunnel *self,
	Vis *obj )
{
	Vis *objPrev, *objNext;
	
	/* Removes object from the list AND frees it. */
	/* Should these functions be separated? */
		
	objPrev = objVar(obj, prev);
	objNext = objVar(obj, next);
	objVar(objPrev, next) = objNext;
	objVar(objNext, prev) = objPrev;
	objVar(objPrev, zToNext) += objVar(obj, zToNext);
	
	(message(obj, Free));
}

defineMethod (CheckCollisions) (
	Tunnel *self )
{
	Fixed zTestBack, zLookFront;
	register Vis *test;
	register Vis *look;
	Vis *lastTest;

	/*
		When two objects overlap, each is notified that the
		other has collided with it (through the Collided method).
		If either object�s checkCollision variable is false,
		no check is performed.
	*/

	/*  
		Now they both check to make sure they really collided.
		If both agree then they collide, otherwise, they don't
	*/

	test = objVar(var(halfTunnel), next);
	while (test != var(halfTunnel)) {
		if (objVar(test, checkCollide)) {
			zTestBack = objVar(test, collideThick);
			look = objVar(test, next);
			zLookFront = objVar(test, zToNext);
				while ((look != test) && (zLookFront <= zTestBack)) {
					if (objVar(look, checkCollide)) {
						if (!(objVar(look, collideExtent.left) + objVar(look, x) >
						  objVar(test, collideExtent.right) + objVar(test, x)) &&
						!(objVar(look, collideExtent.right) + objVar(look, x) <
						  objVar(test, collideExtent.left) + objVar(test, x)) &&
						!(objVar(look, collideExtent.bottom) + objVar(look, y) <
						  objVar(test, collideExtent.top) + objVar(test, y)) &&
						!(objVar(look, collideExtent.top) + objVar(look, y) >
						  objVar(test, collideExtent.bottom) + objVar(test, y))) {
							if ((method(DeepCollide), look, test))
								{
								(message(test, Collided), look);
								(message(look, Collided), test);
								}
							}
						}
					zLookFront += objVar(look, zToNext);
					look = objVar(look, next);
					}
				}
		test = objVar(test, next);
		} 
}

defineMethod (DeepCollide) (
	Tunnel *self,
	Vis *obj1,
	Vis *obj2 )
{
	long result = false;
	register short i = objClassVar(obj1, stdRectCount);
	register short j = objClassVar(obj2, stdRectCount);
	FShapeRect o1Rect;
	FShapeRect o2Rect;
	if (objVar(obj1, isForm) || objVar(obj2, isForm)) return;
	do
		{
		if (i > 0)
			o1Rect = objClassVar(obj1, stdFRects)[i-1];
		else
			o1Rect.rFRect = objClassVar(obj1, stdExtent);
		do
			{
			if (j > 0)
				o2Rect = objClassVar(obj2, stdFRects)[j-1];
			else
				o2Rect.rFRect = objClassVar(obj2, stdExtent);
				
			if (!(o1Rect.rFRect.left + objVar(obj1, x) > o2Rect.rFRect.right + objVar(obj2, x)) &&
			    !(o1Rect.rFRect.right + objVar(obj1, x) < o2Rect.rFRect.left + objVar(obj2, x)) &&
			    !(o1Rect.rFRect.bottom + objVar(obj1, y) < o2Rect.rFRect.top + objVar(obj2, y)) &&
				!(o1Rect.rFRect.top + objVar(obj1, y) > o2Rect.rFRect.bottom + objVar(obj2, y))) 
			  	{
			  	result = true;
				}
			j--;
			} while (( j > 0) && !result);
		i--;
		} while ((i > 0) && !result);
		
	return(result);
}

defineMethod (ProcessObjects) (
	Tunnel *self )
{
	Vis *look, *next;
	
	look = var(player);
	do {
		next = objVar(look, next);
		if (!objVar(look, isProcessed)) {
			(message(look, Process));
			objVar(look, isProcessed) = true;
			}
		look = next;
		} while (look != var(player));

	look = objVar(var(player), next);
	do {
		next = objVar(look, next);
		if (objVar(look, pleaseRemove))
			(method(RemoveObject), look);
		else
			objVar(look, isProcessed) = false;
		look = next;
		} while (look != var(player));
}

defineMethod (DrawObjects) (
	register Tunnel *self )
{
	register Vis *look;
	Fixed zLook;
	short xEyeHold, yEyeHold, zEyeHold;
	short zFirstRect;
	Fixed zNextRect;

	if (gConfig.req32BitSwitch && objVar(theDisplay, useFlipping)) asm {
		moveq	#1, d0
		_SwapMMUMode
		}

	/*
		Draws from farthest to closest. All objects
		are drawn on the radar, but objects are drawn on the main
		view only between maxVisDist and minVisDist (closest).
	*/

	look = objVar(var(halfTunnel), prev);
	zLook = var(tunBox).back - objVar(look, zToNext);
	
	zCurBitMap = objVar(theDisplay, renderRadarBM);
	while (zLook >= var(maxVisDist)) {
		objVar(look, zFromPlayer) = zLook;
		(message(look, DrawRadar));
		look = objVar(look, prev);
		zLook -= objVar(look, zToNext);
		}

	zNextRect = var(maxVisDist) - var(farRectIncr);

	while (zLook > var(minVisDist)) {
		zCurBitMap = objVar(theDisplay, renderViewBM);
		objVar(look, zFromPlayer) = zLook;
		zEyePos = 0;
		while (zNextRect >= zLook) {
			DTunnelRect(zNextRect);
			zNextRect -= var(rectGap);
			}

		/* Object falls within visible range; prepare to draw it. */

		asm {
			
			/* Preserve eye coordinates */
		
			move.w	xEyePos, xEyeHold
			move.w	yEyePos, yEyeHold
			
			
			/*
				Adjust eye position so that drawing at (0, 0, 0)
				occurs at (look->x, look->y, look->z).
				
				Extract only integer part of Fixed coordinates.
			*/
			
			move.l	look, a0
			move.w	zLook, zEyePos
			neg.w	zEyePos
			move.w	OFFSET(Vis, x) (a0), d0
			sub.w	d0, xEyePos
			move.w	OFFSET(Vis, y) (a0), d0
			sub.w	d0, yEyePos
			
			/* (message(look, Draw)); */
			
			move.l	a0, -(sp)
			move.l	(a0), a0
			move.l	OFFSET(VisClass, Draw) (a0), a0
			jsr		(a0)
			clr.l	(sp)+
			
			/* Restore old eye position */
			
			move.w	xEyeHold, xEyePos
			move.w	yEyeHold, yEyePos
			}
		
		zCurBitMap = objVar(theDisplay, renderRadarBM);
		(message(look, DrawRadar));
		look = objVar(look, prev);
		zLook -= objVar(look, zToNext);
		}


	zCurBitMap = objVar(theDisplay, renderViewBM);
	zEyePos = 0;
	while (zNextRect > var(minVisDist)) {
		DTunnelRect(zNextRect);
		zNextRect -= var(rectGap);
		}

	zCurBitMap = objVar(theDisplay, renderRadarBM);
	while (look != (Vis *) var(halfTunnel)) {
		objVar(look, zFromPlayer) = zLook;
		(message(look, DrawRadar));
		look = objVar(look, prev);
		zLook -= objVar(look, zToNext);
		}
		
	if (gConfig.req32BitSwitch && objVar(theDisplay, useFlipping)) asm {
		clr.l	d0
		_SwapMMUMode
		}
}

defineMethod (HiliteNext) (
	Tunnel *self,
	short forward )
{

	/*
		This mechanism exists for debugging purposes.
	*/

#ifdef HILITE
	if (forward) {
		if (var(hilited) != var(player)) {
			(message(var(hilited), Hilite), false);
			var(hilited) = objVar(var(hilited), next);
			(message(var(hilited), Hilite), true);
			}
		}
	else {
		if (var(hilited) != objVar(var(first), next)) {
			(message(var(hilited), Hilite), false);
			var(hilited) = objVar(var(hilited), prev);
			(message(var(hilited), Hilite), true);
			}
		}
#endif
}

defineMethod (ClearAll) (
	Tunnel *self )
{
	Vis *look, *lookNext;
	
	// Removes everything but the player, target, and half-tunnel marker.
	
	look = objVar(var(player), next);
	while (look != var(player)) {
		lookNext = objVar(look, next);
		if (look != var(halfTunnel) && look != var(target))
			(method(RemoveObject), look);
		look = lookNext;
		}
}

defineMethod (Distance) (
	Tunnel *self,
	Vis *obj1,
	Vis *obj2)
{
	Fixed U, V;
	long result = 0;
	
	U = abs(objVar(obj1, x) - objVar(obj2, x));
	V = abs(objVar(obj1, y) - objVar(obj2, y));
	if (V > U)
		{
		Fixed temp = V;
		V = U;
		U = temp;
		}
	result = F2S(FixMul(Ueff,U) + FixMul(Veff,V));
	return (result);
}

defineClassObject;

