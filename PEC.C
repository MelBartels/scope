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

/* PEC area... */

/* arrays hold values that show scope movement errors in tenths of arcseconds with PEC rotation
divided into PECSize units: positive values indicate excess CW movement;
for double or quad worms, set FullstepsPerPECArray to cover 2x or 4x worm rotation */

void InitPEC( void)
{
	AltPECFileNameCount = AzPECFileNameCount = 0;
	SetPECVars();
	ZeroAltPEC();
	ZeroAzPEC();
	LoadPEC();
	LastAutoAltPECSyncSignal = LastAutoAzPECSyncSignal = 0;
   LastPECIxOffset.A = LastPECIxOffset.Z = 0; 
}

void SetPECVars( void)
{
	MsPerPECArray = (long) Ms * FullstepsPerPECArray;
	MsPerPECIx = MsPerPECArray / (long) PECSize;
}

void ZeroAltPEC( void)
{
	for( Ix = 0; Ix < PECSize; Ix++)
		PECs[Ix].A = 0;
}

void ZeroAzPEC( void)
{
	for( Ix = 0; Ix < PECSize; Ix++)
		PECs[Ix].Z = 0;
}

void LoadPEC( void)
{
	Flag ArrayFlag = 0;
	char Index[ValueSize];

	Input = fopen( PECFile, "r");
	if( Input == NULL)
	{
		if( DisplayOpeningMsgs)
			printf( "\nDid not find %s, using flat PEC table", PECFile);
	}
	else
	{
		if( DisplayOpeningMsgs)
			printf( "\nFound %s", PECFile);

		fscanf( Input, "%s %s", Index, Value);
		while( !feof( Input))
		{
			Ix = atoi( Index);
			TenthsArcsec = atoi( Value);
			/* 1st time through indexes, ArrayFlag == 1; 2nd time ArrayFlag == 2 */
			if( Ix == 0)
				ArrayFlag++;
			if( ArrayFlag == 1)
				PECs[Ix].A = TenthsArcsec;
			else
				PECs[Ix].Z = TenthsArcsec;
			fscanf( Input, "%s %s", Index, Value);
		}
	}
	fclose( Input);
}

void SavePEC( char* Name)
{
	Output = fopen( Name, "w");
	if( Output == NULL)
		BadExit( strcat( "Could not open ", Name));

	for( PECIx.A = 0; PECIx.A < PECSize; PECIx.A++)
		fprintf( Output, "%d %d\n", PECIx.A, PECs[PECIx.A].A);
	fprintf( Output, "\n");
	for( PECIx.Z = 0; PECIx.Z < PECSize; PECIx.Z++)
		fprintf( Output, "%d %d\n", PECIx.Z, PECs[PECIx.Z].Z);

	fclose( Output);
}

void BuildAltPECFilename( void)
{
	int len = strlen( PECALT);

	strcpy( Name, PECALT);
	Name[len-1] = '0' + AltPECFileNameCount / 10;
	Name[len] = '0' + AltPECFileNameCount - (AltPECFileNameCount / 10) * 10;
	Name[len+1] = '\0';
	strcat( Name, DOT_TXT);
	AltPECFileNameCount++;
}

void BuildAzPECFilename( void)
{
	int len = strlen( PECAZ);

	strcpy( Name, PECAZ);
	Name[len-1] = '0' + AzPECFileNameCount / 10;
	Name[len] = '0' + AzPECFileNameCount - (AzPECFileNameCount / 10) * 10;
	Name[len+1] = '\0';
	strcat( Name, DOT_TXT);
	AzPECFileNameCount++;
}

/* this function sets an index offset so that the PEC Ix is matched to current rotor angle */
void SetPECIxOffsetA( void)
{
	PECIxOffset.A = (int) ((AccumMs.A%MsPerPECArray) / MsPerPECIx);
}

/* this function sets an index offset so that the PEC Ix is matched to current rotor angle */
void SetPECIxOffsetZ( void)
{
	PECIxOffset.Z = (int) ((AccumMs.Z%MsPerPECArray) / MsPerPECIx);
}

