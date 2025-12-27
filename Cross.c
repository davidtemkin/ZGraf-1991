/*
 * ZGraf - Cross.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include "Inheritance.h"
#include "Utils.h"

#include "Anim.h"
#include "Game.h"
#include "PShot.h"
#include "SShot.h"

#include "Cross.h"


#define CLASS		Cross
#define SUPERCLASS	Vis

#define methodDefinitions													\
			definesMethod (ICross);											\
			definesMethod (Process);										\
			definesMethod (Collided);										\

void ICrossClass (
	CrossClass *classPtr )
{
	IVisClass((VisClass *) classPtr, 128);
		
	InitForm(&classPtr->stdForm, &classPtr->stdExtent,
		classPtr->stdFormIncr, classPtr->stdFormSteps);
}

defineMethod (ICross) (
	Cross *self,
	Fixed x, Fixed y,
	Fixed zFromPlayer,
	Fixed xVel, Fixed yVel, Fixed zVel )
{
	(method(IVis), x, y, zFromPlayer,
		&classVar(stdExtent), classVar(stdThick), (short) true,
		xVel, yVel, zVel);
}

defineMethod (Process) (
	Cross *self )
{
	(inherited(Process));
}

defineMethod (Collided) (
	Cross *self,
	Vis *obj )
{
	if (objVar(obj, theClass) == (void *) gPShotClass )
		{
		var(checkCollide) = false;
		var(isExpl) = true;
		InitExpl(&var(expl), &var(extent), 
			objVar(obj, x) - var(x), objVar(obj, y) - var(y),
			classVar(stdExplIncr), classVar(stdExplSteps));
		(message(theGame, HitOne), self);
		(message(theGame, DeltaScore), 200);
		}
}

defineClassObject;

void AddCross (
	short x, short y, short z,
	short xVel, short yVel, short zVel )
{
	Cross *cr;

	cr = newInstance(Cross);
	(message(cr, ICross), S2F(x), S2F(y), S2F(z),
		FixMul(S2F(xVel), gConfig.speedFactor),
		FixMul(S2F(yVel), gConfig.speedFactor),
		FixMul(S2F(zVel), gConfig.speedFactor));
	(message(theTunnel, AddObject), (Vis *) cr);
	(message(theGame, AddedOne), cr);
}

