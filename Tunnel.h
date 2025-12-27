/*
 * ZGraf - Tunnel.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#ifndef _H_Tunnel

	#include "Root.h"
	#include "Vis.h"

	#define _H_Tunnel
	
	#define 	Tunnel_classDeclarations									\
					Root_classDeclarations									\
					declareMethod (ITunnel);								\
					declareMethod (AddObject);								\
					declareMethod (MoveObject);								\
					declareMethod (RemoveObject);							\
					declareMethod (CheckCollisions);						\
					declareMethod (DeepCollide);							\
					declareMethod (ProcessObjects);							\
					declareMethod (DrawObjects);							\
					declareMethod (HiliteNext);								\
					declareMethod (Distance);								\
					declareMethod (ClearAll);								\
	
	
	#define		Tunnel_instanceDeclarations									\
					Root_instanceDeclarations								\
					Vis *target;											\
					Vis *player;											\
					Vis *halfTunnel;										\
					Vis *hilited;											\
					Box tunBox;												\
					Fixed rectGap;											\
					Fixed farRectIncr;										\
					Fixed maxVisDist;										\
					Fixed minVisDist;										\
					Fixed eyeToTarget;										\
	
	declareClass (Tunnel, Root);
	
	void ITunnelClass (TunnelClass *classObj);
	
	extern short xRadarDiv;
	extern short yRadarDiv;
	
#endif
