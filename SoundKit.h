/*
 * ZGraf - SoundKit.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

extern short OldSound;
extern void InitSoundKit(void);
extern void CloseSoundKit(void);
extern void PlayA(short soundNum, short priority);
extern void PlayB(short soundNum, short priority);
extern void SKVolume(short volume);

// Macros for conditional use of the SoundKit.

#define PLAYA(sn, pr) \
	{ \
		if (gConfig.useSound && !objVar(theGame, inGameOver)) \
			PlayA(sn, pr); \
		}

#define PLAYB(sn, pr) \
	{ \
		if (gConfig.useSound && !objVar(theGame, inGameOver)) \
			PlayB(sn, pr); \
		}

#define SKVOLUME(v) \
	{ \
		if (gConfig.useSound) \
			SKVolume(v); \
		}
