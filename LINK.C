/*
copyright 1990 through 2007 by Mel Bartels

	 This file is part of scope.exe the stepper version.

	 Scope.exe is free software; you can redistribute it and/or modify
	 it under the terms of the GNU General Public License as published by
	 the Free Software Foundation; either version 2 of the License, or
	 (at your option) any later version.

	 Scope.exe is distributed in the hope that it will be useful,
	 but WITHOUT ANY WARRANTY; without even the implied warranty of
	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	 GNU General Public License for more details.

	 You should have received a copy of the GNU General Public License
	 along with scope.exe; if not, write to the Free Software
	 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <math.h>
#include <time.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <values.h>
#include <string.h>
#include <ctype.h>
#include <dir.h>
#include <process.h>
#include <graphics.h>
#include "header.h"

void AddLinkPos( struct LINK_POSITION* LinkPos)
{
	if( FirstLinkPos == NULL)
	{
		LinkPos->PrevLinkPos = NULL;
		LinkPos->NextLinkPos = NULL;
		FirstLinkPos = LinkPos;
		LastLinkPos = LinkPos;
	}
	else
	{
		LinkPos->PrevLinkPos = LastLinkPos;
		LinkPos->NextLinkPos = NULL;
		/* set next to last LinkPos in linked list to current LinkPos */
		LastLinkPos->NextLinkPos = LinkPos;
		/* current LinkPos also LastLinkPos */
		LastLinkPos = LinkPos;
	}
}

void FreeAllLinkPos( void)
{
	struct LINK_POSITION* LinkPos;

	LinkPos = LastLinkPos;
	while( LinkPos != NULL)
	{
		/* set LastLinkPos to next to last in linked list */
		LastLinkPos = LinkPos->PrevLinkPos;
		/* now free last in linked list */
		free( LinkPos);
		LinkPos = LastLinkPos;
	}
	FirstLinkPos = NULL;
	LinkPosCount = 0;
}

struct LINK_POSITION* InitLinkPos( void)
{
	struct LINK_POSITION* LinkPos;

	LinkPos = (struct LINK_POSITION*) malloc( sizeof (struct LINK_POSITION));
	if( LinkPos == NULL)
		BadExit( "Problem with malloc of LinkPos in InitLinkPos");
	LinkPos->PrevLinkPos = NULL;
	LinkPos->NextLinkPos = NULL;

	return LinkPos;
}

/* uses selection sort: smallest of array to the end is moved to the front, then smallest of array+1
to the end is moved to the front of array+1, and so forth */

