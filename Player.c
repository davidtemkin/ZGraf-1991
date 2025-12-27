/*
 * ZGraf - Player.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include <math.h>
#include <FixMath.h>

#include "SoundKit.h"
#include "SoundList.h"
#include "Utils.h"

#include "Inheritance.h"

#include "Anim.h"
#include "Tunnel.h"
#include "PShot.h"
#include "EDS.h"
#include "Aphid.h"
#include "Thing.h"
#include "Cross.h"
#include "Saucer.h"
#include "SShot.h"
#include "Player.h"

#define CLASS		Player
#define SUPERCLASS	Vis

#define methodDefinitions													\
			definesMethod (IPlayer);										\
			definesMethod (DrawRadar);										\
			definesMethod (GetInput);										\
			definesMethod (GetDemoInput);									\
			definesMethod (Collided);										\
			definesMethod (MoveAttachedObjects);							\
			definesMethod (Process);										\

extern Point MTemp : 0x828;
extern Point RawMouse : 0x82C;
extern unsigned char CrsrNew : 0x8CE;
extern unsigned char CrsrCouple : 0x8CF;

void IPlayerClass (
	PlayerClass *classObj )
{
	IVisClass((VisClass *) classObj, 0);
	
	// Class variables
	
	SetFRect(&classObj->stdExtent,
		S2F(-500), S2F(-500), S2F(500), S2F(500));
	classObj->stdThick = S2F(100);

	// Scalable class variables

	classObj->accelStep = FixMul(X2F(3.0), gConfig.speedFactorSquared);
	classObj->slowFactor = X2F(pow(0.95, F2X(gConfig.speedFactor)));
	classObj->speedLimit = FixMul(S2F(500), gConfig.speedFactor);
	classObj->demoCruisingSpeed = FixMul(S2F(10), gConfig.speedFactor);
	
	// Prevents odd stroboscopic effects; also prevents strictly accurate
	// scaling of the player�s maximum speed. This means that players on
	// slower machines simply move slower -- they have a lower maximum speed.
	
	if (classObj->speedLimit > S2F(900))	// 1000 is half of objVar(theTunnel, rectGap)
		classObj->speedLimit = S2F(900);
		
	classObj->hvSpeedLimit = FixMul(classObj->speedLimit, S2F(2));
	classObj->hvAccelStep = FixMul(classObj->accelStep, S2F(2));
	classObj->hvSlowFactor =  X2F(pow(0.5, F2X(gConfig.speedFactor)));
}

defineMethod (IPlayer) (
	Player *self,
	Fixed x, Fixed y, Fixed z )
{
	(method(IVis), x, y, z,
		&classVar(stdExtent), classVar(stdThick), (short) true, 0L, 0L, 0L);

	var(quitFlag) = var(pauseFlag) = false;
	
	/* Prepare for input */

	FlushEvents(everyEvent, 0);
	var(eventMask) = mDownMask | keyDownMask;
	SetEventMask(var(eventMask));
	
	// FIX: When non-main screen is used, this will break
	
	var(xScreenCtr) = zPage1BitMap->xBound / 2;
	var(yScreenCtr) = zPage1BitMap->yBound / 2;
	if (fakeColorDisp)
		var(xScreenCtr) = zPage1BitMap->xBound;
	else
		var(xScreenCtr) = zPage1BitMap->xBound / 2;
		
	var(xMouseFactor) = F2S(tunnelRect.right) / var(xScreenCtr);
	var(yMouseFactor) = F2S(tunnelRect.bottom) / var(yScreenCtr);
	var(hScale) = (F2S(tunnelRect.right)  / 2 ) / 4;
	var(vScale) = (F2S(tunnelRect.bottom) / 2 ) / 4;
		
	/* Put player position in appropriate globals. */
	
	xEyePos = F2S(x);
	yEyePos = F2S(y);
	zEyePos = F2S(z);
	
	var(wasHit) = false;

	{	
		short xNew, yNew;
		
		xNew = var(xScreenCtr);
		yNew = var(yScreenCtr);

		 asm {
			move.w	yNew, d0
			swap    d0
			move.w	xNew, d0
			move.l  d0, MTemp
			move.l  d0, RawMouse
			move.b  CrsrCouple, CrsrNew
	    	}
	    }
	}

