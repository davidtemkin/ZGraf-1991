/*
 * ZGraf - Main.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include <console.h>
#include <profile.h>

#include "Assert.h"

#include "Anim.h"
#include "ZShapes.h"

void main()
{
	/*

		To generate a profile:

		1. Make sure the profile library is included.
		2. Make sure the code generation �profile� option is on.

		The profile info will be in the file �Profile.�

	*/

	#if __option(profile)
		InitProfile(200, 200);
		printf("\n");
		#endif
		
	InitToolbox();
	gLastError = 0;
	gLastMessage = 0;
	
	try {
	
		GetConfig();
		InitUtils();
		InitAnim();
		DoAnim();

		#if __option(profile)
		{
			char volName[100];
			short vRefNum;
			FInfo fndrInfo;
			OSErr err;
	
			cecho2file("Profile Info", 0, stdout);
			DumpProfile();
			GetVol((StringPtr) volName, &vRefNum);
			err = GetFInfo("\pProfile Info", vRefNum, &fndrInfo);
			fndrInfo.fdType = 'TEXT';
			fndrInfo.fdCreator = 'KAHL';
			err = SetFInfo("\pProfile Info", vRefNum, &fndrInfo);
			}
			#endif
			
		SafeQuit();

		}

	catch {
	
		 if (gLastError != kSilentErr) {
		 	noPropagate;
		 	SafeCleanUp();
		 	ErrorAlert(gLastError, gLastMessage);
			SafeQuit();
		 	}
		}

	endTry;
}
