/*
 * ZGraf - ZConfig.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#ifndef _H_ZConfig

	#define _H_ZConfig

	typedef struct {
	
		Boolean hasColorQD;
	
		GDHandle gdev;
		PixMapHandle pmap;
		short slotNo;

		Rect absPlayRect;
		Fixed speedFactor, speedFactorSquared;

		Boolean allowBackgroundProcessing;

		Boolean useOrigAltScreen;
		Boolean req32BitSwitch;
		
		Boolean useVBL;
		Boolean useSound;
		Boolean useSoundManager;
		
		Boolean isSoundKitInstalled;
		Boolean isSyncTaskInstalled;
		Boolean isTimeTaskInstalled;
		Boolean isGameCLUTInstalled;
		
		short origScreenDepth;
		short gameScreenDepth;
		
		} Config;

	extern Config gConfig;

	extern CntrlParam videoParam;
	extern VDPageInfo modeRecord;

	void GetConfig(void);

#endif