defineMethod (DrawRadar) (
	Player *self )
{
	short xScreen, yScreen;

	xScreen = F2S(var(x)) / xRadarDiv;
	
	/*
		Always drawn vertically centered within radar space
	*/

	SHADE(zWhite);
	ZSetPixel(xScreen, 0, true);
	ZHLine(xScreen-1, xScreen+1, 1);
	ZHLine(xScreen-2, xScreen+2, 2);
}

defineMethod (GetInput) (
	Player *self )
{
	EventRecord theEvent;
	KeyMap theKeys;
	Boolean	isEvent;
	Boolean mouseOffScreen;
	char c;
	PShot *shot;
	short xMouseAbs, yMouseAbs;
	Boolean isShot, moveForward, moveBackward, makeThings, makeEDS;
	short xDir, yDir;

	isShot = moveForward = moveBackward = makeThings = makeEDS = false;

	// Read all events which accumulated since last frame was drawn.
	// GetNextEvent should be allowed as an option.
	
	while (isEvent = GetOSEvent(var(eventMask), &theEvent)) {
			if (theEvent.what == keyDown) {
				switch (theEvent.message & charCodeMask) {
					case 'q':
					case 'Q':
						var(quitFlag) = true;
						break;
					case ' ':
						makeThings = true;
						break;
					case 'e':
						makeEDS = true;
						break;
					case 'c':
						(message(theTunnel, ClearAll));
						break;
					case 'r':
					case 'R':
						objVar(theGame, startNewGame) = true;
					}
				}
			else if (theEvent.what == mouseDown)
				isShot = true;
		}

	GetKeys(theKeys);

	// FIX: This needs to be made more general.

	if ((theKeys[0] & 0x00000800) || (theKeys[2] & 0x00001000))
		moveForward = true;				// '2' on keyboard or numeric pad
	if ((theKeys[0] & 0x00000400) || (theKeys[2] & 0x00000800))
		moveBackward = true;			// '1' on keyboard or numeric pad

	if (moveForward && moveBackward)
		moveForward = moveBackward = false;

	// Make sure mouse isn�t off main monitor, if there are
	// more than one.

	xMouseAbs = theEvent.where.h;
	yMouseAbs = theEvent.where.v;

	mouseOffScreen = false;
	if (xMouseAbs < gConfig.absPlayRect.left) {
		xMouseAbs = gConfig.absPlayRect.left;
		mouseOffScreen = true;
		}
	else if (xMouseAbs >= gConfig.absPlayRect.right) {
		xMouseAbs = gConfig.absPlayRect.right - 1;
		mouseOffScreen = true;
		}
	if (yMouseAbs < gConfig.absPlayRect.top) {
		yMouseAbs = gConfig.absPlayRect.top;
		mouseOffScreen = true;
		}
	else if (yMouseAbs >= gConfig.absPlayRect.bottom) {
		yMouseAbs = gConfig.absPlayRect.bottom - 1;
		mouseOffScreen = true;
		}

	// Move the cursor back onto the main screen if necessary

	if (mouseOffScreen) {
		short xNew, yNew;
		
		xNew = xMouseAbs;
		yNew = yMouseAbs;

		 asm {
			move.w	yNew, d0
			swap    d0
			move.w	xNew, d0
			move.l  d0, MTemp
			move.l  d0, RawMouse
			move.b  CrsrCouple, CrsrNew
	    	}
	    }

	/*
		The eyePos globals are shorts. The instance variables x, y, and z
		are of type Fixed -- in other words, the high-order 16 bits of
		these variables are equivalent to the eyePos variables.

		Also set xVel and yVel for AdjustSize method: FIX

	*/
#if 1
	xEyePos = var(xMouseFactor) * (xMouseAbs - var(xScreenCtr));
	yEyePos = var(yMouseFactor) * (yMouseAbs - var(yScreenCtr));	
	var(x) = S2F(xEyePos);
	var(y) = S2F(yEyePos);
#else
	xDir = var(xMouseFactor) * (xMouseAbs - var(xScreenCtr)) / var(hScale);
	yDir = var(yMouseFactor) * (yMouseAbs - var(yScreenCtr)) / var(vScale);

	if (xDir == 0)
		{
		if (var(xVel) != 0)
			{
			var(xVel) = FixMul(var(xVel), classVar(hvSlowFactor));
			if (var(xVel) > -classVar(hvAccelStep) && var(xVel) < classVar(hvAccelStep))
				var(xVel) == 0;
			}
		}

	if (yDir == 0)
		{
		if (var(yVel) != 0)
			{
			var(yVel) = FixMul(var(yVel), classVar(hvSlowFactor));
			if (var(yVel) > -classVar(hvAccelStep) && var(yVel) < classVar(hvAccelStep))
				var(yVel) == 0;
			}
		}

	var(xVel) += FixMul(S2F(xDir), classVar(hvAccelStep));
	var(yVel) += FixMul(S2F(yDir), classVar(hvAccelStep));

	if (var(xVel) > classVar(hvSpeedLimit))
		var(xVel) = classVar(hvSpeedLimit);
	if (var(xVel) < -classVar(hvSpeedLimit))
		var(xVel) = -classVar(hvSpeedLimit);
	if (var(yVel) > classVar(hvSpeedLimit))
		var(yVel) = classVar(hvSpeedLimit);
	if (var(yVel) < -classVar(hvSpeedLimit))
		var(yVel) = -classVar(hvSpeedLimit);
		
	var(x) += var(xVel);
	var(y) += var(yVel);

	if (var(x) + classVar(stdExtent).right > tunnelRect.right) 
		{
		var(xVel) = -var(xVel);
		var(x) += var(xVel);
		var(xVel) = 0L;
		}
	else
	if (var(x) + classVar(stdExtent).left < tunnelRect.left) 
		{
		var(xVel) = -var(xVel);
		var(x) += var(xVel);
		var(xVel) = 0L;
		}
	if (var(y) + classVar(stdExtent).bottom > tunnelRect.bottom) 
		{
		var(yVel) = -var(yVel);
		var(y) += var(yVel);
		var(yVel) = 0L;
		}
	else
	if (var(y) + classVar(stdExtent).top < tunnelRect.top) 
		{
		var(yVel) = -var(yVel);
		var(y) += var(yVel);
		var(yVel) = 0L;
		}
		
	xEyePos = F2S(var(x));
	yEyePos = F2S(var(y));
		
#endif

	

	if (moveBackward) {
		if (var(zVel) > 0)
			var(zVel) = FixMul(var(zVel), classVar(slowFactor));
		var(zVel) -= classVar(accelStep);
		}
	else if (moveForward) {
		if (var(zVel) < 0)
			var(zVel) = FixMul(var(zVel), classVar(slowFactor));
		var(zVel) += classVar(accelStep);
		}
	else 
		var(zVel) = FixMul(var(zVel), classVar(slowFactor));
	

	if (var(zVel) > classVar(speedLimit))
		var(zVel) = classVar(speedLimit);
	if (var(zVel) < -classVar(speedLimit))
		var(zVel) = -classVar(speedLimit);

	// Move the player, target and half-tunnel objects

	(method(MoveAttachedObjects));

	// Fire shot if button�s been pressed

	 if (isShot) {
		shot = newInstance(PShot);
		(message(shot, IPShot), var(x), var(y), var(zVel));
		(message(theTunnel, AddObject), shot);
		PLAYB(kSndNum_PlayerFiredShot, kSndPriority_PlayerFiredShot);
		}

	if (makeThings) {
		short crr;
		Cross *ci;
		Box tBox;
		
		for (crr = 1; crr < 50; crr++) {
			ci = newInstance(Cross);
			tBox = objVar(theTunnel, tunBox);
			(message(ci, ICross),
				FRand(tBox.left, tBox.right),
				FRand(tBox.top,  tBox.bottom),
				FRand(tBox.front, tBox.back),
				FixMul(FRand(-f100, f100), gConfig.speedFactor),
				FixMul(FRand(-f100, f100), gConfig.speedFactor),
				FixMul(FRand(-f100, f100), gConfig.speedFactor));
			(message(theTunnel, AddObject), ci);
			(message(theGame, AddedOne), ci);
			}
		}

	if (makeEDS) {
		short err;
		EDS *ei;
		Box tBox;
		
		for (err = 1; err < 10; err++) {
			ei = newInstance(EDS);
			tBox = objVar(theTunnel, tunBox);
			(message(ei, IEDS),
				FRand(tBox.left, tBox.right),
				FRand(tBox.top,  tBox.bottom),
				FRand(tBox.front, tBox.back),
				FixMul(S2F(SRand(-20, 20)), gConfig.speedFactor),
				FixMul(S2F(SRand(-20, 20)), gConfig.speedFactor),
				FixMul(S2F(SRand(-20, 20)), gConfig.speedFactor));
			(message(theTunnel, AddObject), ei);
			(message(theGame, AddedOne), ei);
			}
		}
}

