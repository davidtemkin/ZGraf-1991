/*
 * ZGraf - PShot.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#ifndef _H_PShot

	#include "Vis.h"

	#define _H_PShot
	
	
	#define 	PShot_classDeclarations							\
					Vis_classDeclarations						\
					declareMethod (IPShot);						\
																\
					Fixed startDist; 							\
					Fixed endDist;								\
					Fixed stdSpeed;								\
	
	
	#define		PShot_instanceDeclarations						\
					Vis_instanceDeclarations					\
					Fixed dist;									\
					short numFrames;							\
	
	declareClass (PShot, Vis);
	
	extern void IPShotClass(PShotClass *classPtr);

#endif