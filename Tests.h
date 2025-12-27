/*
 * ZGraf - Tests.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#define _H_Test

extern long HLineTest(Boolean qd, Boolean shade);
extern long VLineTest(Boolean qd, Boolean shade);
extern long RectTest(Boolean qd, short size);
extern long CopyTest(Boolean qd);
extern void VerticalBenchmark(void);
extern void ScreenBenchmark(void);
extern void ScreenCopy(void);
extern void FullBenchmark(short onScreen);
extern void PixelTest(void);
extern void Scroller(short increment);
