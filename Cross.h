/*
 * ZGraf - Cross.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#ifndef _H_Cross

	#include "Vis.h"
	#define _H_Cross
	
	
	#define 	Cross_classDeclarations							\
					Vis_classDeclarations						\
					declareMethod (ICross);						\
	
	#define		Cross_instanceDeclarations						\
					Vis_instanceDeclarations					\
	
	declareClass (Cross, Vis);
	
	void ICrossClass(CrossClass *classPtr);
	
	void AddCross(short x, short y, short z,
					short xVel, short yVel, short zVel);

#endif