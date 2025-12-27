/*
 * ZGraf - Aphid.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#ifndef _H_Aphid
#define _H_Aphid

	#include "Vis.h"

	#define 	Aphid_classDeclarations										\
					Vis_classDeclarations									\
					declareMethod (IAphid);									\
						

	#define		Aphid_instanceDeclarations									\
					Vis_instanceDeclarations								\
					short 	angle; 											\
					short	newAngle; 										\
					Fixed	velocity; 										\
					short	direction;										\
	
	declareClass (Aphid, Vis);
	
	extern void AddAphid (void);
	extern void IAphidClass (AphidClass *classObj);

#endif
