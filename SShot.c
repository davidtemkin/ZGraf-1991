/*
 * ZGraf - SShot.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include "SoundKit.h"
#include "SoundList.h"

#include "Inheritance.h"

#include "Anim.h"
#include "Tunnel.h"
#include "EDS.h"
#include "Aphid.h"
#include "Thing.h"
#include "Cross.h"
#include "Saucer.h"
#include "SShot.h"

#define CLASS		SShot
#define SUPERCLASS	Vis

#define methodDefinitions													\
			definesMethod (ISShot);											\
			definesMethod (Process);										\
			definesMethod (Draw);											\
			definesMethod (Collided);										\

void ISShotClass (
	SShotClass *classObj )
{
	IVisClass((VisClass *) classObj, 133);

	classObj->endDist = S2F(9000);
	classObj->startDist = S2F(800);

	// Scalable class variables
	
	classObj->stdSpeed = FixMul(S2F(500), gConfig.speedFactor);
}

defineMethod (ISShot) (
	SShot *self,
	Fixed x, Fixed y, Fixed z, Fixed atX, Fixed atY, Fixed zVel )
{
	/* Always put it right in front of the target */
	Fixed speed = classVar(stdSpeed);
	if (zVel < 0) 
		speed = -speed;
	speed += zVel;
	(method(IVis), x , y , z,
		&classVar(stdExtent), classVar(stdThick), (short) true, 
		FixMul(atX, f100), FixMul(atY, f100),
		speed);

	var(dist) = 0;
	
}

defineMethod (Process) (
	SShot *self )
{
	var(dist) += classVar(stdSpeed);
	if (var(dist) >= classVar(endDist))
		var(pleaseRemove) = true;
	(inherited(Process));
}

defineMethod (Draw) (
	SShot *self )
{
	// Don�t draw it if it�s too close to the player
		(inherited(Draw));
}

defineMethod (Collided) (
	SShot *self,
	Vis *obj )
{
	if (objVar(obj, theClass) != (void *) gSaucerClass) 
		{
		var(pleaseRemove) = true;
		var(isExpl) = true;
		InitExpl(&var(expl), &var(extent), 
			objVar(obj, x) - var(x), objVar(obj, y) - var(y),
			classVar(stdExplIncr), classVar(stdExplSteps));
		}
}

defineClassObject;

// ----------

void AddSShot(
	Fixed x, Fixed y, Fixed z,
	Fixed atX, Fixed atY, Fixed zVel )
{
	SShot *ss;
	
	ss = newInstance(SShot);
	(message(ss, ISShot), x, y, z, atX, atY, zVel);
	(message(theTunnel, AddObject), (Vis *) ss);
}

