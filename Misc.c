/*
 * ZGraf - Misc.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include <Traps.h>
#include "Misc.h"

void InitToolbox ()
{
	InitGraf(&qd.thePort);
	InitCursor();
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	MaxApplZone();
	
	/* Initialize random number generator */
	
	GetDateTime((unsigned long *) &qd.randSeed);
}

/*
	This function comes from Think Class Library 1.1
*/

Boolean TrapAvailable (
	short theTrap )
{
	TrapType tType;
	short    numToolBoxTraps;
	
				// first determine the trap type
				
	tType = (theTrap & 0x800) > 0 ? ToolTrap : OSTrap;
	
				// next find out how many traps there are
				
	if (NGetTrapAddress( _InitGraf, ToolTrap) == NGetTrapAddress( 0xAA6E, ToolTrap))
		numToolBoxTraps = 0x200;
	else
		numToolBoxTraps = 0x400;
	
				// check if the trap number is too big for the
				// current trap table
				
	if (tType == ToolTrap)
	{
		theTrap &= 0x7FF;
		if (theTrap >= numToolBoxTraps)
			theTrap = _Unimplemented;
	}
	
				// the trap is implemented if its address is
				// different from the unimplemented trap
				
	return (NGetTrapAddress( theTrap, tType) != 
			NGetTrapAddress(_Unimplemented, ToolTrap));
}
	
void HandleError (
	ErrorCode err )
{
	char s[100];
	
	switch (err) {
		case outOfMemory:
			printf("Sorry, bud. Out of memory.\n\n");
			printf("Press Return to exit: ");
			gets(s);
			ExitToShell();
			break;
		}
}

Boolean IsKeyPressed (
	unsigned short k )

{
	unsigned char km[16];

	asm {
		pea		km
		GetKeys
		clr.l (sp)+
		}

	return ((km[k>>3] >> (k&7)) & 1);
}