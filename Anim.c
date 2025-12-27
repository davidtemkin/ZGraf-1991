/*
 * ZGraf - Anim.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include <console.h>
#include <Retrace.h>

#include "SoundKit.h"
#include "SoundList.h"

#include "Inheritance.h"

#include "Utils.h"
#include "Vis.h"
#include "Player.h"
#include "Cross.h"
#include "Display.h"
#include "CDisplay.h"
#include "Game.h"
#include "Tunnel.h"
#include "Thing.h"
#include "Anim.h"

Tunnel		*theTunnel;
Player		*thePlayer;
Display		*theDisplay;
Game		*theGame;

SyncVBLTask syncTask;
TimeVBLTask timeTask;

short xRadarDiv;
short yRadarDiv;

void InitAnim ()
{
	Box box;
	Thing *theThing;
	long lDist;
	
	PrepareAllClasses();
	InitMainClasses();

	// Install my VBL task before the Sound Kit installs its task.
	
	InitVBL();
	
	// Allocate and initialize the display. Must be done before 
	// other game objects.
	
	if (gConfig.gameScreenDepth == 1) 
		{
		theDisplay = newInstance(Display);
		(message(theDisplay, IDisplay));
		}
	else {
		theDisplay = (Display *) newInstance(CDisplay);
		(message((CDisplay *) theDisplay, ICDisplay));
		}

	// Benchmark to calibrate game�s speed to system�s speed.
	// Initialize the Sound Kit.
	
	BenchmarkSystem();
	InitScalableClasses();

	theGame = newInstance(Game);
	(message(theGame, IGame));
	
	// Complete preparation of display -- draw status box at right.
	
	(message(theDisplay, DrawInitialStatus));
	
	// Allocate and initialize the tunnel. Also creates player object.

	SetBox(&box, HALFMINFIXED + 0x00200000, HALFMINFIXED, HALFMINFIXED, 
		HALFMAXFIXED - 0x00200000, HALFMAXFIXED, HALFMAXFIXED);

	theTunnel = newInstance(Tunnel);
	(message(theTunnel, ITunnel), &box, S2F(2000),
		MAXFIXED, S2F(30), S2F(400));

	thePlayer = (Player *) objVar(theTunnel, player);

	(message(theGame, StartLevel), kDemoLevel);
}

void InitVBL ()
{
	OSErr err;
	
	/*
		 Install sync VBL task for smooth animation:
		 
		 This is installed regardless of the setting of 
		 ovar(theDisplay, useVideoSync). It�s based on the
		 setting of gConfig.useVbl.
		 
		 Also, a timing VBL is always installed, regardless
		 of any configuration settings.
	*/
	
	if (gConfig.useVBL) {
		syncTask.qType = vType;
		syncTask.vblAddr = (ProcPtr) &DoSyncVBL;
		syncTask.vblCount = 1;
		syncTask.vblPhase = 0;
		syncTask.isVBL = false;
		syncTask.doFlip = 0;
		if (gConfig.hasColorQD)
			FailOSErr(SlotVInstall((QElemPtr) &syncTask, gConfig.slotNo));
		else
			FailOSErr(VInstall((QElemPtr) &syncTask));

		gConfig.isSyncTaskInstalled = true;
		
		}
	
	// Always install timer VBL
	
	timeTask.qType = vType;
	timeTask.vblAddr = (ProcPtr) &DoTimeVBL;
	timeTask.vblCount = 1;
	timeTask.vblPhase = 0;
	timeTask.ticksElapsed = 0;
	FailOSErr(VInstall((QElemPtr) &timeTask));
	
	gConfig.isTimeTaskInstalled = true;
}

void BenchmarkSystem ()
{
	ZBitMap *tmp;
	PicHandle pleaseWait;
	long startTime, endTime;
	Rect r;
	short xLeft, yTop;
	short i, x, y;
	FRect firstRect, drawRect;
	
	pleaseWait = nil;
	FailNILRes(pleaseWait = (PicHandle) GetResource('PICT', 128));
	r = (**pleaseWait).picFrame;
	tmp = ZNewBitMap(r.right, r.bottom);
	ZSetBitMap(tmp);		
	SHADE(zWhite);
	ZFillRect(&r);					// DrawPicture depends on white background
	DrawPicture(pleaseWait, &r);
	xLeft = (zPage1BitMap->xBound - r.right) / 2;
	yTop = (zPage1BitMap->yBound - r.bottom) / 2;

	for (i = 0; i < 2; i++) {
		(message(theDisplay, ClearBuffers));
		ZCopyLongs(tmp, objVar(theDisplay, renderFullBM),
			&r, xLeft, yTop);
		(message(theDisplay, ShowBuffers));
		}

	if (gConfig.useSound) {
		OldSound = !gConfig.useSoundManager;
		InitSoundKit();
		gConfig.isSoundKitInstalled = true;
		}

	xEyePos = yEyePos = zEyePos = 0;
	SetFRect(&firstRect, S2F(-500), S2F(-500), S2F(-450), S2F(-450));
	startTime = TickCount();

	for (i = 982; i >= 2; i -= 20) {
		(message(theDisplay, ClearBuffers));
		BITS(objVar(theDisplay, renderViewBM));
		SHADE(zWhite);		
		for (x = 0; x < 10; x++) {
			for (y = 0; y < 10; y++) {
				drawRect = firstRect;
				OffsetFRect(&drawRect, S2F(x * 100), S2F(y * 100));
				DFillRect(&drawRect, S2F(i));
				}
			}
		ZCopyLongs(tmp, objVar(theDisplay, renderFullBM),
			&r, xLeft, yTop);
		(message(theDisplay, ShowBuffers));
		}

	endTime = TickCount();
	
	// Flush out anything which may have been displayed
	
	(message(theDisplay, ClearBuffers));
	(message(theDisplay, ShowBuffers));
	(message(theDisplay, ClearBuffers));
	(message(theDisplay, ShowBuffers));
	
	// "Standard" time: 101 ticks (SE/30; internal monitor, no flipping).
	// Slower configurations will have speedFactor > 1.

	gConfig.speedFactor = FixRatio(endTime - startTime, 101);

//super-fast:
//gConfig.speedFactor = FixMul(gConfig.speedFactor, S2F(10));
	
	gConfig.speedFactorSquared = FixMul(gConfig.speedFactor, gConfig.speedFactor);
	
	ZFreeBitMap(tmp);
	ReleaseResource((Handle) pleaseWait);

	#ifdef TEST

	{
		char volName[100];
		short vRefNum;
		FInfo fndrInfo;
		OSErr err;

		cecho2file("FrameRate", 0, stdout);
		printf("Ticks elapsed: %ld\n", endTime - startTime);
		GetVol((StringPtr) volName, &vRefNum);
		err = GetFInfo("\pFrameRate", vRefNum, &fndrInfo);
		fndrInfo.fdType = 'TEXT';
		fndrInfo.fdCreator = 'KAHL';
		err = SetFInfo("\pFrameRate", vRefNum, &fndrInfo);
	}

	#endif

}

