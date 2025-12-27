/*
 * ZGraf - Assert.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#ifdef forDebug

	#define assert(condition)						\
		condition ? 0 : __assertion_failed( #condition, __FILE__, __LINE__)
	
	short __assertion_failed( char *conditionStr,  char *fileName, long lineNum);

#else
	
	#define assert(condition)

#endif
