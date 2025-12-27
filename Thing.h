/*
 * ZGraf - Thing.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#ifndef _H_Thing

	#include "Vis.h"	
	#define _H_Thing
	
	#define 	Thing_classDeclarations									\
					Vis_classDeclarations								\
																		\
					declareMethod (IThing);								\
																		\
					FRect headRect;										\
					FRect r0, r1, r2, r3, r4;							\
	
	
	#define		Thing_instanceDeclarations								\
					Vis_instanceDeclarations							\
					short cycleFrame;									\
					short frameCount;									\
					Fixed eyeIncr;										\
					Fixed legIncr;										\
					Fixed eyeOffset;									\
					Fixed legOffset;									\
	
	
	declareClass (Thing, Vis);
	
	extern void IThingClass (ThingClass *classObj);
	extern void AddThing(short x, short y, short z);

#endif