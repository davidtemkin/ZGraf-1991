/*
 * ZGraf - Root.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#ifndef _H_Root

	#define _H_Root
	
	
	#define 	Root_classDeclarations										\
					declareMethod (IRoot);									\
					declareMethod (Free);									\
					
	
	#define		Root_instanceDeclarations									\
	
	
	declareRootClass(Root);
	
	
	extern void IRootClass(RootClass *classPtr);

#endif
