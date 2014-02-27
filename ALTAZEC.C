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

void InitAltAzEC( void)
{
	MallocAltAzEC();
	LoadAltAzECFile();
}

void CloseAltAzEC( void)
{
	free( AltAzEC);
}

void MallocAltAzEC( void)
{
	AltAzEC = (float*) malloc( (ECSize) * sizeof( float));
	if( AltAzEC == NULL)
		BadExit( "Problem with malloc of AltAzEC in convert module");
}

Flag LoadAltAzECFile( void)
{
	char Index[ValueSize];

	Input = fopen( AltAzECFile, "r");
	if( Input == NULL)
	{
		for( Ix = 0; Ix < ECSize; Ix++)
			AltAzEC[Ix] = 0;
		if( DisplayOpeningMsgs)
			printf( "\ndid not find ALTAZEC.DAT file, creating new one with zeroed values");
		SaveAltAzECFile();
		return False;
	}
	else
	{
		if( DisplayOpeningMsgs)
			printf( "\nfound ALTAZEC.DAT");
		fscanf( Input, "%s %s", Index, Value);
		while( !feof( Input))
		{
			Ix = atoi( Index)/EC_Resolution_Deg;
			if( Ix >= ECSize || Ix < 0)
				BadExit( "bad Ix in LoadAltAzECFile");
			AltAzEC[Ix] = atof( Value);
			fscanf( Input, "%s %s", Index, Value);
		}
	}
	fclose( Input);
	return True;
}

void SaveAltAzECFile( void)
{
	Output = fopen( AltAzECFile, "w");
	if( Output == NULL)
		BadExit( strcat( "Could not open ", AltAzECFile));

	for( Ix = 0; Ix < ECSize; Ix++)
		fprintf( Output, "%d %f\n", Ix*EC_Resolution_Deg, AltAzEC[Ix]);

	fclose( Output);
}

void SetAltAzECValue( void)
{
	int IxA, IxB;
	double DblIx, Frac;

	DblIx = Current.Az*RadToDeg/EC_Resolution_Deg + .5;
	while( DblIx >= ECSize)
		DblIx -= ECSize;
	while( DblIx < 0)
		DblIx += ECSize;
	IxA = DblIx;
	if( IxA < 0 || IxA >= ECSize)
		BadExit( "Bad IxA in SetAltAzECValue");
	IxB = IxA + 1;
	if( IxB >= ECSize)
		IxB = 0;
	Frac = DblIx - IxA;
	AltAzECRad = AltAzEC[IxA] + Frac * (AltAzEC[IxB] - AltAzEC[IxA]);
	AltAzECRad *= DegToRad;
}

void AnalysisToAltAzEC( void)
{
	struct LINK_POSITION* LowLinkPos;
	struct LINK_POSITION* HighLinkPos;
	double LowSep, HighSep;

	if( LinkPosCount)
	{
		SortLinkPos( "Az_Ascend");
		for( Ix = 0; Ix < ECSize; Ix++)
		{
			CurrentLinkPos = FirstLinkPos;
			while( CurrentLinkPos != NULL && CurrentLinkPos->P.Az < Ix*EC_Resolution_Deg*DegToRad)
				CurrentLinkPos = CurrentLinkPos->NextLinkPos;
			HighLinkPos = CurrentLinkPos;
			if( HighLinkPos == NULL)
				HighLinkPos = FirstLinkPos;
			CurrentLinkPos = LastLinkPos;
			while( CurrentLinkPos != NULL && CurrentLinkPos->P.Az > Ix*EC_Resolution_Deg*DegToRad)
				CurrentLinkPos = CurrentLinkPos->PrevLinkPos;
			LowLinkPos = CurrentLinkPos;
			if( LowLinkPos == NULL)
				LowLinkPos = LastLinkPos;
			HighSep = HighLinkPos->P.Az - Ix*EC_Resolution_Deg*DegToRad;
			if( HighSep < 0)
				HighSep += OneRev;
			LowSep = Ix*EC_Resolution_Deg*DegToRad - LowLinkPos->P.Az;
			if( LowSep < 0)
				LowSep += OneRev;
			/* total separation is LowSep + HighSep, so position multiplier (from 0 to 1) along line
			between the low point and the high point is low/total separation == low/(low+high) */
			AltAzEC[Ix] = RadToDeg * (LowLinkPos->AZErr.A +
			(HighLinkPos->AZErr.A - LowLinkPos->AZErr.A) * LowSep / (LowSep + HighSep));
		}
	}
}

