/*
 * ZGraf - Invis.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#ifndef _H_Invis

	#include "Vis.h"
	
	#define _H_Invis
	
	#define 	Invis_classDeclarations							\
					Vis_classDeclarations						\
					declareMethod (IInvis);						\
	
	
	#define		Invis_instanceDeclarations						\
					Vis_instanceDeclarations					\
					short 	angle; 								\
					short	newAngle; 							\
					Fixed	velocity; 							\
					short	direction;							\
	
	declareClass (Invis, Vis);
	
	extern void IInvisClass (InvisClass *classObj);

#endif