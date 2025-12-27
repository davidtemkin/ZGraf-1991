/*
 * ZGraf - Anim.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#ifndef _H_Anim
#define _H_Anim
	
	#include "Tunnel.h"
	#include "Display.h"
	#include "Player.h"
	#include "Target.h"
	#include "Game.h"

	// Types
	
	typedef struct SyncVBLTask {
		struct MyVBLTask *qLink;
	 	short qType;
		ProcPtr vblAddr;
		short vblCount;
		short vblPhase;
		short isVBL;
		short doFlip;
		} SyncVBLTask;
	
	typedef struct TimeVBLTask {
		struct MyVBLTask *qLink;
	 	short qType;
		ProcPtr vblAddr;
		short vblCount;
		short vblPhase;
		short ticksElapsed;
		} TimeVBLTask;
	
	// Constants
	
	#define vPage2  6
	#define vBase   0xEFE1FE
	#define vBufA (512*15)
	
	// Low memory globals
	
	extern short KeyMap1 : 0x0174;
	extern short KeyMap2 : 0x0176;
	extern short KeyMap3 : 0x0178;
	extern short KeyMap4 : 0x017A;
	
	// Application globals
	
	extern Tunnel *theTunnel;
	extern Player *thePlayer;
	extern Display *theDisplay;
	extern Game *theGame;
	
	extern SyncVBLTask syncTask;
	extern TimeVBLTask timeTask;
	extern short xRadarDiv;
	extern short yRadarDiv;
	
	// Function prototypes
	
	void InitVBL(void);
	void BenchmarkSystem(void);
	void DoAnim(void);
	void DoSyncVBL(void);
	void DoTimeVBL(void);
	void SafeCleanUp(void);
	void SafeQuit(void);

	// Macros
	
	#define ShowPage1() asm { bset.b #vPage2,vBase+vBufA }
	#define ShowPage2() asm { bclr.b #vPage2,vBase+vBufA }
	
#endif