void InitAltAltEC( void)
{
	MallocAltAltEC();
	LoadAltAltECFile();
}

void CloseAltAltEC( void)
{
	free( AltAltEC);
}

void MallocAltAltEC( void)
{
	AltAltEC = (float*) malloc( (ECSize) * sizeof( float));
	if( AltAltEC == NULL)
		BadExit( "Problem with malloc of AltAltEC in convert module");
}

Flag LoadAltAltECFile( void)
{
	char Index[ValueSize];

	Input = fopen( AltAltECFile, "r");
	if( Input == NULL)
	{
		for( Ix = 0; Ix < ECSize; Ix++)
			AltAltEC[Ix] = 0;
		if( DisplayOpeningMsgs)
			printf( "\ndid not find ALTALTEC.DAT file, creating new one with zeroed values");
		SaveAltAltECFile();
		return False;
	}
	else
	{
		if( DisplayOpeningMsgs)
			printf( "\nfound ALTALTEC.DAT");
		fscanf( Input, "%s %s", Index, Value);
		while( !feof( Input))
		{
			Ix = (atoi( Index) + 180)/EC_Resolution_Deg;
			if( Ix >= ECSize || Ix < 0)
				BadExit( "bad Ix in LoadAltAltECFile");
			AltAltEC[Ix] = atof( Value);
			fscanf( Input, "%s %s", Index, Value);
		}
	}
	fclose( Input);
	return True;
}

void SaveAltAltECFile( void)
{
	Output = fopen( AltAltECFile, "w");
	if( Output == NULL)
		BadExit( strcat( "Could not open ", AltAltECFile));

	for( Ix = 0; Ix < ECSize; Ix++)
		fprintf( Output, "%d %f\n", Ix*EC_Resolution_Deg - 180, AltAltEC[Ix]);

	fclose( Output);
}

void SetAltAltECValue( void)
{
	int IxA, IxB;
	double DblIx, Frac;

	DblIx = (Current.Alt*RadToDeg + 180)/EC_Resolution_Deg + .5;
	while( DblIx >= ECSize)
		DblIx -= ECSize;
	while( DblIx < 0)
		DblIx += ECSize;
	IxA = DblIx;
	if( IxA < 0 || IxA >= ECSize)
		BadExit( "Bad IxA in SetAltAltECValue");
	IxB = IxA + 1;
	if( IxB >= ECSize)
		IxB = 0;
	Frac = DblIx - IxA;
	AltAltECRad = AltAltEC[IxA] + Frac * (AltAltEC[IxB] - AltAltEC[IxA]);
	AltAltECRad *= DegToRad;
}

void AnalysisToAltAltEC( void)
{
	int IxA;
	struct LINK_POSITION* LowLinkPos;
	struct LINK_POSITION* HighLinkPos;
	double LowSep, HighSep;

	if( LinkPosCount)
	{
		SortLinkPos( "Alt_Ascend");
		for( IxA = 0; IxA < ECSize; IxA++)
		{
			Ix = IxA - 180 / EC_Resolution_Deg;
			CurrentLinkPos = FirstLinkPos;
			while( CurrentLinkPos != NULL && CurrentLinkPos->P.Alt < Ix*EC_Resolution_Deg*DegToRad)
				CurrentLinkPos = CurrentLinkPos->NextLinkPos;
			HighLinkPos = CurrentLinkPos;
			if( HighLinkPos == NULL)
				HighLinkPos = FirstLinkPos;
			CurrentLinkPos = LastLinkPos;
			while( CurrentLinkPos != NULL && CurrentLinkPos->P.Alt > Ix*EC_Resolution_Deg*DegToRad)
				CurrentLinkPos = CurrentLinkPos->PrevLinkPos;
			LowLinkPos = CurrentLinkPos;
			if( LowLinkPos == NULL)
				LowLinkPos = LastLinkPos;
			HighSep = HighLinkPos->P.Alt - Ix*EC_Resolution_Deg*DegToRad;
			if( HighSep < 0)
				HighSep += OneRev;
			LowSep = Ix*EC_Resolution_Deg*DegToRad - LowLinkPos->P.Alt;
			if( LowSep < 0)
				LowSep += OneRev;
			AltAltEC[IxA] = RadToDeg * (LowLinkPos->AZErr.A +
			(HighLinkPos->AZErr.A - LowLinkPos->AZErr.A) * LowSep / (LowSep + HighSep));
		}
	}
}

