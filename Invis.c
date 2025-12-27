/*
 * ZGraf - Invis.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include "Inheritance.h"

#include "Invis.h"

#define CLASS		Invis
#define SUPERCLASS	Vis

#define methodDefinitions													\
			definesMethod (IInvis);											\
			definesMethod (DrawRadar);										\
			definesMethod (Process);										\


void IInvisClass (
	InvisClass *classPtr )
{
	IVisClass((VisClass *) classPtr, 0);	
}

defineMethod (IInvis) (
	Invis *self,
	Fixed x, Fixed y,
	Fixed zFromPlayer )
{
	FRect dummyExtent;

	/*
		Objects of this class have a location, but no
		thickness or extent. No collision detection is performed
		on them.
	*/

	SetFRect(&dummyExtent, 0L, 0L, 0L, 0L);
	(method(IVis), x, y, zFromPlayer, dummyExtent, 0L, (short) false,
		0L, 0L, 0L);
	var(isExpl) = false;
	var(isForm) = false;
}

defineMethod (Process) (
	Invis *self )
{
}

defineMethod (DrawRadar) (
	Invis *self )
{
}

defineClassObject;