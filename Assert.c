/*
 * ZGraf - Assert.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"
#include "Assert.h"

#define		kAssertAlert		253		// resource ID for assertion alert

enum {								 	// buttons in assert alert
	assertQuit = 1,
	assertFail,
	assertContinue
	};

#ifdef forDebug

	short __assertion_failed (
		char *condition,
		char *fileName,
		long lineNum )
	{
		Str255	msg;
		unsigned char fName[63], numStr[20];
		short itemHit;
	
		if (gBreakFailure)
			Debugger();
	
		BlockMove(condition, msg, cstrlen(condition)+1);
		CtoPstr((char *) msg);
	
		BlockMove(fileName, fName, cstrlen(fileName)+1);
		CtoPstr((char*) fName);
		NumToString(lineNum, numStr);
		
		ParamText(msg, fName, numStr, 0L);
		PositionDialog('ALRT', kAssertAlert);

		SafeCleanUp();
		
		itemHit = Alert(kAssertAlert, 0L);
		switch (itemHit) {

			case assertQuit:
				SafeQuit();
				break;
				
			case assertFail:
				Failure(kSilentErr, 0);
				break;
				
			case assertContinue:
				break;
			}
		
		return 0;	
	}

#endif