defineMethod (GetDemoInput) (
	Player *self )
{
	EventRecord theEvent;
	Boolean	isEvent;
	Boolean isMouseDown;

	isMouseDown = isEvent = false;

	// Read all events which accumulated since last frame was drawn.
	// FIX: GetNextEvent should be allowed as an option.
	
	while (isEvent = GetOSEvent(var(eventMask), &theEvent)) {
		if (theEvent.what == keyDown) {
			switch (theEvent.message & charCodeMask) {
				case 'q':
				case 'Q':
					var(quitFlag) = true;
					break;
				case 'r':
				case 'R':
					objVar(theGame, startNewGame) = true;
				}
			}	
		else if (theEvent.what == mouseDown)
			isMouseDown = true;
		}

	if (objVar(theGame, inGameOver))
		var(zVel) = FixMul(var(zVel), classVar(slowFactor));

	else {
		if (isMouseDown)
			objVar(theGame, startNewGame) = true;
		else {
			var(x) = var(y) = 0L;
			xEyePos = yEyePos = 0;
			var(xVel) = var(yVel) = 0;
			var(zVel) = classVar(demoCruisingSpeed);
			}
		}

	(method(MoveAttachedObjects));
}

defineMethod (MoveAttachedObjects) (
	Player *self )
{
	// May need to move player, target, screen, and half-tunnel marker in tunnel list:
	// Assumes that zVel is set properly and all that
	
	(message(theTunnel, MoveObject), self);
	
	objVar(objVar(theTunnel, target), x) = var(x);
	objVar(objVar(theTunnel, target), y) = var(y);
	objVar(objVar(theTunnel, target), zVel) = var(zVel);
	(message(theTunnel, MoveObject), objVar(theTunnel, target));

	objVar(objVar(theTunnel, halfTunnel), zVel) = var(zVel);
	(message(theTunnel, MoveObject), objVar(theTunnel, halfTunnel));
	
	// Adjust positioning of tunnel border rects

	if (var(zVel) > 0) {
		objVar(theTunnel, farRectIncr) += var(zVel);
		objVar(theTunnel, farRectIncr) %= objVar(theTunnel, rectGap);
		}

	else if (var(zVel) < 0) {
		Fixed incr;	
		
		incr = -var(zVel) % objVar(theTunnel, rectGap);
		objVar(theTunnel, farRectIncr) -= incr;
		if (objVar(theTunnel, farRectIncr) < 0L)
			objVar(theTunnel, farRectIncr) += objVar(theTunnel, rectGap);
		}
}

