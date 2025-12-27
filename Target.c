/*
 * ZGraf - Target.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include "Inheritance.h"

#include "Anim.h"
#include "Tunnel.h"
#include "Display.h"
#include "Game.h"
#include "Target.h"

#define CLASS		Target
#define SUPERCLASS	Invis

#define methodDefinitions													\
			definesMethod (ITarget);										\
			definesMethod (Draw);											\

void ITargetClass (
	TargetClass *classPtr )
{
	IInvisClass((InvisClass *) classPtr);

}

defineMethod (ITarget) (
	Target *self,
	Fixed x, Fixed y,
	Fixed zFromPlayer )
{
	(method(IInvis), x, y, zFromPlayer);	
}

defineMethod (Draw) (
	Target *self )
{
	if (objVar(theGame, level) != kDemoLevel)
		(message(theDisplay, DrawTarget));
}

defineClassObject;
