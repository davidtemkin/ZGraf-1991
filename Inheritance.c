/*
 * ZGraf - Inheritance.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include "Exceptions.h"
#include "Inheritance.h"

long _Member (
	void *instance,
	void *class )
{
	asm 68000 {
		clr.l	d0				// return value set to false at outset
		move.l	instance, a0
		move.l	class, a1
		move.l	(a0), a0		// get the address of the class
								// object from the instance object
	@check:
		cmp.l	a1, a0
		beq		@isMember
		move.l	(a0), a0
		beq		@out
		bra		@check
	@isMember:
		move.l	#true, d0
	@out:
		}
}

void *_NewInstance (
	long instSize,
	void *classObject )
{
	long *theInst;
	
	FailNIL(theInst = (long *) allocMem(instSize));
	*theInst = (long) classObject;

	return ((void *) theInst);
}
