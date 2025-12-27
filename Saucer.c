/*
 * ZGraf - Saucer.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include "Utils.h"

#include "Inheritance.h"

#include "Anim.h"
#include "Game.h"
#include "Saucer.h"
#include "SShot.h"
#include "PShot.h"

#define CLASS		Saucer
#define SUPERCLASS	Vis

#define methodDefinitions													\
			definesMethod (ISaucer);										\
			definesMethod (Process);										\
			definesMethod (Collided);										\
			definesMethod (Free);											\

void ISaucerClass (
	SaucerClass *classObj )
{
	IVisClass((VisClass *) classObj, 132);

	InitForm(&classObj->stdForm, &classObj->stdExtent,
		classObj->stdFormIncr, classObj->stdFormSteps);

	classObj->fShootDistance = S2F(8000);
	classObj->fShotWait = 100;
}

defineMethod (ISaucer) (
	Saucer *self,
	Fixed x, Fixed y,
	Fixed zFromPlayer,
	Fixed xVel, Fixed yVel, Fixed zVel )
{
	var(fInstanceFRects) = nil;

	var(fShotTimer) = 0;
	var(fShotCount) = 0;
	(method(IVis), x, y, zFromPlayer,
		&classVar(stdExtent), classVar(stdThick), (short) true,
		xVel, yVel, zVel);
	FailNIL(var(fInstanceFRects) = (FShapeRect *) ZNewPtr(sizeof(FShapeRect) * classVar(stdRectCount)));

	BlockMove(classVar(stdFRects),var(fInstanceFRects),
		sizeof(FShapeRect) * classVar(stdRectCount));

}

defineMethod (Process) (
	Saucer *self )
{
	short xDist;
	short yDist;
	short xDir;
	short yDir;
	register short i;
	Vis *obj = var(next);
	if (objVar(obj, theClass) != (void *) gPShotClass) 	
		{	
		obj = var(prev);
		if (objVar(obj, theClass) != (void *) gPShotClass) 		
			obj = NULL;
		}
	
	if (var(fShotTimer))
		var(fShotTimer)--;
	if (var(fShotTimer) <= 0)
		{
		if (var(zFromPlayer) > -classVar(fShootDistance) && 
			var(zFromPlayer) < classVar(fShootDistance) && SRand(-100, 100) > 95)
			{
			xDist = F2S(objVar(thePlayer,x)) - F2S(var(x));
			yDist = F2S(objVar(thePlayer,y)) - F2S(var(y));
			
			if (xDist < 5000 && xDist > -5000 && yDist <= 5000 && yDist > -5000)
				{
				if (yDist < -4000) 
					yDir = -2;
				else if (yDist < -2000)
					yDir = -1;
				else if (yDist > 2000)
					yDir = 1;
				else if (yDist > 4000)
					yDir = 2;
				else yDir = 0;
				
				if (xDist < -4000) 
					xDir = -2;
				else if (xDist < -2000)
					xDir = -1;
				else if (xDist > 2000)
					xDir = 1;
				else if (xDist > 4000)
					xDir = 2;
				else xDir = 0;
				AddSShot(var(x),var(y),var(zFromPlayer), S2F(xDir), S2F(yDir), var(zVel));
				var(fShotTimer) = 10;
				var(fShotCount)++;
				if (var(fShotCount) == 5)
					{
					var(fShotTimer) = classVar(fShotWait);
					var(fShotCount) = 0;
					}
				}
			}
		}
	if (var(fColorTimer))
		var(fColorTimer)--;
	if (!var(fColorTimer))
		{
		for (i = 0; i < 10; i++)
			{
			var(fInstanceFRects)[i].rShade++;
			if (var(fInstanceFRects)[i].rShade > 10)
				var(fInstanceFRects)[i].rShade = 1;
			}
		var(fColorTimer) = 10;
		}
	
	if (obj && 
	    (abs(objVar(obj,zFromPlayer) -var(zFromPlayer) < f500)) && 
	    (message(theTunnel, Distance), self, obj) < 1000)
		{
		Fixed tVal = FRand(f100,f200);
		if (SRand(0,100) < 50)
			tVal = -tVal;
		var(xVel) = FixMul(tVal, gConfig.speedFactor);
		tVal = FRand(f100,f200);
		if (SRand(0,100) < 50)
			tVal = -tVal;		
		var(yVel) = FixMul(tVal, gConfig.speedFactor);
		}
	
	if (SRand(0,1000) > 500)
		{
		if (objVar(thePlayer,x) < var(x))
			var(xVel) -= FixMul(f1, gConfig.speedFactor);
		if (objVar(thePlayer,x) > var(x))
			var(xVel) += FixMul(f1, gConfig.speedFactor);
		if (objVar(thePlayer,y) < var(y))
			var(yVel) -= FixMul(f1, gConfig.speedFactor);
		if (objVar(thePlayer,y) > var(y))
			var(yVel) += FixMul(f1, gConfig.speedFactor);
		}
	(inherited(Process));
}

defineMethod (Collided) (
	Saucer *self,
	Vis *obj )
{
	if (objVar(obj, theClass) == (void *) gPShotClass) 
		{		
		var(checkCollide) = false;
		var(isExpl) = true;
		InitExpl(&var(expl), &var(extent), 
			objVar(obj, x) - var(x), objVar(obj, y) - var(y),
			classVar(stdExplIncr), classVar(stdExplSteps));
		(message(theGame, HitOne), self);
		(message(theGame, DeltaScore), 400);
		}
}

defineMethod (Free) ( Saucer *self )
{
	ForgetPtr(var(fInstanceFRects));
	
	(inherited(Free));
}

defineClassObject;

void AddSaucer (
	short x, short y, short z,
	short xVel, short yVel, short zVel )
{
	Saucer *saucer;

	if (SRand(0,100) > 50) zVel = -zVel;
	saucer = newInstance(Saucer);
	(message(saucer, ISaucer), S2F(x), S2F(y), S2F(z),
		FixMul(S2F(xVel), gConfig.speedFactor),
		FixMul(S2F(yVel), gConfig.speedFactor),
		FixMul(S2F(zVel), gConfig.speedFactor));
	(message(theTunnel, AddObject), (Vis *) saucer);
	(message(theGame, AddedOne), saucer);
}

