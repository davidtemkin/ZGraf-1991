/*
 * ZGraf - Player.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#ifndef _H_Player

	#include "Vis.h"
	#define _H_Player
	
	
	#define 	Player_classDeclarations									\
					Vis_classDeclarations									\
																			\
					declareMethod (IPlayer);								\
					declareMethod (GetInput);								\
					declareMethod (GetDemoInput);							\
					declareMethod (MoveAttachedObjects);					\
																			\
					Fixed accelStep;										\
					Fixed slowFactor;										\
					Fixed demoCruisingSpeed;								\
					Fixed speedLimit;										\
					Fixed hvSpeedLimit;										\
					Fixed hvAccelStep;										\
					Fixed hvSlowFactor;										\
	
	
	#define		Player_instanceDeclarations									\
					Vis_instanceDeclarations								\
																			\
					short xMouseFactor, yMouseFactor;						\
					short xScreenCtr, yScreenCtr;							\
					short hScale, vScale;									\
					Boolean quitFlag, pauseFlag;							\
					short eventMask;										\
					Boolean wasHit;											\
	
	
	declareClass (Player, Vis);
	
	void IPlayerClass (PlayerClass *classObj);

#endif
