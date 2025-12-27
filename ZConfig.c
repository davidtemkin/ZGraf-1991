/*
 * ZGraf - ZConfig.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

// #define forDebug

#include "GameHeaders.h"

#include <Video.h>

#include "ZGraf.h"

#include "ZConfig.h"


Config gConfig;

CntrlParam videoParam;
VDPageInfo modeRecord;

void GetConfig ()
{
	SysEnvRec theSys;
	short volume;
	AuxDCEHandle aHand;
	Byte mode;
	long *altScrnBase;

	SysEnvirons(1, &theSys);
	gConfig.hasColorQD = theSys.hasColorQD;

	if (gConfig.hasColorQD) {
		gConfig.gdev = GetMainDevice();
		gConfig.pmap = (**gConfig.gdev).gdPMap;
		gConfig.origScreenDepth = (**gConfig.pmap).pixelSize;
		gConfig.gameScreenDepth = gConfig.origScreenDepth;
		aHand = (AuxDCEHandle) GetDCtlEntry((**gConfig.gdev).gdRefNum);
		gConfig.slotNo = (**aHand).dCtlSlot;
		gConfig.absPlayRect = (**gConfig.pmap).bounds;
		OffsetRect(&gConfig.absPlayRect,
			-gConfig.absPlayRect.left,
			-gConfig.absPlayRect.top);
		if (addrMode == true32b)
			gConfig.req32BitSwitch = false;
		else
			gConfig.req32BitSwitch = true;
		gConfig.useOrigAltScreen = false;
		}		
	else {
		gConfig.req32BitSwitch = false;
		gConfig.origScreenDepth = 1;
		gConfig.gameScreenDepth = gConfig.origScreenDepth;
		gConfig.absPlayRect = qd.screenBits.bounds;
		OffsetRect(&gConfig.absPlayRect,
			-gConfig.absPlayRect.left,
			-gConfig.absPlayRect.top);
		altScrnBase = (long *) ((long) ScrnBase - PAGEDIFF);
		if (*altScrnBase == 'MINE')
			gConfig.useOrigAltScreen = true;
		else
			gConfig.useOrigAltScreen = false;
		}

	gConfig.speedFactor = 0x00010000;
	gConfig.allowBackgroundProcessing = false;
	gConfig.useVBL = true;

	GetSoundVol(&volume);
	if (volume == 0)
		gConfig.useSound = false;
	else {
		gConfig.useSound = true;
		gConfig.useSoundManager = false;
		}

/*
	#ifdef forDebug
		gConfig.useSound = false;
		gConfig.useVBL = false;
		#endif
*/
	gConfig.isSoundKitInstalled = false;	
	gConfig.isSyncTaskInstalled = false;
	gConfig.isTimeTaskInstalled = false;

	gConfig.isGameCLUTInstalled = false;

}
