/*
 * ZGraf - ClassStart.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include "Inheritance.h"

#include "Root.h"
#include "Display.h"
#include "CDisplay.h"
#include "Vis.h"
#include "Player.h"
#include "Cross.h"
#include "Thing.h"
#include "PShot.h"
#include "Target.h"
#include "Invis.h"
#include "Tunnel.h"
#include "EDS.h"
#include "Game.h"
#include "Aphid.h"
#include "Blocker.h"
#include "Saucer.h"
#include "SShot.h"

#include "ClassStart.h"

void PrepareAllClasses()
{
	gRootClass = prepareRootClass (Root);

	gTunnelClass = prepareClass (Tunnel, Root);

	gDisplayClass = prepareClass (Display, Root);
	gCDisplayClass = prepareClass (CDisplay, Display);

	gGameClass = prepareClass (Game, Root);

	gVisClass = prepareClass (Vis, Root);
	gPlayerClass = prepareClass (Player, Vis);
	gCrossClass = prepareClass (Cross, Vis);
	gPShotClass = prepareClass (PShot, Vis);
	gInvisClass = prepareClass (Invis, Vis);
	gThingClass = prepareClass (Thing, Vis);

	gAphidClass = prepareClass (Aphid, Vis);
	gBlockerClass = prepareClass (Blocker, Vis);
	gSaucerClass = prepareClass (Saucer, Vis);
	gSShotClass = prepareClass (SShot, Vis);
	gEDSClass = prepareClass (EDS, Vis);

	gTargetClass = prepareClass (Target, Invis);
}

void InitMainClasses()
{
	IRootClass(gRootClass);	
	ITunnelClass(gTunnelClass);
	IDisplayClass(gDisplayClass);
	ICDisplayClass(gCDisplayClass);
}

void InitScalableClasses()
{
	IVisClass(gVisClass, 0);
	IPlayerClass(gPlayerClass);
	ICrossClass(gCrossClass);
	IPShotClass(gPShotClass);
	ITargetClass(gTargetClass);
	IInvisClass(gInvisClass);
	IThingClass(gThingClass);
	IAphidClass(gAphidClass);
	IEDSClass(gEDSClass);
	IBlockerClass(gBlockerClass);
	ISaucerClass(gSaucerClass);
	ISShotClass(gSShotClass);
	IGameClass(gGameClass);
}
