/*
 * ZGraf - Target.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#ifndef _H_Target

	#include "Invis.h"

	#define _H_Target

	#define 	Target_classDeclarations									\
					Invis_classDeclarations									\
					declareMethod (ITarget);								\
	
	
	#define		Target_instanceDeclarations									\
					Invis_instanceDeclarations								\
					Fixed xNew, yNew, zNew;									\
	
	declareClass (Target, Invis);

	void ITargetClass(TargetClass *classObj);

#endif