/* this function sets the PEC array indexes based on rotor angle */
void GetPECIx( void)
{
	struct AZDouble DoublePECIx;

	DoublePECIx.A = (double) (AccumMs.A%MsPerPECArray)/ MsPerPECIx - PECIxOffset.A;
	while( DoublePECIx.A < 0)
		DoublePECIx.A += PECSize;
	while( DoublePECIx.A >= PECSize)
		DoublePECIx.A -= PECSize;
	PECIx.A = (int) DoublePECIx.A;
	DecimalPECIx.A = DoublePECIx.A - PECIx.A;
	PECDir.A = Dir.A;
	if( PECDir.A == CW)
	{
		NextPECIx.A = PECIx.A + 1;
		if( NextPECIx.A >= PECSize)
			NextPECIx.A = 0;
	}
	else
	{
		NextPECIx.A = PECIx.A - 1;
		if( NextPECIx.A < 0)
			NextPECIx.A = PECSize- 1;
		DecimalPECIx.A = 1 - DecimalPECIx.A;
	}

	DoublePECIx.Z = (double) (AccumMs.Z%MsPerPECArray)/ MsPerPECIx - PECIxOffset.Z;
	while( DoublePECIx.Z < 0)
		DoublePECIx.Z += PECSize;
	while( DoublePECIx.Z >= PECSize)
		DoublePECIx.Z -= PECSize;
	PECIx.Z = (int) DoublePECIx.Z;
	DecimalPECIx.Z = DoublePECIx.Z - PECIx.Z;
	PECDir.Z = Dir.Z;
	if( PECDir.Z == CW)
	{
		NextPECIx.Z = PECIx.Z + 1;
		if( NextPECIx.Z >= PECSize)
			NextPECIx.Z = 0;
	}
	else
	{
		NextPECIx.Z = PECIx.Z - 1;
		if( NextPECIx.Z < 0)
			NextPECIx.Z = PECSize- 1;
		DecimalPECIx.Z = 1 - DecimalPECIx.Z;
	}
	/*
	gotoxy( 2, 2); printf( "%d %d %f %d", PECIx.A, NextPECIx.A, DecimalPECIx.A, PECDir.A);
	gotoxy( 2, 3); printf( "%d %d %f %d", PECIx.Z, NextPECIx.Z, DecimalPECIx.Z, PECDir.Z);
	*/
	PECToAdd.A = PECs[PECIx.A].A + DecimalPECIx.A * (PECs[NextPECIx.A].A - PECs[PECIx.A].A);
	PECToAdd.Z = PECs[PECIx.Z].Z + DecimalPECIx.Z * (PECs[NextPECIx.Z].Z - PECs[PECIx.Z].Z);
}

void DisplayPECgraphically( void)
{
	int altpecy;
	int azpecy;
	const int linecolor = RED;
	const int altcolor = GREEN;
	const int azcolor = BLUE;
	/* pixels for each PEC index */
	double xscale;
	/* pixels for each tenths of an arcsecond */
	double yscale;
	int maxh = 0;

	CloseMouseControl();
	InitGraphics();

	altpecy = 1 * maxy / 3;
	azpecy = 2 * maxy / 3;
	xscale = (double) maxx / (double) PECSize;

	/* set maxh to greatest deviation from zero for either axis */
	for( Ix = 0; Ix < PECSize; Ix++)
	{
		if( PECs[Ix].A > maxh)
			maxh = PECs[Ix].A;
		if( PECs[Ix].A < -maxh)
			maxh = -PECs[Ix].A;
		if( PECs[Ix].Z > maxh)
			maxh = PECs[Ix].Z;
		if( PECs[Ix].Z < -maxh)
			maxh = -PECs[Ix].Z;
	}
	if( maxh == 0)
		yscale = 1;
	else
		yscale = (double) (maxy/6) / (double) maxh;

	/* draw lines */
	for( Ix = 0; Ix <= maxx; Ix++)
		putpixel( Ix, altpecy, linecolor);
	for( Ix = 0; Ix <= maxx; Ix++)
		putpixel( Ix, azpecy, linecolor);

	/* draw vertical scales of 1 arcsecond */
	for( Ix = -10 * yscale; Ix < 10 * yscale; Ix++)
		putpixel( 0, altpecy + Ix, linecolor);
	for( Ix = -10 * yscale; Ix < 10 * yscale; Ix++)
		putpixel( 0, azpecy + Ix, linecolor);

	/* plot altitude PEC */
	for( Ix = 0; Ix < PECSize; Ix++)
		putpixel( Ix * xscale, altpecy - PECs[Ix].A * yscale, altcolor);

	/* plot azimuth PEC */
	for( Ix = 0; Ix < PECSize; Ix++)
		putpixel( Ix * xscale, azpecy - PECs[Ix].Z * yscale, azcolor);

	gotoxy( 1, 1);
	printf( "current periodic error correction values display\n");
	printf( "alt=upper curve, az=lower curve\n");
	printf( "press 'y' to save to pec.dat file, any other key to exit");
	GetResponseWithLX200Check();
	if( Response == 'Y' || Response == 'y' || Response == Return)
		SavePEC( PECFile);

	CloseGraphics();
	_setcursortype( _NOCURSOR);
	InitMouseControl();
}

