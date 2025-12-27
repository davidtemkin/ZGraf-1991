/*
 * ZGraf - Display.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#ifndef _H_Display

	#include "Root.h"
	#define _H_Display	
	
	#define 	Display_classDeclarations						\
					Root_classDeclarations						\
					declareMethod (IDisplay);					\
					declareMethod (InitGraphics);				\
					declareMethod (InitRects);					\
					declareMethod (InitBitMaps);				\
					declareMethod (BlankScreen);				\
					declareMethod (DrawInitialStatus);			\
					declareMethod (ProcessEvent);				\
					declareMethod (RenderTarget);				\
					declareMethod (DrawTarget);					\
					declareMethod (ClearBuffers);				\
					declareMethod (WaitForVBL);					\
					declareMethod (ShowAbsRect);				\
					declareMethod (ShowStatus);					\
					declareMethod (ShowBuffers);				\
					declareMethod (CleanUp);					\
	
	
	#define		Display_instanceDeclarations					\
					Root_instanceDeclarations					\
					Boolean hasSlots;							\
					Boolean useFullScreen;						\
					Boolean useVideoSync;						\
					Boolean useFlipping;						\
					Boolean page1Visible;						\
					GDHandle myGDevice;							\
					VDPageInfo videoPgInfo;						\
					long frameNo;								\
					Rect fullScreen, compactScreen;				\
					Rect gameRect;								\
					ZBitMap *renderViewBM, *renderRadarBM;		\
					ZBitMap *renderFullBM, *renderStatusBM;		\
					ZBitMap *view1BM, *view2BM;					\
					ZBitMap *radar1BM, *radar2BM;				\
					ZBitMap *full1BM, *full2BM;					\
					ZBitMap *status1BM, *status2BM;				\
					Rect viewRect, statusRect;					\
					Rect radarRect, radarCopyRect;				\
					short xViewSize, yViewSize;					\
					short halfTargetOffset;						\
					WindowRecord gameWindow;					\
	
	
	declareClass (Display, Root);
	
	// Display-related constants
	
	#define kCompactWidth 512
	#define kCompactHeight 342
	#define kStatusWidth 160
	#define kStatusHeight 342
	#define kRadarFrame 8
	#define kRadarWidth (32 - kRadarFrame)
	#define kRadarHeight (kStatusHeight - 2 * kRadarFrame)
	#define kWaitPict 128
	#define kSidePict 129
	
#endif