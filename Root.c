/*
 * ZGraf - Root.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include "Inheritance.h"

#include "Root.h"

#define CLASS	Root

#define methodDefinitions													\
			definesMethod (IRoot);											\
			definesMethod (Free);											\

void IRootClass (
	RootClass *classPtr )
{
}

defineMethod (IRoot) (
	Root *self )
{
}

defineMethod (Free) (
	Root *self )
{
	freeMem(self);
}

defineRootClassObject;
