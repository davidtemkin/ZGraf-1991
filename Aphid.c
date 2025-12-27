/*
 * ZGraf - Aphid.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include "Utils.h"

#include "Inheritance.h"

#include "Anim.h"
#include "Game.h"
#include "PShot.h"
#include "Aphid.h"

#define CLASS		Aphid
#define SUPERCLASS	Vis

#define methodDefinitions													\
			definesMethod (IAphid);											\
			definesMethod (Process);										\
			definesMethod (DrawRadar);										\
			definesMethod (Collided);										\

void IAphidClass (
	AphidClass *classPtr )
{
	IVisClass((VisClass *) classPtr, 134);
}


defineMethod (IAphid) (
	Aphid *self,
	Fixed x, Fixed y,
	Fixed zFromPlayer )
{
	(method(IVis), x, y, zFromPlayer,
		&classVar(stdExtent), classVar(stdThick), (short) true, 
			0L,
			0L,
			FixMul(gConfig.speedFactor, FRand(-f100, f100)));

	var(angle) = SRand(0,360);
	var(newAngle) = var(angle);
	var(velocity) = f10;
	if (SRand(0,100) > 50)
		var(direction) = 1;
	else
		var(direction) = -1;
	CalcVectors(var(angle), var(velocity) * var(direction),
		&var(xVel), &var(yVel));
}

defineMethod (Process) (
	Aphid *self )
{
	Vis *obj = var(next);
	Fixed xVect, yVect;
	
	if (!obj) 	
		obj = var(prev);
#if 1
	if (obj)
		{
		Fixed oZVel = objVar(obj, zVel);
		if (oZVel > -f100 && oZVel < f100)
			if (oZVel > var(zVel))
				var(zVel) += FixMul(gConfig.speedFactor, f2);
			else
				if (oZVel < var(zVel))
					var(zVel) -= FixMul(gConfig.speedFactor, f2);

		if (objVar(obj, x) > var(x) && !(var(newAngle) < 270 && var(newAngle) > 90))
			var(newAngle)++;	
		if (objVar(obj, x) < var(x) && (var(newAngle) < 270 && var(newAngle) > 90))
			var(newAngle)--;	
		if (objVar(obj, y) < var(y) && var(newAngle) > 180)
			var(newAngle)--;	
		if (objVar(obj, y) > var(y) && var(newAngle) < 180)
			var(newAngle)++;
		
		if ((message(theTunnel, Distance), self, obj) < 400)
			var(newAngle) += 10;
		}
#endif
		
	if (0 > var(x) && !(var(newAngle) < 270 && var(newAngle) > 90))
		var(newAngle)++;	
	if (0 < var(x) && (var(newAngle) < 270 && var(newAngle) > 90))
		var(newAngle)--;	
	if (0 < var(y) && var(newAngle) > 180)
		var(newAngle)--;	
	if (0 > var(y) && var(newAngle) < 180)
		var(newAngle)++;
		
		
	if (var(newAngle) >= 360)
		var(newAngle) -= 360;
	else if (var(newAngle) < 0)
		var(newAngle) += 360;
		
	if (var(newAngle) != var(angle))
		{
		var(angle) += 2 * var(direction);
		if (var(angle) >= 360)
			var(angle) -= 360;
		else if (var(angle) < 0)
			var(angle) += 360;
		var(velocity) += f2;
		}
	else
		var(velocity) -= f1;
	
		
	if (var(velocity) > f30)
		var(velocity) = f30;
	else if (var(velocity) < -f30)
		var(velocity) = f30;
		
	CalcVectors(var(angle),var(velocity),&xVect,&yVect);
	
	var(xVel) += xVect;
	if (var(xVel) >= f100)
		var(xVel) = f100;
	else if (var(xVel) < -f100)
		var(xVel) = -f100;
		
	var(yVel) += yVect;
	if (var(yVel) >= f100)
		var(yVel) = f100;
	else if (var(yVel) < -f100)
		var(yVel) = -f100;
	
	if (var(zVel) >= f100)
		var(zVel) = f100;
	else if (var(zVel) < -f100)
		var(zVel) = -f100;

	(inherited(Process));
	
}

defineMethod (DrawRadar) (
	Aphid *self )
{

}


defineMethod (Collided) (
	Aphid *self,
	Vis *obj )
{
//	if (objVar(obj, theClass) == (void *) gPShotClass) 
		{
		var(checkCollide) = false;
		var(isExpl) = true;
		InitExpl(&var(expl), &var(extent), 
			objVar(obj, x) - var(x), objVar(obj, y) - var(y),
			f100, classVar(stdExplSteps));
		}
}

defineClassObject;

// ------------------------

void AddAphid (void)
{
	Aphid *ai;

	ai = newInstance(Aphid);

	(message(ai, IAphid),
		FRand(objVar(theTunnel, tunBox).left,
			objVar(theTunnel, tunBox).right),
		FRand(objVar(theTunnel, tunBox).top,
			objVar(theTunnel, tunBox).bottom),
		FRand(objVar(theTunnel, tunBox).front,
			objVar(theTunnel, tunBox).back));
	(message(theTunnel, AddObject), (Vis *) ai);
}