void SortLinkPos (char* Key)
{
	struct LINK_POSITION* PtrA;
	struct LINK_POSITION* PtrB;
	struct LINK_POSITION* PtrInsert;

	if( strcmpi( Key, "Ra_Ascend") != 0 &&
		 strcmpi( Key, "Dec_Ascend") != 0 &&
		 strcmpi( Key, "Alt_Ascend") != 0 &&
		 strcmpi( Key, "Az_Ascend") != 0 &&
		 strcmpi( Key, "SidT_Ascend") != 0 &&
		 strcmpi( Key, "Ra_Descend") != 0 &&
		 strcmpi( Key, "Dec_Descend") != 0 &&
		 strcmpi( Key, "Alt_Descend") != 0 &&
		 strcmpi( Key, "Az_Descend") != 0 &&
		 strcmpi( Key, "SidT_Descend") != 0)
		BadExit( "Bad Key in SortLinkPos()");

	PtrA = FirstLinkPos;
	/* while something in the linked list and while pointer to the start of the array under
	consideration points to something */
	while( PtrA != NULL && PtrA->NextLinkPos != NULL)
	{
		/* mark place to possibly insert */
		PtrInsert = PtrA;
		/* start comparison of PtrA to remainder of array with next linked item */
		PtrB = PtrA->NextLinkPos;
		/* compare to each remaining linked item */
		while( PtrB != NULL)
		{
			if( (strcmpi( Key, "Ra_Ascend") == 0) && (PtrB->P.Ra < PtrInsert->P.Ra))
				PtrInsert = PtrB;
			if( (strcmpi( Key, "Dec_Ascend") == 0) && (PtrB->P.Dec < PtrInsert->P.Dec))
				PtrInsert = PtrB;
			if( (strcmpi( Key, "Alt_Ascend") == 0) && (PtrB->P.Alt < PtrInsert->P.Alt))
				PtrInsert = PtrB;
			if( (strcmpi( Key, "Az_Ascend") == 0) && (PtrB->P.Az < PtrInsert->P.Az))
				PtrInsert = PtrB;
			if( (strcmpi( Key, "SidT_Ascend") == 0) && (PtrB->P.SidT < PtrInsert->P.SidT))
				PtrInsert = PtrB;
			if( (strcmpi( Key, "Ra_Descend") == 0) && (PtrB->P.Ra > PtrInsert->P.Ra))
				PtrInsert = PtrB;
			if( (strcmpi( Key, "Dec_Descend") == 0) && (PtrB->P.Dec > PtrInsert->P.Dec))
				PtrInsert = PtrB;
			if( (strcmpi( Key, "Alt_Descend") == 0) && (PtrB->P.Alt > PtrInsert->P.Alt))
				PtrInsert = PtrB;
			if( (strcmpi( Key, "Az_Descend") == 0) && (PtrB->P.Az > PtrInsert->P.Az))
				PtrInsert = PtrB;
			if( (strcmpi( Key, "SidT_Descend") == 0) && (PtrB->P.SidT > PtrInsert->P.SidT))
				PtrInsert = PtrB;
			PtrB = PtrB->NextLinkPos;
		}
		/* if comparison said to insert linked item */
		if( PtrInsert != PtrA)
		{
			/* remove PtrInsert from linked list... */
			/* handle forward link from previous item in linked list */
			PtrInsert->PrevLinkPos->NextLinkPos = PtrInsert->NextLinkPos;
			if( PtrInsert->NextLinkPos == NULL)
				/* pointer to last item in linked list points to previous item */
				LastLinkPos = PtrInsert->PrevLinkPos;
			else
				/* handle previous link from next item in linked list */
				PtrInsert->NextLinkPos->PrevLinkPos = PtrInsert->PrevLinkPos;
			/* insert PtrInsert before PtrA... */
			if( PtrA->PrevLinkPos == NULL)
				/* pointer to first item points to newly inserted item */
				FirstLinkPos = PtrInsert;
			else
				/* previous item forward pointer points to newly inserted item */
				PtrA->PrevLinkPos->NextLinkPos = PtrInsert;
			/* inserted item previous pointer points to where PtrA's previous used
			to point */
			PtrInsert->PrevLinkPos = PtrA->PrevLinkPos;
			/* point PtrA back to newly inserted item */
			PtrA->PrevLinkPos = PtrInsert;
			/* point newly inserted item forward pointer to PtrA */
			PtrInsert->NextLinkPos = PtrA;
			/* set PtrA to newly inserted linked item */
			PtrA = PtrInsert;
		}
		PtrA = PtrA->NextLinkPos;
	}
}

void DisplayLinkPos( void)
{
	printf( "\ndisplaying linked list Positions:");

	CurrentLinkPos = FirstLinkPos;
	while( CurrentLinkPos != NULL)
	{
		ShowCoord( &CurrentLinkPos->P);
		CurrentLinkPos = CurrentLinkPos->NextLinkPos;
	}
	printf( "\nend of linked list\n");
}

void AddLinkPMC( struct LINK_PMC* LinkPMC)
{
	if( FirstLinkPMC == NULL)
	{
		LinkPMC->PrevLinkPMC = NULL;
		LinkPMC->NextLinkPMC = NULL;
		FirstLinkPMC = LinkPMC;
		LastLinkPMC = LinkPMC;
	}
	else
	{
		LinkPMC->PrevLinkPMC = LastLinkPMC;
		LinkPMC->NextLinkPMC = NULL;
		/* set next to last LinkPMC in linked list to current LinkPMC */
		LastLinkPMC->NextLinkPMC = LinkPMC;
		/* current LinkPMC also LastLinkPMC */
		LastLinkPMC = LinkPMC;
	}
}

void FreeAllLinkPMC( void)
{
	struct LINK_PMC* LinkPMC;

	LinkPMC = LastLinkPMC;
	while( LinkPMC != NULL)
	{
		/* set LastLinkPMC to next to last in linked list */
		LastLinkPMC = LinkPMC->PrevLinkPMC;
		/* now free last in linked list */
		free( LinkPMC);
		LinkPMC = LastLinkPMC;
	}
	FirstLinkPMC = NULL;
   LinkPMCCount = 0;
}

struct LINK_PMC* InitLinkPMC( void)
{
	struct LINK_PMC* LinkPMC;

	LinkPMC = (struct LINK_PMC*) malloc( sizeof (struct LINK_PMC));
	if( LinkPMC == NULL)
		BadExit( "Problem with malloc of LinkPMC in InitLinkPMC");
	LinkPMC->PrevLinkPMC = NULL;
	LinkPMC->NextLinkPMC = NULL;

	return LinkPMC;
}

void AddLinkFname( struct LINK_FNAME* LinkFname)
{
	if( FirstLinkFname == NULL)
	{
		LinkFname->PrevLinkFname = NULL;
		LinkFname->NextLinkFname = NULL;
		FirstLinkFname = LinkFname;
		LastLinkFname = LinkFname;
	}
	else
	{
		LinkFname->PrevLinkFname = LastLinkFname;
		LinkFname->NextLinkFname = NULL;
		/* set next to last LinkFname in linked list to current LinkFname */
		LastLinkFname->NextLinkFname = LinkFname;
		/* current LinkFname also LastLinkFname */
		LastLinkFname = LinkFname;
	}
}