void InitAzAzEC( void)
{
	MallocAzAzEC();
	LoadAzAzECFile();
}

void CloseAzAzEC( void)
{
	free( AzAzEC);
}

void MallocAzAzEC( void)
{
	AzAzEC = (float*) malloc( (ECSize) * sizeof( float));
	if( AzAzEC == NULL)
		BadExit( "Problem with malloc of AzAzEC in convert module");
}

Flag LoadAzAzECFile( void)
{
	char Index[ValueSize];

	Input = fopen( AzAzECFile, "r");
	if( Input == NULL)
	{
		for( Ix = 0; Ix < ECSize; Ix++)
			AzAzEC[Ix] = 0;
		if( DisplayOpeningMsgs)
			printf( "\ndid not find AZAZEC.DAT file, creating new one with zeroed values");
		SaveAzAzECFile();
		return False;
	}
	else
	{
		if( DisplayOpeningMsgs)
			printf( "\nfound AZAZEC.DAT");
		fscanf( Input, "%s %s", Index, Value);
		while( !feof( Input))
		{
			Ix = atoi( Index)/EC_Resolution_Deg;
			if( Ix >= ECSize || Ix < 0)
				BadExit( "bad Ix in LoadAzAzECFile");
			AzAzEC[Ix] = atof( Value);
			fscanf( Input, "%s %s", Index, Value);
		}
	}
	fclose( Input);
	return True;
}

void SaveAzAzECFile( void)
{
	Output = fopen( AzAzECFile, "w");
	if( Output == NULL)
		BadExit( strcat( "Could not open ", AzAzECFile));

	for( Ix = 0; Ix < ECSize; Ix++)
		fprintf( Output, "%d %f\n", Ix*EC_Resolution_Deg, AzAzEC[Ix]);

	fclose( Output);
}

void SetAzAzECValue( void)
{
	int IxA, IxB;
	double DblIx, Frac;

	DblIx = Current.Az*RadToDeg/EC_Resolution_Deg + .5;
	while( DblIx >= ECSize)
		DblIx -= ECSize;
	while( DblIx < 0)
		DblIx += ECSize;
	IxA = DblIx;
	if( IxA < 0 || IxA >= ECSize)
		BadExit( "Bad IxA in SetAzAzECValue");
	IxB = IxA + 1;
	if( IxB >= ECSize)
		IxB = 0;
	Frac = DblIx - IxA;
	AzAzECRad = AzAzEC[IxA] + Frac * (AzAzEC[IxB] - AzAzEC[IxA]);
	AzAzECRad *= DegToRad;
}

void AnalysisToAzAzEC( void)
{
	struct LINK_POSITION* LowLinkPos;
	struct LINK_POSITION* HighLinkPos;
	double LowSep, HighSep;

	if( LinkPosCount)
	{
		SortLinkPos( "Az_Ascend");
		for( Ix = 0; Ix < ECSize; Ix++)
		{
			CurrentLinkPos = FirstLinkPos;
			while( CurrentLinkPos != NULL && CurrentLinkPos->P.Az < Ix*EC_Resolution_Deg*DegToRad)
				CurrentLinkPos = CurrentLinkPos->NextLinkPos;
			HighLinkPos = CurrentLinkPos;
			if( HighLinkPos == NULL)
				HighLinkPos = FirstLinkPos;
			CurrentLinkPos = LastLinkPos;
			while( CurrentLinkPos != NULL && CurrentLinkPos->P.Az > Ix*EC_Resolution_Deg*DegToRad)
				CurrentLinkPos = CurrentLinkPos->PrevLinkPos;
			LowLinkPos = CurrentLinkPos;
			if( LowLinkPos == NULL)
				LowLinkPos = LastLinkPos;
			HighSep = HighLinkPos->P.Az - Ix*EC_Resolution_Deg*DegToRad;
			if( HighSep < 0)
				HighSep += OneRev;
			LowSep = Ix*EC_Resolution_Deg*DegToRad - LowLinkPos->P.Az;
			if( LowSep < 0)
				LowSep += OneRev;
			AzAzEC[Ix] = RadToDeg * (LowLinkPos->AZErr.Z +
			(HighLinkPos->AZErr.Z - LowLinkPos->AZErr.Z) * LowSep / (LowSep + HighSep));
		}
	}
}

