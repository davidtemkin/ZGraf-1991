/*
 * ZGraf - PShot.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include "Inheritance.h"

#include "Utils.h"

#include "SoundKit.h"

#include "Anim.h"
#include "Tunnel.h"
#include "EDS.h"
#include "SoundList.h"
#include "Aphid.h"
#include "Thing.h"
#include "Cross.h"
#include "Saucer.h"
#include "PShot.h"


#define CLASS		PShot
#define SUPERCLASS	Vis

#define methodDefinitions													\
			definesMethod (IPShot);											\
			definesMethod (Process);										\
			definesMethod (Draw);											\
			definesMethod (Collided);										\
			

void IPShotClass (
	PShotClass *classObj )
{
	IVisClass((VisClass *) classObj, 130);

	// Class variables

	classObj->endDist = S2F(16000);
	classObj->startDist = S2F(800);

	// Scalable class variables
	
	classObj->stdSpeed = FixMul(S2F(500), gConfig.speedFactor);
}

defineMethod (IPShot) (
	PShot *self,
	Fixed x, Fixed y, Fixed zVelPlayer )
{

	/* Always put it right in front of the target */
	(method(IVis), x , y ,
		classVar(startDist) - (classVar(stdSpeed) + zVelPlayer),
		&classVar(stdExtent), classVar(stdThick), (short) true, 0L, 0L,
		classVar(stdSpeed) + zVelPlayer);

	var(dist) = 0;
}

defineMethod (Process) (
	PShot *self )
{
	var(dist) += classVar(stdSpeed);
	if (var(dist) >= classVar(endDist))
		var(pleaseRemove) = true;
	(inherited(Process));
}

defineMethod (Draw) (
	PShot *self )
{
	// Don�t draw it if it�s too close to the player

	if (!(var(zFromPlayer) >= 0 && var(zFromPlayer) < classVar(startDist))) 
		(inherited(Draw));
}

defineMethod (Collided) (
	PShot *self,
	Vis *obj )
{
	if (obj != (Vis *) thePlayer && obj->theClass != (void *) gPShotClass)
		var(pleaseRemove) = true;

	// Sounds

	if (objVar(obj, theClass) == (void *) gThingClass)
		PLAYA(kSndNum_PShotHitThing, kSndPriority_PShotHitThing)
	else if (objVar(obj, theClass) == (void *) gCrossClass)
		PLAYA(kSndNum_PShotHitCross, kSndPriority_PShotHitCross)
	else if (objVar(obj, theClass) == (void *) gAphidClass)
		PLAYA(kSndNum_PShotHitAphid, kSndPriority_PShotHitAphid)
	else if (objVar(obj, theClass) == (void *) gEDSClass)
		PLAYA(kSndNum_PShotHitEDS, kSndPriority_PShotHitEDS)
	else if (objVar(obj, theClass) == (void *) gSaucerClass)
		PLAYA(kSndNum_PShotHitSaucer, kSndPriority_PShotHitSaucer)
}

defineClassObject;

