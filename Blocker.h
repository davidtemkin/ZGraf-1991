/*
 * ZGraf - Blocker.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#ifndef _H_Blocker
#define _H_Blocker

	#include "Vis.h"



	#define 	Blocker_classDeclarations									\
					Vis_classDeclarations									\
					declareMethod (IBlocker);								\


	#define		Blocker_instanceDeclarations								\
					Vis_instanceDeclarations								\
					short 	angle; 											\
					short	newAngle; 										\
					Fixed	velocity; 										\
					short	direction;										\
					FRect	bRect;											\
	
	
	declareClass (Blocker, Vis);
	
	
	extern void AddBlocker(short x, short y, short z);
	extern void IBlockerClass(BlockerClass *classPtr);
	

#endif
