/*
 * ZGraf - SShot.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#ifndef _H_SShot

	#include "Vis.h"
	#define _H_SShot
	
	#define 	SShot_classDeclarations									\
					Vis_classDeclarations								\
																		\
					declareMethod (ISShot);								\
																		\
					Fixed startDist;									\
					Fixed endDist;										\
					Fixed stdSpeed;										\
	
	
	#define		SShot_instanceDeclarations								\
					Vis_instanceDeclarations							\
					Fixed dist; 										\
	
	declareClass (SShot, Vis);
	
	extern void ISShotClass (SShotClass *classObj);
	extern void AddSShot (Fixed x, Fixed y, Fixed z,
							Fixed atX, Fixed atY,
							Fixed zVel );

#endif