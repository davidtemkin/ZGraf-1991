/*
 * ZGraf - Thing.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include "Inheritance.h"

#include "Anim.h"
#include "PShot.h"
#include "Thing.h"
#include "Game.h"

#define CLASS		Thing
#define SUPERCLASS	Vis

#define methodDefinitions													\
			definesMethod (IThing);											\
			definesMethod (DrawRadar);										\
			definesMethod (Collided);										\


void IThingClass (
	ThingClass *classObj )
{
	IVisClass((VisClass *) classObj, 131);

	SetFRect(&classObj->headRect, -f1000,
		-f1000, f1000, f1000);

	SetFRect(&classObj->r0, -f800, -f800, f800, f800);
	SetFRect(&classObj->r1, -f200, -f600, f200, -f400);
	SetFRect(&classObj->r2, -f200, -f200, f200, f200);
	SetFRect(&classObj->r3, -f200, f400, f200, f600);
		
	classObj->stdExtent = classObj->headRect;
	
	InitForm(&classObj->stdForm, &classObj->headRect,
		S2F(100), 50);
}

defineMethod (IThing) (
	Thing *self,
	Fixed x, Fixed y,
	Fixed zFromPlayer )
{
	(method(IVis), x, y, zFromPlayer,
		&classVar(stdExtent), classVar(stdThick), (short) true, 0L, 0L, 0L);
}

defineMethod (DrawRadar) (
	Thing *self )
{
	short xScreen, yScreen;
	Rect r;
	
	xScreen = F2S(var(x)) / xRadarDiv;
	yScreen = F2S(var(zFromPlayer)) / yRadarDiv;
	
	if (!var(isExpl)) {
		ZSETRECT(r, xScreen - 2, yScreen - 1, xScreen + 1, yScreen + 1);
		zCurPat = zWhite;
		ZFillRect(&r);
		}
}


// Doesn't actually get called at all.

defineMethod (Draw) (
	Thing *self )
{
	(inherited(Draw));

#ifdef NEWTHING
	FRect rFrameLeft, rFrameTop, rFrameRight, rFrameBottom;
	FRect rFull, rInside;
	FRect rInside1, rInside2;
	
	
	SetFRect(&rFull, S2F(-1000), S2F(-500), S2F(1000), S2F(500));
	SetFRect(&rInside, S2F(-900), S2F(-400), S2F(900), S2F(400));

	SetFRect(&rFrameLeft, S2F(-1000),  S2F(-500), S2F(-900), S2F(500));
	SetFRect(&rFrameTop, S2F(-900), S2F(-500), S2F(1000), S2F(-400));
	SetFRect(&rFrameRight, S2F(900), S2F(-400), S2F(1000), S2F(500));
	SetFRect(&rFrameBottom, S2F(-900), S2F(400), S2F(900), S2F(500));
	
	SHADE(zMed);
	DFillRect(&rInside, 0L);
	SHADE(zWhite);
	DFillRect(&rFrameLeft, 0L);
	DFillRect(&rFrameTop, 0L);
	DFillRect(&rFrameRight, 0L);
	DFillRect(&rFrameBottom, 0L);

#endif
#if 0
	if (var(isExpl) || var(isForm)) {
		Expl *e = &var(expl);

		SHADE(zLt1);
		EDFrameRect(e, &classVar(headRect), 0L);
		SHADE(zWhite);
		EDFrameRect(e, &classVar(r0), 0L);
		DITHERSHADE(18);
		EDFillRect(e, &classVar(r1), 0L);
		EDFillRect(e, &classVar(r3), 0L);
		DITHERSHADE(28);
		EDFillRect(e, &classVar(r2), 0L);
		EDHLine(e, -f600, f600, -f600, 0L);
		EDHLine(e, -f600, f600, f600, 0L);
		EDVLine(e, -f600, f600, -f600, 0L);
		SHADE(zWhite);
		EDVLine(e, -f600, f600, f600, 0L);
		EDSetPixel(e, 0L, -f900, 0L, true);
		EDSetPixel(e, 0L, f900, 0L, true);
		}
	else {
		SHADE(zLt1);
		DFrameRect(&classVar(headRect), 0L);
		SHADE(zWhite);
		DFrameRect(&classVar(r0), 0L);
		DITHERSHADE(18);
		DFillRect(&classVar(r1), 0L);
		DFillRect(&classVar(r3), 0L);
		DITHERSHADE(28);
		DFillRect(&classVar(r2), 0L);
		DHLine(-f600, f600, -f600, 0L);
		DHLine(-f600, f600, f600, 0L);
		DVSLine(-f600, f600, -f600, 0L, true, 0);
		SHADE(zWhite);
		DVLine(-f600, f600, f600, 0L);
		DSetPixel(0L, -f900, 0L, true);
		DSetPixel(0L, f900, 0L, true);
		}
#endif

}

defineMethod (Collided) (
	Thing *self,
	Vis *obj )
{
	if (objVar(obj, theClass) == (void *) gPShotClass) {
		var(checkCollide) = false;
		var(isExpl) = true;
		InitExpl(&var(expl), &var(extent), 
			objVar(obj, x) - var(x), objVar(obj, y) - var(y),
			S2F(100), 50);
		(message(theGame, DeltaScore), -400);
		}
	else if (objVar(obj, theClass) == (void *) gPlayerClass) {
		var(pleaseRemove) = true;
		var(checkCollide) = false;
		(message(theGame, DeltaEnergy), 40);
		}
}

defineClassObject;

void AddThing ( short x, short y, short z )
{
	Thing *th;
	
	th = newInstance(Thing);
	(message(th, IThing), S2F(x), S2F(y), S2F(z));
	(message(theTunnel, AddObject), (Vis *) th);
}

