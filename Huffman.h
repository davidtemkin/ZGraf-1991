/*
 * ZGraf - Huffman.h
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#define	SOUNDFILE	'FSSD'

#define	QTBITS		10		/*	Should always be less than 16!	*/
#define	VALUES		128
#define	DROPBITS	1
#define	ANDMASK		0x7F

typedef	struct	treenode
{
	char				value;
	
	int					codelen;
	int					code;

	long				freq;

	struct	treenode	*zeroptr;
	struct	treenode	*oneptr;
	int					typeflag;

}	treenode;