void FreeAllLinkFname( void)
{
	struct LINK_FNAME* LinkFname;

	LinkFname = LastLinkFname;
	while( LinkFname != NULL)
	{
		/* set LastLinkFname to next to last in linked list */
		LastLinkFname = LinkFname->PrevLinkFname;
		/* now free last in linked list */
		free( LinkFname);
		LinkFname = LastLinkFname;
	}
	FirstLinkFname = NULL;
}

struct LINK_FNAME* InitLinkFname( void)
{
	struct LINK_FNAME* LinkFname;

	LinkFname = (struct LINK_FNAME*) malloc( sizeof (struct LINK_FNAME));
	if( LinkFname == NULL)
		BadExit( "Problem with malloc of LinkFname in InitLinkFname");
	LinkFname->PrevLinkFname = NULL;
	LinkFname->NextLinkFname = NULL;

	return LinkFname;
}

/* uses selection sort: smallest of array to the end is moved to the front, then smallest of array+1
to the end is moved to the front of array+1, and so forth */

void SortLinkFname( void)
{
	struct LINK_FNAME* PtrA;
	struct LINK_FNAME* PtrB;
	struct LINK_FNAME* PtrInsert;

	PtrA = FirstLinkFname;
	/* while something in the linked list and while pointer to the start of the array under
	consideration points to something */
	while( PtrA != NULL && PtrA->NextLinkFname != NULL)
	{
		/* mark place to possibly insert */
		PtrInsert = PtrA;
		/* start comparison of PtrA to remainder of array with next linked item */
		PtrB = PtrA->NextLinkFname;
		/* compare to each remaining linked item */
		while( PtrB != NULL)
		{
			if( PtrB->FNameType < PtrInsert->FNameType ||
			PtrB->FNameType == PtrInsert->FNameType && strcmpi( PtrB->Name, PtrInsert->Name) < 0)
				PtrInsert = PtrB;
			PtrB = PtrB->NextLinkFname;
		}
		/* if comparison says to insert linked item */
		if( PtrInsert != PtrA)
		{
			/* remove PtrInsert from linked list... */
			/* handle forward link from previous item in linked list */
			PtrInsert->PrevLinkFname->NextLinkFname = PtrInsert->NextLinkFname;
			if( PtrInsert->NextLinkFname == NULL)
				/* pointer to last item in linked list points to previous item */
				LastLinkFname = PtrInsert->PrevLinkFname;
			else
				/* handle previous link from next item in linked list */
				PtrInsert->NextLinkFname->PrevLinkFname = PtrInsert->PrevLinkFname;
			/* insert PtrInsert before PtrA... */
			if( PtrA->PrevLinkFname == NULL)
				/* pointer to first item points to newly inserted item */
				FirstLinkFname = PtrInsert;
			else
				/* previous item forward pointer points to newly inserted item */
				PtrA->PrevLinkFname->NextLinkFname = PtrInsert;
			/* inserted item previous pointer points to where PtrA's previous used
			to point */
			PtrInsert->PrevLinkFname = PtrA->PrevLinkFname;
			/* point PtrA back to newly inserted item */
			PtrA->PrevLinkFname = PtrInsert;
			/* point newly inserted item forward pointer to PtrA */
			PtrInsert->NextLinkFname = PtrA;
			/* set PtrA to newly inserted linked item */
			PtrA = PtrInsert;
		}
		PtrA = PtrA->NextLinkFname;
	}
}

void DisplayLinkFnames( void)
{
	printf( "\ndisplaying linked list Fname:");

	CurrentLinkFname = FirstLinkFname;
	while( CurrentLinkFname != NULL)
	{
		printf( "\n%s", CurrentLinkFname->Name);
		CurrentLinkFname = CurrentLinkFname->NextLinkFname;
	}
	printf( "\nend of linked list\n");
}

void LoadFnames( char* Name)
{
	struct LINK_FNAME* LinkFname;

	if( FirstLinkFname != NULL)
		FreeAllLinkFname();

	/* get directory names */
	FindFirst = True;
	GetDir();
	while( FoundFile)
	{
		LinkFname = InitLinkFname();
		strcpy( LinkFname->Name, Ffblk.ff_name);
		LinkFname->FNameType = NameIsADirectory;
		AddLinkFname( LinkFname);
		GetDir();
	}

	/* get file names */
	FindFirst = True;
	GetFileName( Name);
	while( FoundFile)
	{
		LinkFname = InitLinkFname();
		strcpy( LinkFname->Name, Ffblk.ff_name);
		LinkFname->FNameType = NameIsAFile;
		AddLinkFname( LinkFname);
		GetFileName( Name);
	}
}

