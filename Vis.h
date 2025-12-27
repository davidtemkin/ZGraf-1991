/*
 * ZGraf - Vis.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#ifndef _H_Vis

	#include "Root.h"
	#define _H_Vis


	// Shape resource structs
	
	typedef struct   {
		FRect	 	rFRect;
		short	 	rShade;
		Boolean		rFrame;
		Boolean  	rCollidable;
		RGBColor 	rColor;
		} FShapeRect;
	
	typedef struct   {
		Rect	 	rRect;
		Byte	 	rShade;
		Byte		rDummy;
		short		rFrame;
		short	  	rCollidable;
		RGBColor 	rColor;
		} SShapeRect;
	
	typedef struct ShapeRec {
		short 		 sSize;
		short		 sCount;
		SShapeRect   sRects;
		} *ShapeRecPtr, **ShapeRecHdl;


	#define 	Vis_classDeclarations									\
					Root_classDeclarations								\
																		\
					declareMethod (IVis);								\
					declareMethod (Process);							\
					declareMethod (Hilite);								\
					declareMethod (Draw);								\
					declareMethod (DrawRadar);							\
					declareMethod (CheckBounds);						\
					declareMethod (AdjustSize);							\
					declareMethod (Collided);							\
																		\
					FRect 		stdExtent;								\
					Fixed 		stdThick;								\
					Fixed 		stdExplIncr, stdFormIncr;				\
					short 		stdExplSteps, stdFormSteps;				\
					Expl 		stdForm;								\
					Boolean 	stdExploding;							\
					short 		stdRectCount;							\
					FShapeRect*	stdFRects;								\
	
	
	#define		Vis_instanceDeclarations								\
					Root_instanceDeclarations							\
																		\
					Boolean isProcessed;								\
					Boolean pleaseRemove;								\
					Boolean hilite;										\
					Boolean checkCollide;								\
					Boolean isExpl, isForm;								\
					Fixed x, y;											\
					Fixed zToNext;										\
					Fixed zFromPlayer;									\
					Fixed xVel, yVel, zVel;								\
					FRect extent;										\
					Fixed thick;										\
					Rect hotSpot;										\
					FRect collideExtent;								\
					Fixed collideThick;									\
					Expl expl;											\
					struct Vis *prev, *next;							\
					FShapeRect*	fInstanceFRects;						\

	declareClass (Vis, Root);
	
	extern void IVisClass(VisClass *classObj, short shapeResID);


#endif
