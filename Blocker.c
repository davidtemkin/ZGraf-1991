/*
 * ZGraf - Blocker.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include "Inheritance.h"

#include "Anim.h"
#include "PShot.h"
#include "Blocker.h"

#define CLASS		Blocker
#define SUPERCLASS	Vis

#define methodDefinitions													\
			definesMethod (IBlocker);										\
			definesMethod (Draw);											\
			definesMethod (Collided);

void IBlockerClass (
	BlockerClass *classPtr )
{
	IVisClass((VisClass *) classPtr, 0);
}


defineMethod (IBlocker) (
	Blocker *self,
	Fixed x, Fixed y,
	Fixed zFromPlayer,
	Boolean isHorz,
	Fixed width)
{
	(method(IVis), x, y, zFromPlayer,
		&classVar(stdExtent), classVar(stdThick), (short) true,
		0L, 0L, 0L);
	if (isHorz)
		{
		SetFRect(&var(bRect),	theTunnel->tunBox.left, y - width, theTunnel->tunBox.right, y + width);
		}
	else
		{
		SetFRect(&var(bRect),	x - width, theTunnel->tunBox.top, x + width, theTunnel->tunBox.bottom);
		}
}

defineMethod (Draw) (
	Blocker *self)
{
		zCurPat = zWhite;
		DFillRect(&var(bRect), 0L);
}

defineMethod (Collided) (
	Blocker *self,
	Vis *obj )
{
}

defineClassObject;

// ------------------------

void AddBlocker (
	short x, short y, short z )
{
	Blocker *bl;

	bl = newInstance(Blocker);
	(message(bl, IBlocker), S2F(x), S2F(y), S2F(z), true, FRand(S2F(100), S2F(1000)));
	(message(theTunnel, AddObject), (Vis *) bl);
	(message(theGame, AddedOne), bl);
}


