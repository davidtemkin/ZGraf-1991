/*
 * ZGraf - DataFiles.c
 *
 * Copyright (c) 1991-1994 David Temkin
 * SPDX-License-Identifier: MIT
 */

#include "GameHeaders.h"

#pragma options(!check_ptrs)

#include "Shuddup.h"

Handle	ReadDataFiles(ftype)
long	ftype;
{
	register	Handle			fdata;
	register	Handle			sinfo;
	register	long			datasize;
				long			len;
	register	int				numfiles,i,numread;
				int				ref;
				VolumeParam		volupram;
				FileParam		filepram;
				char			fname[256];
	
	sinfo=GetResource(SKRESTYPE,SKSTABLE);
	fdata=NewHandle(0);
	datasize=0;

	volupram.ioCompletion=0;
	volupram.ioNamePtr=0;
	volupram.ioVRefNum=0;
	volupram.ioVolIndex=0;
	
	PBGetVInfo(&volupram,0);

	numfiles=volupram.ioVNmFls;

	filepram.ioCompletion=0;
	filepram.ioVRefNum=0;
	filepram.ioFVersNum=0;
	filepram.ioNamePtr=(void *)fname;
	
	numread=0;
	for(i=1;i<=numfiles;i++)
	{	filepram.ioFDirIndex=i;
		if(PBGetFInfo(&filepram,0)==0)
		{	if(filepram.ioFlFndrInfo.fdType==ftype)
			{	if(FSOpen(fname,0,&ref)==0)
				{	if(GetEOF(ref,&len)==0)
					{	SetHandleSize(fdata,datasize+len);
						if(!MemErr)
						{	HLock(fdata);
							FSRead(ref,&len,datasize+*fdata);
							datasize+=len;
							HUnlock(fdata);
							
							numread++;
							SetHandleSize(sinfo,numread*sizeof(long));
							if(!MemErr)
							{	((long *)(*sinfo))[numread-1]=len;
							}
						}
					}
					FSClose(ref);
				}
			}
		}
	}
	if(datasize==0)
	{	datasize=10;
	}
	SetHandleSize(fdata,datasize);
	ChangedResource(sinfo);
	WriteResource(sinfo);
	return fdata;
}