defineMethod (Process) (
	Player *self )
{
	var(x) += var(xVel);
	var(y) += var(yVel);
	xEyePos = F2S(var(x));
	yEyePos = F2S(var(y));
}

defineMethod (Collided) (
	Player *self,
	Vis *obj )
{


	if (objVar(obj, theClass) == (void *) gCrossClass || 
		objVar(obj, theClass) == (void *) gAphidClass ||
		objVar(obj, theClass) == (void *) gSaucerClass ||
		objVar(obj, theClass) == (void *) gSShotClass) 
		{
		(message(theGame, DeltaEnergy), -1);
		var(wasHit) = true;
		}
	
	// sounds
	
	if (objVar(obj, theClass) == (void *) gThingClass) 
		{
		PLAYB(kSndNum_PlayerHitThing, kSndPriority_PlayerHitThing);
		PLAYA(kSndNum_PlayerHitThing, kSndPriority_PlayerHitThing);
		}
	else if (objVar(obj, theClass) == (void *) gCrossClass)
		PLAYB(kSndNum_PlayerHitCross, kSndPriority_PlayerHitCross)
	else if (objVar(obj, theClass) == (void *) gSaucerClass)
		PLAYB(kSndNum_PlayerHitCross, kSndPriority_PlayerHitCross)
	else if (objVar(obj, theClass) == (void *) gSShotClass)
		PLAYB(kSndNum_PlayerHitCross, kSndPriority_PlayerHitCross)
	else if (objVar(obj, theClass) == (void *) gAphidClass)
		PLAYB(kSndNum_PlayerHitAphid, kSndPriority_PlayerHitAphid)
	else if (objVar(obj, theClass) == (void *) gEDSClass)
		PLAYB(kSndNum_PlayerHitEDS, kSndPriority_PlayerHitEDS)
}

defineClassObject;