void AddScroll( struct SCROLL* Scroll)
{
	if( FirstScroll == NULL)
	{
		Scroll->PrevScroll = NULL;
		Scroll->NextScroll = NULL;
		FirstScroll = Scroll;
		LastScroll = Scroll;
	}
	else
	{
		Scroll->PrevScroll = LastScroll;
		Scroll->NextScroll = NULL;
		/* set next to last Scroll in linked list to current Scroll */
		LastScroll->NextScroll = Scroll;
		/* current Scroll also LastScroll */
		LastScroll = Scroll;
	}
   TotalScrollCount++;
}

void FreeAllScroll( void)
{
	struct SCROLL* Scroll;

	Scroll = LastScroll;
	while( Scroll != NULL)
	{
		/* set LastScroll to next to last in linked list */
		LastScroll = Scroll->PrevScroll;
		/* if necessary, free the linked Position */
		if( Scroll->P != NULL)
			free( Scroll->P);
		/* if necessary, free the linked AZLong */
		if( Scroll->L != NULL)
			free( Scroll->L);
		/* now free last in linked list */
		free( Scroll);
		Scroll = LastScroll;
	}
	FirstScroll = NULL;
	ScrollLoaded = No;
	TotalScrollCount = 0;
   CurrentScrollCount = 0;
}

struct SCROLL* InitScroll( Flag InitPosition, Flag InitLong)
{
	struct SCROLL* Scroll;

	Scroll = (struct SCROLL*) malloc( sizeof (struct SCROLL));
	if( Scroll == NULL)
		BadExit( "Problem with malloc of Scroll");

	Scroll->ScrollType = no_type;
	Scroll->PrevScroll = NULL;
	Scroll->NextScroll = NULL;
	if( InitPosition)
		Scroll->P = InitScrollPosition();
	else
		Scroll->P = NULL;
	if( InitLong)
		Scroll->L = InitScrollLong();
	else
		Scroll->L = NULL;
	Scroll->Sec = 0;
	strcpy( Scroll->Note, NameBlanks);

	return Scroll;
}

struct Position* InitScrollPosition( void)
{
	struct Position* Pptr;

	Pptr = (struct Position*) malloc( sizeof (struct Position));
	if( Pptr == NULL)
		BadExit( "Problem with malloc of Pptr");
	Pptr->Ra = 0;
	GetHMSH( Pptr->Ra, &Pptr->RaHMSH);
	Pptr->Dec = 0;
	GetDMS( Pptr->Dec, &Pptr->DecDMS);

	return Pptr;
}

struct AZLong* InitScrollLong( void)
{
	struct AZLong* Lptr;

	Lptr = (struct AZLong*) malloc( sizeof (struct AZLong));
	if( Lptr == NULL)
		BadExit( "Problem with malloc of Lptr");
	Lptr->A = Lptr->Z = 0;

	return Lptr;
}

void AddRaDecInit( struct RA_DEC_INIT* RaDecInit)
{
	if( FirstRaDecInit == NULL)
	{
		RaDecInit->PrevRaDecInit = NULL;
		RaDecInit->NextRaDecInit = NULL;
		FirstRaDecInit = RaDecInit;
		LastRaDecInit = RaDecInit;
	}
	else
	{
		RaDecInit->PrevRaDecInit = LastRaDecInit;
		RaDecInit->NextRaDecInit = NULL;
		/* set next to last RaDecInit in linked list to current RaDecInit */
		LastRaDecInit->NextRaDecInit = RaDecInit;
		/* current RaDecInit also LastRaDecInit */
		LastRaDecInit = RaDecInit;
	}
}

void FreeAllRaDecInit( void)
{
	struct RA_DEC_INIT* RaDecInit;

	RaDecInit = LastRaDecInit;
	while( RaDecInit != NULL)
	{
		/* set LastRaDecInit to next to last in linked list */
		LastRaDecInit = RaDecInit->PrevRaDecInit;
		/* now free last in linked list */
		free( RaDecInit);
		RaDecInit = LastRaDecInit;
	}
	FirstRaDecInit = NULL;
}

struct RA_DEC_INIT* InitRaDecInit( void)
{
	struct RA_DEC_INIT* RaDecInit;

	RaDecInit = (struct RA_DEC_INIT*) malloc( sizeof (struct RA_DEC_INIT));
	if( RaDecInit == NULL)
		BadExit( "Problem with malloc of RaDecInit in InitRaDecInit");
	RaDecInit->PrevRaDecInit = NULL;
	RaDecInit->NextRaDecInit = NULL;

	return RaDecInit;
}

