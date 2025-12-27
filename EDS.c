/*
 * ZGraf - EDS.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include "Inheritance.h"

#include "Anim.h"
#include "Game.h"
#include "PShot.h"
#include "Tunnel.h"

#include "EDS.h"

#define CLASS		EDS
#define SUPERCLASS	Vis

#define methodDefinitions													\
			definesMethod (IEDS);											\
			definesMethod (Draw);											\
			definesMethod (Collided);										\

void IEDSClass (
	EDSClass *classObj )
{
	IVisClass((VisClass *) classObj, (short) 0);

	// Class variables
	
	SetFRect(&classObj->e1, S2F(-600), S2F(-450), S2F(-150), S2F(50));
	SetFRect(&classObj->e2, S2F(-500), S2F(-350), S2F(-150), S2F(-250));
	SetFRect(&classObj->e3, S2F(-500), S2F(-150), S2F(-150), S2F(-50));
	
	SetFRect(&classObj->d1, S2F(-100), S2F(-450), S2F(350), S2F(50));
	SetFRect(&classObj->d2, S2F(0), S2F(-350), S2F(250), S2F(-50));
	SetFRect(&classObj->d3, S2F(250), S2F(-450), S2F(350), S2F(-350));
	SetFRect(&classObj->d4, S2F(250), S2F(-50), S2F(350), S2F(50));
	

	SetFRect(&classObj->s1, S2F(400), S2F(-450), S2F(850), S2F(50));
	SetFRect(&classObj->s2, S2F(500), S2F(-350), S2F(850), S2F(-250));
	SetFRect(&classObj->s3, S2F(400), S2F(-150), S2F(750), S2F(-50));
	
	SetFRect(&classObj->stdExtent, S2F(-1200), S2F(-1000),
		S2F(1000), S2F(1200));

	// classObj->stdExplIncr = S2F(12);
	// classObj->stdExplSteps = 250;
	
	// put this in Vis if possible
	
	InitForm(&classObj->stdForm, &classObj->stdExtent,
		classObj->stdExplIncr, classObj->stdExplSteps);

}

defineMethod (IEDS) (
	EDS *self,
	Fixed x, Fixed y,
	Fixed zFromPlayer,
	Fixed xVel, Fixed yVel, Fixed zVel )

{
	(method(IVis), x, y, zFromPlayer,
		&classVar(stdExtent), classVar(stdThick), (short) true,
		xVel, yVel, zVel);
}

defineMethod (Draw) (
	EDS *self )
{
	if (var(isExpl) || var(isForm)) {


		SHADE(zWhite);
		EDFillRect(&var(expl), &classVar(stdExtent), 0L);

		// 'E'

		SHADE(zBlack);
		EDFillRect(&var(expl), &classVar(e1), 0L);
		SHADE(zWhite);
		EDFillRect(&var(expl), &classVar(e2), 0L);
		EDFillRect(&var(expl), &classVar(e3), 0L);

		// 'D'
		
		SHADE(zBlack);
		EDFillRect(&var(expl), &classVar(d1), 0L);
		SHADE(zWhite);
		EDFillRect(&var(expl), &classVar(d2), 0L);
		EDFillRect(&var(expl), &classVar(d3), 0L);
		EDFillRect(&var(expl), &classVar(d4), 0L);
		
		// 'S'
		
		SHADE(zBlack);
		EDFillRect(&var(expl), &classVar(s1), 0L);
		SHADE(zWhite);
		EDFillRect(&var(expl), &classVar(s2), 0L);
		EDFillRect(&var(expl), &classVar(s3), 0L);

		}
	else {

		SHADE(zWhite);
		DFillRect(&classVar(stdExtent), 0L);

		// 'E'

		SHADE(zBlack);
		DFillRect(&classVar(e1), 0L);
		SHADE(zWhite);
		DFillRect(&classVar(e2), 0L);
		DFillRect(&classVar(e3), 0L);

		// 'D'
		
		SHADE(zBlack);
		DFillRect(&classVar(d1), 0L);
		SHADE(zWhite);
		DFillRect(&classVar(d2), 0L);
		DFillRect(&classVar(d3), 0L);
		DFillRect(&classVar(d4), 0L);

		// 'S'
		
		SHADE(zBlack);
		DFillRect(&classVar(s1), 0L);
		SHADE(zWhite);
		DFillRect(&classVar(s2), 0L);
		DFillRect(&classVar(s3), 0L);
	}
}

defineMethod (Collided) (
	EDS *self,
	Vis *obj )
{
	if (objVar(obj, theClass) == (void *) gPShotClass) {
		var(checkCollide) = false;
		var(isExpl) = true;
		InitExpl(&var(expl), &var(extent), 
			objVar(obj, x) - var(x), objVar(obj, y) - var(y),
			classVar(stdExplIncr), classVar(stdExplSteps));
		(message(theGame, HitOne), self);
		(message(theGame, DeltaScore), 2000);
		}
}


defineClassObject;
