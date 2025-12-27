/*
 * ZGraf - CDisplay.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#ifndef _H_CDisplay

	#include "Display.h"
	#define _H_CDisplay

	
	#define 	CDisplay_classDeclarations									\
					Display_classDeclarations								\
					declareMethod (ICDisplay);								\
	

	#define		CDisplay_instanceDeclarations								\
					Display_instanceDeclarations							\
					short useColor; 										\
					ZBitMap *leftViewBM, *rightViewBM; 						\
	
	
	declareClass (CDisplay, Display);
	
	void ICDisplayClass (CDisplayClass *classObj);

	#define _HWPriv 0xA198

#endif
