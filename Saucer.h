/*
 * ZGraf - Saucer.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#ifndef _H_Saucer

	#include "Vis.h"
		
	#define _H_Saucer
	
	
	#define 	Saucer_classDeclarations								\
					Vis_classDeclarations								\
					declareMethod (ISaucer);							\
					Fixed fShootDistance;								\
					short fShotWait;									\
	
	
	#define		Saucer_instanceDeclarations								\
					Vis_instanceDeclarations							\
					short fShotTimer;									\
					short fShotCount;									\
					short fColorTimer;									\
	
	declareClass (Saucer, Vis);
	
	extern void ISaucerClass(SaucerClass *classPtr);
	extern void AddSaucer ( short x, short y, short z,
		short xVel, short yVel, short zVel );
	
#endif