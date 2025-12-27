/*
 * ZGraf - EDS.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#ifndef _H_EDS

	#include "Vis.h"
	
	#define _H_EDS
	
	#define 	EDS_classDeclarations							\
					Vis_classDeclarations						\
																\
					declareMethod (IEDS);						\
																\
					FRect e1, e2, e3;							\
					FRect d1, d2, d3, d4;						\
					FRect s1, s2, s3;							\
	
	
	#define		EDS_instanceDeclarations						\
					Vis_instanceDeclarations					\
	
	
	declareClass (EDS, Vis);
	
	void IEDSClass(EDSClass *classObj);
	

#endif