Flag AveragePECAnalysisFiles( Flag axis)
{
	int pecy;
	int analysisy;
	int resulty;
	const int linecolor = RED;
	const int peccolor = WHITE;
	const int analysiscolor = YELLOW;
	const int resultcolor = GREEN;
	/* pixels for each PEC index */
	double xscale;
	/* pixels for each tenths of an arcsecond */
	double yscale;
	int maxh = 0;
	char Index[ValueSize];
	int FilesRead = 0.;
	/* declare a pointer to type of int */
	int* P;


	if( axis == azaxis)
	{
		strcpy( Fname, PECAZ);
		strcat( Fname, "*");
		strcat( Fname, DOT_TXT);
	}
	else
	{
		strcpy( Fname, PECALT);
		strcat( Fname, "*");
		strcat( Fname, DOT_TXT);
	}

	/* assign to P the address of the beginning of the allocated memory */
	P = (int*) malloc( PECSize * sizeof( int));
	/* zero out array */
	for( Ix = 0; Ix < PECSize; Ix++)
		P[Ix] = 0;

	FindFirst = True;
	FoundFile = True;

	WriteWindow( MsgFrame);
	while( FoundFile)
	{
		if( FindFirst)
		{
			FindFirst = False;
			/* returns 0 if successful */
			FoundFile = !findfirst( Fname, &Ffblk, 0);
		}
		else
			FoundFile = !findnext( &Ffblk);
		if( FoundFile)
		{
			for( Ix = MsgFrame.Left + 3; Ix < MsgFrame.Right - 1; Ix++)
			{
				gotoxy( Ix, MsgFrame.Top + 3);
				printf( " ");
			}
			gotoxy( MsgFrame.Left + 3, MsgFrame.Top + 3);
			printf( "Use file %s ? (y/n) ", Ffblk.ff_name);
			GetResponseWithLX200Check();
			printf( "%c", Response);
			if( Response == 'Y' || Response == 'y' || Response == Return)
			{
				Input = fopen( Ffblk.ff_name, "r");
				if( Input == NULL)
					BadExit( strcat( "Could not open ", Ffblk.ff_name));
				else
				{
					FilesRead++;
					fscanf( Input, "%s %s", Index, Value);
					while( !feof( Input))
					{
						/* int* P and P[] are equivalent;
						P[x] accesses element x of P, so *(P+x) == P[x];
						P == &P[0] so *P == P[0] */
						P[atoi( Index)] += atoi( Value);
						fscanf( Input, "%s %s", Index, Value);
					}
				}
				fclose( Input);
			}
		}
	}
	RemoveWindow( MsgFrame);

	if( FilesRead)
	{
		for( Ix = 0; Ix < PECSize; Ix++)
			P[Ix] /= FilesRead;

		CloseMouseControl();
		InitGraphics();

		pecy = 1 * maxy / 4;
		analysisy = 2 * maxy / 4;
		resulty = 3 * maxy / 4;
		xscale = (double) maxx / (double) PECSize;

		/* draw lines */
		for( Ix = 0; Ix <= maxx; Ix++)
			putpixel( Ix, pecy, linecolor);
		for( Ix = 0; Ix <= maxx; Ix++)
			putpixel( Ix, analysisy, linecolor);
		for( Ix = 0; Ix <= maxx; Ix++)
			putpixel( Ix, resulty, linecolor);

		/* set maxh to greatest deviation from zero for either axis */
		for( Ix = 0; Ix < PECSize; Ix++)
		{
			if( P[Ix] > maxh)
				maxh = P[Ix];
			if( P[Ix] < -maxh)
				maxh = -P[Ix];
			if( axis == azaxis)
			{
				if( PECs[Ix].Z > maxh)
					maxh = PECs[Ix].Z;
				if( PECs[Ix].Z < -maxh)
					maxh = -PECs[Ix].Z;
				if( PECs[Ix].Z + P[Ix] > maxh)
					maxh = PECs[Ix].Z + P[Ix];
				if( PECs[Ix].Z + P[Ix] < -maxh)
					maxh = -(PECs[Ix].Z + P[Ix]);
			}
			else
			{
				if( PECs[Ix].A > maxh)
					maxh = PECs[Ix].A;
				if( PECs[Ix].A < -maxh)
					maxh = -PECs[Ix].A;
				if( PECs[Ix].A + P[Ix] > maxh)
					maxh = PECs[Ix].A + P[Ix];
				if( PECs[Ix].A + P[Ix] < -maxh)
					maxh = -(PECs[Ix].A + P[Ix]);
			}
		}
		if( maxh == 0)
			yscale = 1;
		else
			yscale = (double) (maxy/8) / (double) maxh;

		/* draw vertical scales of 1 arcsecond */
		for( Ix = -10 * yscale; Ix < 10 * yscale; Ix++)
			putpixel( 0, pecy + Ix, linecolor);
		for( Ix = -10 * yscale; Ix < 10 * yscale; Ix++)
			putpixel( 0, analysisy + Ix, linecolor);
		for( Ix = -10 * yscale; Ix < 10 * yscale; Ix++)
			putpixel( 0, resulty + Ix, linecolor);

		/* plot original PEC */
		if( axis == azaxis)
			for( Ix = 0; Ix < PECSize; Ix++)
				putpixel( Ix * xscale, pecy - PECs[Ix].Z * yscale, peccolor);
		else
			for( Ix = 0; Ix < PECSize; Ix++)
				putpixel( Ix * xscale, pecy - PECs[Ix].A * yscale, peccolor);

		/* plot averaged analysis curve */
			for( Ix = 0; Ix < PECSize; Ix++)
				putpixel( Ix * xscale, analysisy - P[Ix] * yscale, analysiscolor);

		/* plot possible updated PEC */
		if( axis == azaxis)
			for( Ix = 0; Ix < PECSize; Ix++)
				putpixel( Ix * xscale, resulty - (PECs[Ix].Z + P[Ix]) * yscale,
				resultcolor);
		else
			for( Ix = 0; Ix < PECSize; Ix++)
				putpixel( Ix * xscale, resulty -(PECs[Ix].A + P[Ix]) * yscale,
				resultcolor);

		gotoxy( 1, 1);
		printf( "adding %d averaged pec", FilesRead);
		if( axis == azaxis)
			printf( "az");
		else
			printf( "alt");
		printf( "*.txt files to pec.dat\n");
		printf( "original pec color = white, average of pec files = yellow,\n");
		printf( "pec + average color = green\n");
		printf( "press 'y' to do so, any other key to abort");
		GetResponseWithLX200Check();
		if( Response == 'Y' || Response == 'y' || Response == Return)
		{
			/* positive PEC indicates excess CW movement; if scope is moving clockwise, Current. must
			be increased by adding PEC so that (target Current. - corrected Current.) is a smaller
			value resulting in less clockwise motion;
			positive P or guide values mean the same: so add P to PEC array */
			if( axis == azaxis)
				for( Ix = 0; Ix < PECSize; Ix++)
					PECs[Ix].Z += P[Ix];
			else
				for( Ix = 0; Ix < PECSize; Ix++)
					PECs[Ix].A += P[Ix];
			SavePEC( PECFile);
		}
		CloseGraphics();
		_setcursortype( _NOCURSOR);
		InitMouseControl();

		free( P);
		return True;
	}
	else
	{
		free( P);
		return False;
	}
}

