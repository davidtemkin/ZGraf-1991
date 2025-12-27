/*
 * ZGraf - Game.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#ifndef _H_Game

	#include "Root.h"
	#define _H_Game

	#define 	Game_classDeclarations							\
					Root_classDeclarations						\
																\
					declareMethod (IGame);						\
					declareMethod (UpdateStatusDisplay);		\
					declareMethod (DeltaScore);					\
					declareMethod (DeltaEnergy);				\
					declareMethod (Process);					\
					declareMethod (AddedOne);					\
					declareMethod (HitOne);						\
					declareMethod (StartLevel);					\
					declareMethod (NextLevel);					\
	
	
	#define		Game_instanceDeclarations						\
					Root_instanceDeclarations					\
																\
					long displayedScore, score; 				\
					short ticksAccumulated;						\
					short ticksPerEnergyUnit;					\
					short ticksInGameOver;						\
					short displayedEnergy, energy;				\
					short displayedLevel, level;				\
					Boolean startNewGame, startDemoMode;		\
					Rect fullScoreRect, fullLevelRect;			\
					Rect fullEnergyRect;						\
					Boolean scoreChanged;						\
					Boolean levelChanged;						\
					Boolean energyChanged;						\
					short ticks;								\
					short seconds;								\
					short minutes;								\
					long totalLevelSeconds;						\
					short numLiveObjs;							\
					short minObjsAllowed;						\
					Boolean holdUseSound;						\
					Boolean inGameOver;							\
					ZBitMap *pressToPlay;						\
					ZBitMap *gameOver;							\
					short noticeLeft, noticeTop;				\
					Rect noticeRect;							\
	
	
	declareClass (Game, Root);
	
	void IGameClass (GameClass *classObj);
	
	#define kDemoLevel 0

#endif