void SafeCleanUp ()
{
	// make sure no exceptions are raised here

	if (theDisplay)
		(message(theDisplay, CleanUp), (long) true);
	if (gConfig.isSoundKitInstalled)
		CloseSoundKit();
}

void SafeQuit ()
{
	// make sure no exceptions are raised here

	SafeCleanUp();

	NSetTrapAddress(gOldExit, 0x9F4, ToolTrap);
	ExitToShell();
}

/*
	These routines are called every 1/60th of a second.
	Profiling must be off for these or else the
	program will choke while running.

	If theTask.doFlip = 0, DoSyncVBL simply increments theTask.isVBL.

	If theTask.doFlip is 1 or 2, DoSyncVBL flips the video page as
		appropriate (on 68000 toaster Macs only).

*/

void DoAnim ()
{
	long startTime, endTime;
	double totalTicks, frameRate;
	Boolean calcFrameRate = false;
	short oldTicks;
	short ticksElapsed;

	startTime = TickCount();
	objVar(theDisplay, frameNo) = 0;
	timeTask.ticksElapsed = 0;

	while (!objVar(thePlayer,quitFlag)) {
		(message(theGame, Process));
		if (objVar(theGame, level) == kDemoLevel || objVar(theGame, inGameOver))
			(message(thePlayer, GetDemoInput));
		else
			(message(thePlayer, GetInput));
		(message(theDisplay, ClearBuffers));
		(message(theTunnel, CheckCollisions));
		(message(theTunnel, ProcessObjects));
		if (objVar(thePlayer, wasHit)) {
			BITS(objVar(theDisplay, renderFullBM));
			DITHERSHADE(27);
			ZFillRect(&objVar(theDisplay, viewRect));
			objVar(thePlayer, wasHit) = false;
			}
		else 
			(message(theTunnel, DrawObjects));
		
		(message(theGame, UpdateStatusDisplay));
		(message(theDisplay, ShowBuffers));
		zCurDitherIndex++;
		if (zCurDitherIndex == 8)
			zCurDitherIndex = 0;
		zCurDitherTable = &(*zDitherShades)[zCurDitherIndex];
		objVar(theDisplay, frameNo)++;
		}
	endTime = TickCount();

	(message(theDisplay, CleanUp), (long) false);
	
	if (gConfig.isSoundKitInstalled) {
		CloseSoundKit();
		gConfig.isSoundKitInstalled = false;
		}

//	#define RECORDING
	#ifdef RECORDING

	{
		char volName[100];
		short vRefNum;
		FInfo fndrInfo;
		OSErr err;
		long frames = objVar(theDisplay, frameNo);

		cecho2file("FrameRate", 0, stdout);

		printf("Ticks elapsed: %ld   Frames: %ld\n",
			endTime - startTime, frames);
		totalTicks = (double) endTime - (double) startTime;
		frameRate = (double) frames / (totalTicks / (double) 60.0);
		printf("\n\nFrames per second: %f\n", frameRate);
		GetVol((StringPtr) volName, &vRefNum);
		err = GetFInfo("\pFrameRate", vRefNum, &fndrInfo);
		fndrInfo.fdType = 'TEXT';
		fndrInfo.fdCreator = 'KAHL';
		err = SetFInfo("\pFrameRate", vRefNum, &fndrInfo);
		}

	#endif

}


#pragma options(!profile)

void DoSyncVBL ()
{
	asm {
		tst.w	OFFSET(SyncVBLTask, doFlip) (a0)
		beq		@setFlag
		cmp.w	#1, OFFSET(SyncVBLTask, doFlip) (a0)
		bne		@showPage2
	@showPage1:
		bset.b	#vPage2, vBase+vBufA
		clr.w	OFFSET(SyncVBLTask, doFlip) (a0)
		bra		@setFlag
	@showPage2:
		bclr.b	#vPage2, vBase+vBufA
		clr.w	OFFSET(SyncVBLTask, doFlip) (a0)
	@setFlag:
		move.w	#1, OFFSET(SyncVBLTask, vblCount) (a0)
		move.w	#1, OFFSET(SyncVBLTask, isVBL) (a0)
		}
}

void DoTimeVBL ()
{
	asm {
		move.w	#1, OFFSET(TimeVBLTask, vblCount) (a0)
		add.w	#1, OFFSET(TimeVBLTask, ticksElapsed) (a0)
		}
}
