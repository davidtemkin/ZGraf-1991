/*
 * ZGraf - Exceptions.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <setjmp.h>


// -------- resource ids & other constants

#define kSilentErr				1		// pass to Failure to fail without any user alert

	enum								// within the STR# 131 rsrc, these are the
	{									//   error msg indices
		iOutOfMemory = 1,
		iNoResource,
		iOSError,
		iMemoryError,
		iResourceError
	};


// -------- typedefs

typedef struct FailInfo {
	long A6Link;
	char handlerType;
	char fPropagate;
	void *handlerProc;
	struct FailInfo	*next;
	jmp_buf regs;
	} FailInfo;

typedef pascal void (*HandlerFuncPtr)(short error, long message);


// -------- globals

extern char			gAskFailure;	/* When TRUE, break into debugger on functions
									 *	that may conditionally fail, e.g. FailNIL().
									 *	Provides opportunity for simulating failures
									 */
										
extern char 		gBreakFailure;	/* When TRUE, break into debugger when a failure
									 *	has occurred.
									 */

extern short		gLastError;		/* last error code that caused a failure  */
extern long			gLastMessage;	/* last message associated with a failure */


// -------- functions

void	PushTryHandler(FailInfo *fi);

pascal short CatchException(FailInfo *fi);
pascal void CatchFailures(FailInfo *fi, HandlerFuncPtr handler, long staticLink);
pascal void	Failure(short error, long message);
pascal void	FailMemError(void);
pascal void	FailResError(void);
pascal void	FailNIL(void *p);
pascal void	FailOSErr(short error);
pascal void	FailNILRes(void* p);
pascal void	Success(void);
pascal void	RetryException(FailInfo *fi);

pascal long	SpecifyMsg(short strListID, short strIndex);
pascal void	SetFailInfo(short newError, long newMessage);

#pragma nooptimize(CatchException)


// -------- macros for normal usage

#define try											\
		{ FailInfo __fi;							\
		  PushTryHandler( &__fi);					\
		  if (!setjmp( __fi.regs)) {
		

#define catch										\
			Success(); }							\
		else {
		

#define endTry										\
		if (__fi.fPropagate)						\
		Failure( gLastError, gLastMessage);			\
		}	}

			
#define retry										\
		RetryException( &__fi)
	

#define noPropagate									\
		__fi.fPropagate = 0

