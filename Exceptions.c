/*
 * ZGraf - Exceptions.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#include "Exceptions.h"

#include <Packages.h>


// -------- typedefs, constants

#define		JMP_BUF_A6			36		// offset of reg a6 in jmp_buf
 	
enum { 									// for handlerType field of FailInfo
	kCatchException, 
	kTry, 
	kCatchFailure
	}; 


// -------- configuration globlals

char gDefaultPropagation = true;
char gAskFailure;
char gBreakFailure;

// --------- other globals

FailInfo *gTopHandler;
short gLastError;			// OS error no.
long gLastMessage;			// Index into error msg STR# resource (should be short)

// -------- code

pascal void ThrowHandler(FailInfo*);
void NoHandler(void);

void NoHandler()
{

 	// Signals fatal error caused by a call to ThrowHandler() or Success() when 
 	// the handler stack is empty.

	#ifdef forDebug
		DebugStr( (StringPtr) "\pfailure stack is empty!");
	#endif

	ExitToShell();
}

void PushTryHandler (
	FailInfo *fi )
{
	fi->A6Link = 0;
	fi->fPropagate = gDefaultPropagation;
	fi->handlerType = kTry;
	fi->next = gTopHandler;
	gTopHandler = fi;
}

pascal void FailNIL (
	void *p )
{
 	// Calls Failure if p == NULL. If gAskFailure is TRUE, breaks into debugger
 	// before testing p. Failure may be simulated by setting p to NULL in the
 	// debugger.


	#ifdef forDebug
		if (gAskFailure)
			Debugger();
	#endif
	
	if (!p)
		Failure(memFullErr, iOutOfMemory);
}

pascal void FailNILRes (
	void *p )
{
 	// Calls Failure if p == NULL. If gAskFailure is TRUE, breaks into debugger
 	// before testing p. Failure may be simulated by setting p to NULL in the
 	// debugger.
 	
	#ifdef forDebug
		if (gAskFailure)
			Debugger();
	#endif
	
	if (!p)
	{
		OSErr err = ResError();				/* Resource Mgr may not set ResErr	*/
											/* when resource is not found. We	*/
		Failure( err? err : resNotFound, iNoResource);/* check if there is an error code,	*/
											/* if not, we use resNotFound.		*/
	}

}

pascal void FailOSErr (
	OSErr err )
{
 	// Calls Failure if err != noErr. If gAskFailure is TRUE, breaks into debugger
 	// before testing err. Failure may be simulated by setting err to the desired
 	// error code in the debugger.

	#ifdef forDebug
		if (gAskFailure)
			Debugger();
	#endif

	if (err != noErr) 
		Failure(err, iOSError);

}

pascal void FailMemError()
{
 	// Calls Failure if MemError() != noErr. If gAskFailure is TRUE, breaks into
 	// debugger before calling MemError(). Failures may then be simulated by
 	// modifying MemErr in the debugger.

	OSErr err = MemError();
	
	#ifdef forDebug
		if (gAskFailure)
			Debugger();
	#endif

	if (err != noErr)
		Failure(err, iMemoryError);

}

pascal void FailResError()
{
 	// Calls Failure if ResError() != noErr. If gAskFailure is TRUE, breaks into
 	// debugger before calling ResError(). Failures may then be simulated by
 	// modifying ResErr in the debugger.

	OSErr	err = ResError();
	
	#ifdef forDebug
		if (gAskFailure)
			Debugger();
	#endif
	
	if (err != noErr)
		Failure(err, iResourceError);
}

pascal void Failure (
	OSErr err,
	long message )
{	
	// Causes exception handling to begin by invoking the top handler in the
	// stack. When gBreakFailure is true, breaks into the the debugger before
	// invoking the handler. You can use the debugger to look up the call chain
	// to see where the failure originated.

	FailInfo	*handler;
	
	#ifdef forDebug
		if (gBreakFailure)
			Debugger();
	#endif

	if (gTopHandler) {
		gLastError = err;
		gLastMessage = message;
		
		handler = gTopHandler;
		gTopHandler = handler->next;
		
		ThrowHandler( handler);
		}

	else {

		// No handler is posted. All we can do is post an alert and quit.

		ErrorAlert(err, message);
		ExitToShell();
	}

}

pascal void Success()
{
	// Pops the top handler off the stack. Should be called after code that code
 	// try has successfully completed. The try/catch macros automatically call
 	// Success at the end of the try block.

	if (gTopHandler)
		gTopHandler = gTopHandler->next;

	else
		NoHandler();
	
}

pascal void RetryException (
	FailInfo *fi )
{
	// Pushes a handler back on the stack, resets the error and message to zero,
	// and invokes the handler. This has the effect of re-executing the try
	// block for the handler.

	gLastError = noErr;
	gLastMessage = 0;
	
	fi->next = gTopHandler;
	gTopHandler = fi;
	
	ThrowHandler(fi);
}

pascal long SpecifyMsg (
	short strListID,
	short strIndex )
{
	/*
	 	Utility routine to specify the message parameter for Failure or
	 	SetFailInfo.
	 	
		Use SpecifyMsg(strListID, strIndex), where strResID is
	 	the resource ID of a STR# resource, and
	 	strIndex is the index of the particular string in that
	 	STR# resource.
	 	
	 	Valid resource/index pairs are as follows:
	 	
	 		... (to be filled in)
	 		
	 	Change from TCL -- there�s no �standard� resource id.

	*/

	return ((long) strListID << 16L) + strIndex;
}

