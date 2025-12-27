/*
 * ZGraf - JumpTable.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#define _H_JumpTable

typedef struct {
	short routineOffset;
	short pushInstruction;
	short segmentNumber;
	short loadSegTrap;
	} UnloadedJTEntry;
	
typedef struct {
	short segmentNumber;
	short jumpInstruction;
	void *routineAddress;
	} LoadedJTEntry;
	