void MedianSmoothPEC( int Num)
{
	int IxA, IxB;
	int SortIx, SortIxB, SortIxC;
	int* SortArray;
	int* P;
	int Tot, Avg;

	/* round up to odd number */
	if( Num%2)
		;
	else
		Num++;

	/* Num size + 1 for swap */
	SortArray = (int*) malloc( (Num + 1) * sizeof( int));
	if( SortArray == NULL)
		BadExit( "Problem with malloc of SortArray in MedianSmoothPEC");

	P = (int*) malloc( PECSize * sizeof( int));
	if( P == NULL)
		BadExit( "Problem with malloc of P in MedianSmoothPEC");

	/* do altitude... */

	/* fill array */
	for( Ix = 0; Ix < PECSize; Ix++)
		P[Ix] = PECs[Ix].A;
	/* Ix will be the base index to work from */
	for( Ix = 0; Ix < PECSize; Ix++)
	{
		Tot = 0;
		SortIx = Num/2;
		/* so that SortArray mirrors PEC values with middle of SortArray being the base 'Ix' value of
		the PEC array */
		for( IxA = 0; IxA <= Num/2; IxA++)
		{
			IxB = Ix + IxA;
			if( IxB >= PECSize)
				IxB -= PECSize;
			Tot += P[IxB];
			SortArray[SortIx++] = P[IxB];
		}
		SortIx = 0;
		for( IxA = -Num/2; IxA < 0; IxA++)
		{
			IxB = Ix + IxA;
			if( IxB < 0)
				IxB += PECSize;
			Tot += P[IxB];
			SortArray[SortIx++] = P[IxB];
		}
		/* bubble sort SortArray */
		for( SortIxC = Num - 1; SortIxC > 0; SortIxC--)
			for( SortIxB = 1; SortIxB <= SortIxC; SortIxB++)
			{
				SortIx = SortIxB - 1;
				if( SortArray[SortIx] > SortArray[SortIxB])
				{
					SortArray[Num] = SortArray[SortIxB];
					SortArray[SortIxB] = SortArray[SortIx];
					SortArray[SortIx] = SortArray[Num];
				}
			}
		Avg = Tot / Num;
		/* update PEC array with new value */
		PECs[Ix].A = Avg;
		/* ignore averaging and go with pure median smoothing for now */
		PECs[Ix].A = SortArray[Num/2];
	}

	/* do azimuth... */

	/* fill array */
	for( Ix = 0; Ix < PECSize; Ix++)
		P[Ix] = PECs[Ix].Z;
	/* Ix will be the base index to work from */
	for( Ix = 0; Ix < PECSize; Ix++)
	{
		Tot = 0;
		SortIx = Num/2;
		/* so that SortArray mirrors PEC values with middle of SortArray being the base 'Ix' value of
		the PEC array */
		for( IxA = 0; IxA <= Num/2; IxA++)
		{
			IxB = Ix + IxA;
			if( IxB >= PECSize)
				IxB -= PECSize;
			Tot += P[IxB];
			SortArray[SortIx++] = P[IxB];
		}
		SortIx = 0;
		for( IxA = -Num/2; IxA < 0; IxA++)
		{
			IxB = Ix + IxA;
			if( IxB < 0)
				IxB += PECSize;
			Tot += P[IxB];
			SortArray[SortIx++] = P[IxB];
		}
		/* bubble sort SortArray */
		for( SortIxC = Num - 1; SortIxC > 0; SortIxC--)
			for( SortIxB = 1; SortIxB <= SortIxC; SortIxB++)
			{
				SortIx = SortIxB - 1;
				if( SortArray[SortIx] > SortArray[SortIxB])
				{
					SortArray[Num] = SortArray[SortIxB];
					SortArray[SortIxB] = SortArray[SortIx];
					SortArray[SortIx] = SortArray[Num];
				}
			}
		Avg = Tot / Num;
		/* update PEC array with new value */
		PECs[Ix].Z = Avg;
		/* ignore averaging and go with pure median smoothing for now */
		PECs[Ix].Z = SortArray[Num/2];
	}

	free( SortArray);
	free( P);
}

void DisplayAltPEC( void)
{
	sprintf( StrBuf, "%03d:%03.0f ", PECIx.A, PECToAdd.A);
	VidMemXY = DisplayXY[DisplayPEC];
	WriteStrBufToScreen_f_ptr();
}

void DisplayAzPEC( void)
{
	sprintf( StrBuf, "%03d:%03.0f ", PECIx.Z, PECToAdd.Z);
	VidMemXY.Y = DisplayXY[DisplayPEC].Y;
	VidMemXY.X = DisplayXY[DisplayPEC].X + 8;
	WriteStrBufToScreen_f_ptr();
}

