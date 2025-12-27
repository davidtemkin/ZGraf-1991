/*
 * ZGraf - Game.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include "Inheritance.h"

#include "StatusIndicator.h"
#include "Anim.h"
#include "Display.h"
#include "Game.h"
#include "Cross.h"
#include "Thing.h"
#include "Tunnel.h"
#include "Aphid.h"
#include "Blocker.h"
#include "Saucer.h"

#include "SoundKit.h"
#include "SoundList.h"
#include "Utils.h"

#define CLASS		Game
#define SUPERCLASS	Root

#define methodDefinitions													\
			definesMethod (IGame);											\
			definesMethod (UpdateStatusDisplay);							\
			definesMethod (DeltaScore);										\
			definesMethod (DeltaEnergy);									\
			definesMethod (Process);										\
			definesMethod (AddedOne);										\
			definesMethod (HitOne);											\
			definesMethod (StartLevel);										\
			definesMethod (NextLevel);										\

void IGameClass (
	GameClass *classPtr )
{
	IRootClass((RootClass *) classPtr);
}

defineMethod (IGame) (
	Game *self )
{
	PicHandle pic;
	Rect r;

	(method(IRoot));

	// All of these rectangles assume that (0, 0) is at the top left
	//   of the status indicator.

	SetRect(&var(fullScoreRect), kXCenter - 30, kYScore - 10,
		kXCenter + 30, kYScore + 3);
	SetRect(&var(fullLevelRect), kXCenter - 30, kYLevel - 10,
		kXCenter + 30, kYLevel + 3);
	SetRect(&var(fullEnergyRect), kLeftEnergy, kTopEnergy,
		kRightEnergy, kBottomEnergy);
	
	// Force updates of all of these

	var(displayedScore) = -1;
	var(displayedLevel) = -1;
	var(displayedEnergy) = -1;

	var(score) = 0;
	var(level) = 0;
	var(energy) = kMaxEnergy;
	var(ticksPerEnergyUnit) = 100;
	var(ticksAccumulated) = 0;
	var(ticksInGameOver) = 7 * 60;

	var(startNewGame) = false;
	var(startDemoMode) = false;

	var(inGameOver) = false;

	// Set up notice stuff

	pic = nil;
	FailNILRes(pic = (PicHandle) GetResource('PICT', 130));
	r = (**pic).picFrame;
	var(pressToPlay) = ZNewBitMap(r.right, r.bottom);
	ZSetBitMap(var(pressToPlay));		
	SHADE(zWhite);
	ZFillRect(&r);					// DrawPicture depends on white background
	DrawPicture(pic, &r);

	pic = nil;
	FailNILRes(pic = (PicHandle) GetResource('PICT', 131));
	r = (**pic).picFrame;
	var(gameOver) = ZNewBitMap(r.right, r.bottom);
	ZSetBitMap(var(gameOver));		
	SHADE(zWhite);
	ZFillRect(&r);					// DrawPicture depends on white background
	DrawPicture(pic, &r);

	var(noticeRect) = r;
	
	var(noticeLeft) = (objVar(theDisplay, statusRect).left - r.right) / 2;
	var(noticeTop) = (zPage1BitMap->yBound - r.bottom) / 2;
}

defineMethod (UpdateStatusDisplay) (
	Game *self )
{
	Rect r;
	Str255 s;
	short pixWidth;

	ZSetBitMap(objVar(theDisplay, renderStatusBM));

	if (var(score) != var(displayedScore)) {
		SHADE(zBlack);
		ZFillRect(&var(fullScoreRect));
		if (var(level) != kDemoLevel) {
			NumToString(var(score), s);
			pixWidth = StringWidth(s);
			MoveTo(kXCenter - pixWidth / 2, kYScore);
			DrawString(s);
			}
		var(displayedScore) = var(score);
		var(scoreChanged) = true;
		}

	if (var(level) != var(displayedLevel)) {
		SHADE(zBlack);
		ZFillRect(&var(fullLevelRect));
		if (var(level) != kDemoLevel) {		// Don�t show level while in demo mode
			NumToString((long) var(level), s);
			pixWidth = StringWidth(s);
			MoveTo(kXCenter - pixWidth / 2, kYLevel);
			DrawString(s);
			}
		var(displayedLevel) = var(level);
		var(levelChanged) = true;
		}

	if (var(energy) != var(displayedEnergy)) {
		SHADE(zBlack);
		ZFillRect(&var(fullEnergyRect));
		if (var(level) != kDemoLevel) {
			SHADE(zWhite);
			ZSetRect(&r, kLeftEnergy, kTopEnergy,
				kLeftEnergy + var(energy), kBottomEnergy);
			ZFillRect(&r);
			}
		var(displayedEnergy) = var(energy);
		var(energyChanged) = true;
		}

	if (var(inGameOver))
		ZCopyLongs(var(gameOver), objVar(theDisplay, renderFullBM),
			&var(noticeRect), var(noticeLeft), var(noticeTop));

	else if (var(level) == kDemoLevel && (var(totalLevelSeconds) % 3 <= 1))
		ZCopyLongs(var(pressToPlay), objVar(theDisplay, renderFullBM),
			&var(noticeRect), var(noticeLeft), var(noticeTop));

}

defineMethod (DeltaScore) (
	Game *self,
	short deltaScore )
{
	var(score) += (long) deltaScore;
}

defineMethod (DeltaEnergy) (
	Game *self,
	short deltaEnergy )
{
	var(energy) += deltaEnergy;
	if (var(energy)  > kMaxEnergy)
		var(energy) = kMaxEnergy;
	if (var(energy) < 0)
		var(energy) = 0;
}

defineMethod (NextLevel) (
	Game *self )
{
	var(level)++;
	PLAYA(kSndNum_NextLevel, kSndPriority_NextLevel);
	PLAYB(kSndNum_NextLevel, kSndPriority_NextLevel);
	(method(StartLevel), var(level));
}

defineMethod (Process) (
	Game *self )
{
	short ticksElapsed;
	long oldTotalLevelSeconds;

	ticksElapsed = timeTask.ticksElapsed;
	timeTask.ticksElapsed = 0;

	oldTotalLevelSeconds = var(totalLevelSeconds);

	// Take care of timing info

	var(ticks) += ticksElapsed;
	if (var(ticks) >= 60) {
		var(seconds) += var(ticks) / 60;
		var(totalLevelSeconds) += var(ticks) / 60;
		var(ticks) = var(ticks) % 60;
		if (var(seconds) >= 60) {
			var(minutes) += var(seconds) / 60;
			var(seconds) = var(seconds) % 60;
			}
		}

	// Check to see if a restart is in order

	if (var(startNewGame)) {
		var(startNewGame) = false;
		var(score) = 0;
		var(energy) = kMaxEnergy;
		objVar(thePlayer, zVel) = 0;
		(method(StartLevel), 1);
		}

	// Do normal processing during game play only

	if (var(inGameOver)) {
		var(ticksAccumulated) += ticksElapsed;
		if (var(ticksAccumulated) >= var(ticksInGameOver)) {
			var(inGameOver) = false;
			var(startDemoMode) = true;
			}
		}

	else if (var(level) != kDemoLevel) {
		var(ticksAccumulated) += ticksElapsed;
		if (var(ticksAccumulated) >= var(ticksPerEnergyUnit)) {
			(method(DeltaEnergy), -(var(ticksAccumulated) / var(ticksPerEnergyUnit)));
			var(ticksAccumulated) %= var(ticksPerEnergyUnit);
			}
		if (var(startDemoMode)) {
			var(startDemoMode) = false;
			(method(StartLevel), kDemoLevel);
			}
		else {
			if (var(energy) <= 0) {

				// sound must be played before going into game over mode
				
				PLAYA(kSndNum_GameOver, kSndPriority_GameOver);
				PLAYB(kSndNum_GameOver, kSndPriority_GameOver);
			
				var(inGameOver) = true;
				var(ticksAccumulated) = 0;
				}
			else if (var(numLiveObjs) == 0)
				(method(NextLevel));
			}
		}
}

defineMethod (AddedOne) (
	Game *self,
	Vis *obj )
{
	var(numLiveObjs)++;
}

defineMethod (HitOne) (
	Game *self,
	Vis *obj )
{
	var(numLiveObjs)--;
}

defineMethod (StartLevel) (
	Game *self,
	short levelNo )
{
	short numAdding;
	short i, speed;
	Cross *ci;
	Thing *th;

	var(level) = levelNo;
	var(totalLevelSeconds) = 0;
	var(numLiveObjs) = 0;
	var(ticksAccumulated) = 0;
	
	switch (levelNo) {
		case kDemoLevel:
			(message(theTunnel, ClearAll));
			var(holdUseSound) = gConfig.useSound;
			gConfig.useSound = false;
		    numAdding = SRand(100, 150);
			for (i = 0; i < numAdding; i++)
				AddAphid();
			break;
		case 1:
			var(ticksPerEnergyUnit) = 100;
			(message(theTunnel, ClearAll));
			gConfig.useSound = var(holdUseSound);
			// PLAYA(kSndNum_GameStart, kSndPriority_GameStart);
			var(displayedScore) = -1;
			var(displayedLevel) = -1;
			var(displayedEnergy) = -1;
			var(energy) = kMaxEnergy;

			numAdding = SRand(12, 25);			// was 25, 40
			for (i = 0; i < numAdding; i++)
				AddAphid();

			AddThing(6000, 200, -8000);
			
			AddCross(0, 0, 16000, 0, 0, -30);
			AddCross(0, 0, 15000, 0, 0, -30);
			AddCross(0, 0, 14000, 0, 0, -30);
			AddCross(0, 0, 13000, 0, 0, -30);
			AddSaucer(0, 0, 12200, 0, 0, -30);
			
			//AddBlocker(9500, 12100, 5000);
			
			AddSaucer(-15000, 9200, 6500, 0, 0, 0);
			AddCross(9500, 12100, 5000, 0, 0, 0);
			AddCross(12321, -800, 1200, 0, 0, 0);
			AddCross(-2000, 1000, 1920, 0, 0, 0);
		
			AddCross(1500, -4000, 2529, 0, 0, 0);
			AddCross(-4599, -3000, 2300, 0, 0, 0);
			AddCross(40, 700, -3230, 0, 0, 0);
			AddCross(1000, 5801, -7700, 0, 0, 0);
			AddCross(0, 0, -12000, 0, 0, 0);
			AddCross(-2000, 5000, -14000, 0, 0, 0);	
			break;
		default:
			var(ticksPerEnergyUnit) = 100 - 10 * (levelNo - 1);
			if (var(ticksPerEnergyUnit) < 20)
				var(ticksPerEnergyUnit) = 20;
			numAdding = (var(level) + 1) * 4;	// level 2 gets 12, 3 gets 16, etc
			for (i = 1; i < numAdding; i++) {
				if (SRand(0,100) > 50) 
					AddCross(FRand(tunnelRect.left,
							  tunnelRect.right),
							FRand(tunnelRect.top,
							  tunnelRect.bottom),
							FRand(objVar(theTunnel, tunBox).front,
							objVar(theTunnel, tunBox).back),
							SRand(-100, 100),
							SRand(-100, 100),
							SRand(-100, 100));	
				else
					AddSaucer(FRand(tunnelRect.left,
							  tunnelRect.right),
							FRand(tunnelRect.top,
							  tunnelRect.bottom),
							FRand(objVar(theTunnel, tunBox).front,
							objVar(theTunnel, tunBox).back),
							SRand(-100, 100),
							SRand(-100, 100),
							SRand(200, 400));		
	
				}
				
			// Add some "Things" for refueling (one per 15 crosses)

			numAdding = numAdding / 12;
			for (i = 1; i <= numAdding; i++) {
				th = newInstance(Thing);
				(message(th, IThing),
					FRand(objVar(theTunnel, tunBox).left,
						objVar(theTunnel, tunBox).right),
					FRand(objVar(theTunnel, tunBox).top,
						objVar(theTunnel, tunBox).bottom),
					FRand(objVar(theTunnel, tunBox).front,
						objVar(theTunnel, tunBox).back));
				(message(theTunnel, AddObject), th);
				}

			// Add some aphids starting on level 4

			if (var(level) >= 1) {
				numAdding = 3 + (var(level) ) / 3;
				for (i = 1 ; i<= numAdding; i++)
					AddAphid();
				}
			break;
		}
}

defineClassObject;