pascal void SetFailInfo (
	short newError,
	long newMessage )
{
	/*
	 	Always sets gLastError and conditionally sets gLastMessage.

	 	gLastMessage is modified if it has not previously been set.
	 	This allows handlers closest to where a failure occured to set gLastMessage
	 	without fear that some more general handler up the chain will stomp on it.
	*/

	gLastError = newError;
	
	if (gLastMessage == 0)
		gLastMessage = newMessage;

}

static void AsmThrowHandler()
{	

	asm		{
	

extern ThrowHandler:
		
		// Invokes the exception handler at the top of the stack. 

		addq.l	#4, sp							; pop return address
		move.l	(sp)+, d0						; get pointer to FailInfo
		bne		@1								; make sure it's not NIL
		jsr		NoHandler						; exit if NIL - doesn't return
		
	@1	move.l	d0, a0							; load FailInfo to a0
		clr.w	d0								; dispatch on FailInfo->handlerType
		move.b  FailInfo.handlerType(a0), d0
		add.w	d0,d0
		add.w	@2(d0.w),d0
		jmp		@2(d0.w)

	@2	dc.w	@_catchexception
		dc.w	@_try
		dc.w	@_catchfailure
		
	_catchexception:
		
		lea		FailInfo.regs(a0), a0   		; point to jmp_buf			
		movem.l	(a0),d3-d7/a1-a4/a6-a7			; restore registers. note that
												; return address is already popped into a1
												; and a7 points to return value for the
												; original CatchSignal
		move.w	gLastError, (sp)				; set error code
		jmp		(a1)

	_try:
		
		lea  	FailInfo.regs(a0), a0   		; point to jmp_buf			
		movem.l	(a0),d3-d7/a1-a4/a6-a7			; restore registers. note that
												; return address is already popped into a1
												; and a7 points to return value for the
												; original setjmp
		move.w	gLastError, d0					; set error code
		ext.l	d0								; extend in case of 4 byte int
		jmp		(a1)	

		
	_catchfailure:
		
		movem.l	FailInfo.regs(a0),d3-d7/a1-a4/a6-a7	
												; restore registers. note that
												; return address is already popped 
												; into a1
										

		tst.w	gLastError						; if gLastError != noErr then
												; call handler, else retry
		beq.s	@_retry
						
		move.w  gLastError, -(sp)				; push error and message
		move.l  gLastMessage, -(sp)
		
		tst.l   FailInfo.A6Link(a0)				; push static link if one
		beq.s	@nolink							; was provided
		move.l	FailInfo.A6Link(a0), -(sp)
		
	nolink:
		move.l	FailInfo.handlerProc(a0), a0
		jsr		(a0)							; call the handler
		
		move.w  gLastError, -(sp)				; push error and message
		move.l  gLastMessage, -(sp)
		
		jsr		Failure							; propagate failure
												; doesn't return
	
	_retry:

		jmp		(a1)							; return following CatchFailures

	}
}

static void AsmCatchFailures()
{
	asm	
	{
  extern CatchFailures:
			
		movea.l (sp)+, a1								; pop return address
		move.l	(sp)+, d0								; pop static link
		move.l	(sp)+, d1								; pop procptr
		movea.l (sp)+, a0								; pop failinfo
		
		movem.l	d3-d7/a1-a4/a6-a7, FailInfo.regs(a0) 	; save registers
		move.l	d0, FailInfo.A6Link(a0)					; save static link
		move.l	d1, FailInfo.handlerProc(a0)			; save procptr
														
		move.b  #1, FailInfo.fPropagate(a0)				; always propagates
			
														; set handler type
		move.b	#kCatchFailure, FailInfo.handlerType(a0)
					  
		move.l	gTopHandler, FailInfo.next(a0)			; link to next handler
		move.l	a0, gTopHandler							; stack top is new handler
				
		jmp		(a1)									; return to caller
	}

}

static pascal short AsmCatchException()
{
	/*
	 	Records the state of the calling routine in a FailInfo record and
	 	pushes this record onto the handler stack. Initially, zero is returned
	 	as the function result. If any call to Failure() occurs, the handler 
	 	will be popped, and the caller will be resumed and the return value 
	 	will be the error code passed to Failure().
	*/

	asm	
	{
	
  extern CatchException:

		movea.l (sp)+, a1								; pop return address
		movea.l (sp)+, a0								; pop handler
		movem.l	d3-d7/a1-a4/a6-a7, FailInfo.regs(a0) 	; save registers
		clr.l	FailInfo.A6Link(a0)						; no A6 link for CatchSignal
														; set fPropagate
		move.b  gDefaultPropagation, FailInfo.fPropagate(a0)
			
														; set handler type
		move.b	#kCatchException, FailInfo.handlerType(a0)
					  
		move.l	gTopHandler, FailInfo.next(a0)			; link to next handler
		move.l	a0, gTopHandler							; stack top is new handler
				
		clr.w	(sp)									; return noErr to caller
		jmp		(a1)		
	}

}
