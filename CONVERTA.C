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

/* advanced convert routines */

Flag LoadAnalysisFileIntoMemory( void)
{
	struct LINK_POSITION* LinkPos;
	struct Position P;

	Input = fopen( AnalysisFile, "r");
	if( Input == NULL)
		return False;

	if( FirstLinkPos != NULL)
		FreeAllLinkPos();

	while( !feof( Input))
	{
		FReadPositionToNewLine( Input, &P);
		if( !feof( Input))
		{
			LinkPos = InitLinkPos();
			AddLinkPos( LinkPos);
			LinkPosCount++;
			LinkPos->InvAngSepSqr = 0;
			LinkPos->AZWeightedErr.A = 0;
			LinkPos->AZWeightedErr.Z = 0;
			LinkPos->AZErr.A = 0;
			LinkPos->AZErr.Z = 0;
			LinkPos->P = P;
		}
	}
	fclose( Input);
	return True;
}

void CalcAnalysisErrors( void)
{
	double PointErr;
	double PointErrTot = 0;
	struct Position Temp, Test;

	Temp = Current;

	/* calculate errors */
	MaxPointErr = 0;
	CurrentLinkPos = FirstLinkPos;
	while( CurrentLinkPos != NULL)
	{
		Test = CurrentLinkPos->P;
		Current = Test;
		GetAltaz();
		/* if scope aimed higher or more CW than what it should be, then it is defined as a positive
		error */
		CurrentLinkPos->AZErr.A = Test.Alt - Current.Alt;
		/* azimuth errors in terms of true field decrease towards the zenith */
		CurrentLinkPos->AZErr.Z = (Test.Az - Current.Az) * cos( Test.Alt);
		PointErr = sqrt( CurrentLinkPos->AZErr.A * CurrentLinkPos->AZErr.A +
					  CurrentLinkPos->AZErr.Z * CurrentLinkPos->AZErr.Z);
		PointErrTot += PointErr;
		if( PointErr > MaxPointErr)
			MaxPointErr = PointErr;
		CurrentLinkPos = CurrentLinkPos->NextLinkPos;
	}
	PointErrRMS = PointErrTot / LinkPosCount;

	Current = Temp;
}

void CalcAnalysisErrorsWriteFile( void)
{
	double PointErr;
	double PointErrTot = 0;
	struct Position Temp, Test;

	Temp = Current;

	/* calculate errors */
	Output = fopen( AnalysisErrorFile, "w");
	if( Output == NULL)
		BadExit( strcat( "Could not open for write ", AnalysisErrorFile));
	MaxPointErr = 0;
	CurrentLinkPos = FirstLinkPos;
	while( CurrentLinkPos != NULL)
	{
		Test = CurrentLinkPos->P;
		FWritePosition( Output, &Test, No);
		Current = Test;
		GetAltaz();
		/* if scope aimed higher or more CW than what it should be, then it is defined as a positive
		error */
		CurrentLinkPos->AZErr.A = Test.Alt - Current.Alt;
		/* azimuth errors in terms of true field decrease towards the zenith */
		CurrentLinkPos->AZErr.Z = (Test.Az - Current.Az) * cos( Test.Alt);
		PointErr = sqrt( CurrentLinkPos->AZErr.A * CurrentLinkPos->AZErr.A +
					  CurrentLinkPos->AZErr.Z * CurrentLinkPos->AZErr.Z);
		PointErrTot += PointErr;
		if( PointErr > MaxPointErr)
			MaxPointErr = PointErr;
		fprintf( Output, " alt_err_arcmin %3.3f az_err_arcmin %3.3f\n",
		CurrentLinkPos->AZErr.A*RadToArcmin, CurrentLinkPos->AZErr.Z*RadToArcmin);
		CurrentLinkPos = CurrentLinkPos->NextLinkPos;
	}
	fclose( Output);
	PointErrRMS = PointErrTot / LinkPosCount;

	Current = Temp;
}

void AppendAnalysisToPMC( void)
{
	Output = fopen( PMCFile, "a");
	if( Output == NULL)
		BadExit( strcat( "Could not open %s for append", PMCFile));

	CurrentLinkPos = FirstLinkPos;
	while( CurrentLinkPos != NULL)
	{
		fprintf( Output, "%f %f %f %f\n",
		CurrentLinkPos->P.Alt*RadToDeg,
		CurrentLinkPos->P.Az*RadToDeg,
		CurrentLinkPos->AZErr.A*RadToDeg,
		CurrentLinkPos->AZErr.Z*RadToDeg);

		CurrentLinkPos = CurrentLinkPos->NextLinkPos;
	}
	fclose( Output);
}

Flag LoadPMC( void)
{
	struct LINK_PMC* LinkPMC;
	struct AZDouble AZ;
	struct AZDouble AZErr;

	Input = fopen( PMCFile, "r");
	if( Input == NULL)
		return False;
	else
	{
		if( FirstLinkPMC != NULL)
			FreeAllLinkPMC();

		while( !feof( Input))
		{
			FReadDouble( Input, &AZ.A);
			FReadDouble( Input, &AZ.Z);
			FReadDouble( Input, &AZErr.A);
			FReadDouble( Input, &AZErr.Z);
			if( !feof( Input))
			{
				LinkPMC = InitLinkPMC();
				AddLinkPMC( LinkPMC);
				LinkPMCCount++;
				LinkPMC->AZ.A = AZ.A * DegToRad;
				LinkPMC->AZ.Z = AZ.Z * DegToRad;
				LinkPMC->AZErr.A = AZErr.A * DegToRad;
				LinkPMC->AZErr.Z = AZErr.Z * DegToRad;
			}
		}
		fclose( Input);
		return True;
	}
}

void DisplayLinkPMC( void)
{
	printf( "\n\n\ndisplay of PMC.DAT file\n\n");
	CurrentLinkPMC = FirstLinkPMC;
	while( CurrentLinkPMC != NULL)
	{
		printf( "%f %f %f %f\n",
		CurrentLinkPMC->AZ.A * RadToDeg,
		CurrentLinkPMC->AZ.Z * RadToDeg,
		CurrentLinkPMC->AZErr.A * RadToDeg,
		CurrentLinkPMC->AZErr.Z * RadToDeg);

		CurrentLinkPMC = CurrentLinkPMC->NextLinkPMC;
	}
}

void CalcPMCErrors( void)
{
	double PointErr;
	double PointErrTot = 0;

	MaxPointErr = 0;
	CurrentLinkPMC = FirstLinkPMC;
	while( CurrentLinkPMC != NULL)
	{
		/* azimuth errors in terms of true field decrease towards the zenith */
		PointErr = sqrt( CurrentLinkPMC->AZErr.A * CurrentLinkPMC->AZErr.A +
							  CurrentLinkPMC->AZErr.Z * CurrentLinkPMC->AZErr.Z);
		PointErrTot += PointErr;
		if( PointErr > MaxPointErr)
			MaxPointErr = PointErr;
		CurrentLinkPMC = CurrentLinkPMC->NextLinkPMC;
	}
	PointErrRMS = PointErrTot / LinkPMCCount;
}

void CalcPMC( double A, double Z)
{
	double sinA, cosA;
	double angsep;
	double Total_angsep;
	double InvAngSepSqr;
	struct AZDouble AZWeightedErr;


	Total_angsep = 0;
	AZWeightedErr.A = AZWeightedErr.Z = 0;
	sinA = sin( A);
	cosA = cos( A);
	CurrentLinkPMC = FirstLinkPMC;
	while( CurrentLinkPMC != NULL)
	{
		angsep = acos( sinA * sin( CurrentLinkPMC->AZ.A) +
		cosA * cos( CurrentLinkPMC->AZ.A) * cos( Z - CurrentLinkPMC->AZ.Z));
		InvAngSepSqr = 1/(angsep*angsep*angsep);
		Total_angsep += InvAngSepSqr;
		AZWeightedErr.A += CurrentLinkPMC->AZErr.A * InvAngSepSqr;
		AZWeightedErr.Z += CurrentLinkPMC->AZErr.Z * InvAngSepSqr;

		CurrentLinkPMC = CurrentLinkPMC->NextLinkPMC;
	}
	/* positive values mean scope pointed too far CW */
	PMC.A = AZWeightedErr.A / Total_angsep;
	PMC.Z = AZWeightedErr.Z / Total_angsep;
}

Flag ComputeBestZ123FromPosition( struct Position* P)
{
	Flag Rtn;
	struct LINK_POSITION* LinkPos;

	if( FirstLinkPos != NULL)
		FreeAllLinkPos();

	LinkPos = InitLinkPos();
	AddLinkPos( LinkPos);
	LinkPosCount++;
	LinkPos->InvAngSepSqr = 0;
	LinkPos->AZWeightedErr.A = 0;
	LinkPos->AZWeightedErr.Z = 0;
	LinkPos->AZErr.A = 0;
	LinkPos->AZErr.Z = 0;
	LinkPos->P = *P;

	Rtn = ComputeBestZ123FromLinkPos();
	return Rtn;
}

Flag ComputeBestZ123FromAnalysisFile( void)
{
	Flag Rtn;

	if( Two.Init && LoadAnalysisFileIntoMemory() && LinkPosCount)
	{
		Rtn = ComputeBestZ123FromLinkPos();
		return Rtn;
	}
	return False;
}

/* compute mount misalignment errors Z1, Z2, Z3 using iterative search;
	best values picked by mean square of resulting altitude and azimuth errors: that is, find best values of Z123 that,
	after plugging in Z123 values, minimize altitude and azimuth errors (azimuth error corrected for cos of altitude);
	search for Z3 outside of search for Z12 because Z3 does not depend on re-init'ing;
	ComputeBestZ3FromLL() starts with CalcAltOffsetIteratively();
	Z3 range after determining AltOffset iteratively is +- 1 deg, Z12 range is +- 5 deg;
	critically important to separate Z1 Z2 cleanly by determining accurate azimuths for series of altitudes between 10 and 80 deg;
	routine takes several seconds to run on 1.5ghz machine;
*/

Flag ComputeBestZ123FromLinkPos( void)
{
		double StartZ1 = 0.;
		double StartZ2 = 0.;
		double Z12Range = 18000. * ArcsecToRad;
		double Z12Interval = 1800. * ArcsecToRad;
		double MinInterval = 3.6 * ArcsecToRad;

		if( ComputeBestZ3FromLinkPos())
			return ComputeBestZ12FromLinkPosSubr( StartZ1, StartZ2, Z12Range, Z12Interval, MinInterval, BestZ3);

		return False;
}

Flag ComputeBestZ12FromLinkPosSubr( double StartZ1, double StartZ2, double Z12Range, double Z12Interval, double MinInterval, double WorkZ3)
{
	double HoldZ1, HoldZ2;
	struct Position Temp;

	if( Two.Init && LinkPosCount)
	{
		WriteInitHistoryFile = False;
		Temp = Current;
		HoldZ1 = Z1;
		HoldZ2 = Z2;

		BestPointErrRMS = MAXDOUBLE;
		BestZ1 = BestZ2 = MAXDOUBLE;
		do
		{
			for( Z1 = StartZ1 - Z12Range; Z1 <= StartZ1 + Z12Range; Z1 += Z12Interval)
				for( Z2 = StartZ2 - Z12Range; Z2 <= StartZ2 + Z12Range; Z2 += Z12Interval)
				{
					SetMountErrorsDeg( Z1*RadToDeg, Z2*RadToDeg, WorkZ3*RadToDeg);
					Current = One;
					InitMatrix( 1);
					CalcAnalysisErrors();
					if( PointErrRMS < BestPointErrRMS)
					{
						BestPointErrRMS = PointErrRMS;
						BestZ1 = Z1;
						BestZ2 = Z2;
					}
				}
			StartZ1 = BestZ1;
			StartZ2 = BestZ2;
			Z12Range /= 10.;
			Z12Interval /= 10.;
		}while( Z12Interval >= MinInterval);

		Z1 = HoldZ1;
		Z2 = HoldZ2;
		SetMountErrorsDeg( Z1*RadToDeg, Z2*RadToDeg, Z3*RadToDeg);
		InitMatrix( 1);
		Current = Temp;
		WriteInitHistoryFile = True;
		return True;
	}
	return False;
}

Flag ComputeBestZ3FromPosition( struct Position* P)
{
	Flag Rtn;
	struct LINK_POSITION* LinkPos;

	if( FirstLinkPos != NULL)
		FreeAllLinkPos();

	LinkPos = InitLinkPos();
	AddLinkPos( LinkPos);
	LinkPosCount++;
	LinkPos->InvAngSepSqr = 0;
	LinkPos->AZWeightedErr.A = 0;
	LinkPos->AZWeightedErr.Z = 0;
	LinkPos->AZErr.A = 0;
	LinkPos->AZErr.Z = 0;
	LinkPos->P = *P;

	Rtn = ComputeBestZ3FromLinkPos();
	return Rtn;
}

// computing Z12 errors critically depends on accurately pegging Z3 error;
// compare all positions against each other in linked list position, calculating altitude offset for each pairing, then take the average

Flag ComputeBestZ3FromLinkPos( void)
{
	struct LINK_POSITION* A;
	struct LINK_POSITION* B;
	double Tot = 0.;
	int Cnt = 0;

	if( Two.Init==True && LinkPosCount>0)
	{
		CalcAltOffsetIteratively( One, Two);
		Tot += AltOffset;
		Cnt++;
		A = FirstLinkPos;
		while( A != NULL)
		{
			CalcAltOffsetIteratively( A->P, One);
			Tot += AltOffset;
			Cnt++;
			CalcAltOffsetIteratively( A->P, Two);
			Tot += AltOffset;
			Cnt++;
			A = A->NextLinkPos;
		}

		A = FirstLinkPos;
		while( A != LastLinkPos)
		{
			B = A->NextLinkPos;
			while( B != NULL)
			{
				CalcAltOffsetIteratively( A->P, B->P);
				Tot += AltOffset;
				Cnt++;
				B = B->NextLinkPos;
			}
			A = A->NextLinkPos;
		}
		BestZ3 = Tot/(double)Cnt;
		//gotoxy( 2, 2); printf( "\nBestZ3Deg 3.3%f", BestZ3*RadToDeg);
		return True;
	}
	return False;
}

Flag ComputeBestZ3FromAnalysisFile( void)
{
	Flag Rtn;

	if( Two.Init && LoadAnalysisFileIntoMemory() && LinkPosCount)
	{
		Rtn = ComputeBestZ3FromLinkPos();
		return Rtn;
	}
	return False;
}

double CalcEquatAngularSep( struct Position* A, struct Position* Z)
{
	double angsep;
	double AHA, ZHA, HADiff;

	/* hour angles */
	AHA = A->SidT - A->Ra;
	ZHA = Z->SidT - Z->Ra;
	HADiff = AHA - ZHA;

	angsep =  acos( sin( A->Dec) * sin( Z->Dec)
	+ cos( A->Dec) * cos( Z->Dec) * cos( HADiff));

	return angsep;
}

double CalcAltazAngularSep( struct Position* A, struct Position* Z)
{
	double angsep;
	double AzDiff;

	AzDiff = A->Az - Z->Az;
	angsep = acos( sin( A->Alt) * sin( Z->Alt)
	+ cos( A->Alt) * cos( Z->Alt) * cos( AzDiff));

	return angsep;
}

double AngSepDiff( struct Position* A, struct Position* Z)
{
	double Diff;

	Diff = fabs( CalcEquatAngularSep( A, Z)) - fabs( CalcAltazAngularSep( A, Z));

	if( Diff < 0)
		Diff = -Diff;
	return Diff;
}

/* angular separation of two equatorial coordinates should = the angular separation of the
corresponding altazimuth coordinates;
for target altitudes that cross the equator of their coordinate system, there are two solutions */

/* formula from Dave Ek <ekdave@earthlink.net> */
void CalcAltOffset( struct Position A, struct Position Z)
{
	double A1, A2;
	double N = cos( A.Az - Z.Az);
	double M = cos( CalcEquatAngularSep( &A, &Z));
	double X = (2*M - (N+1) * cos( A.Alt - Z.Alt)) / (N-1);

	/* azimuths not separate enough resulting in N-1 term being too small, or variation from ideal
	numbers in other variables likely cause */
	if( X > 1 || X < -1)
	{
		sprintf( StrBuf,
		"X=%f A.A=%3.3f Z.A=%3.3f A.Z=%3.3f Z.Z=%3.3f sep=%3.3f",
		X, A.Alt*RadToDeg, Z.Alt*RadToDeg, A.Az*RadToDeg,  Z.Az*RadToDeg,
		CalcEquatAngularSep( &A, &Z) * RadToDeg);
		BadExit( StrBuf);
	}
	else
	{
		A1 = .5 * (+acos( X) - A.Alt - Z.Alt);
		A2 = .5 * (-acos( X) - A.Alt - Z.Alt);
		if( fabs( A1) < fabs( A2))
			AltOffset = A1;
		else
			AltOffset = A2;
	}
}

/* when angular separation of altaz values closest to that of equat values, best altitude offset
found; don't pass pointers as the .Alt values are changed */
void CalcAltOffsetIteratively( struct Position A, struct Position Z)
{
	double BestAltOff;
	double Diff, LastDiff, BestDiff;
	double Incr = ArcminToRad;
	int Iter;
	/* +- 30 deg search range */
	int MaxIter = 1800;
	double BegA = A.Alt;
	double BegZ = Z.Alt;

	BestDiff = MAXDOUBLE;

	/* start from zero offset and increment offset until difference starts to get worse */
	Diff = LastDiff = MAXDOUBLE;
	for( Iter = 0; Iter < MaxIter; Iter++, A.Alt += Incr, Z.Alt += Incr)
	{
		Diff = AngSepDiff( &A, &Z);
		if( Diff < BestDiff)
		{
			BestDiff = Diff;
			BestAltOff = A.Alt - BegA;
		}
		if( Diff > LastDiff)
			break;
		else
			LastDiff = Diff;
	}
	/* again, start from zero offset, but this time decrement offset */
	A.Alt = BegA;
	Z.Alt = BegZ;
	Diff = LastDiff = MAXDOUBLE;
	for( Iter = 0; Iter < MaxIter; Iter++, A.Alt -= Incr, Z.Alt -= Incr)
	{
		Diff = AngSepDiff( &A, &Z);
		if( Diff < BestDiff)
		{
			BestDiff = Diff;
			BestAltOff = A.Alt - BegA;
		}
		if( Diff > LastDiff)
			break;
		else
			LastDiff = Diff;
	}
	if( BestAltOff == MAXDOUBLE)
		AltOffset = 0;
	else
		AltOffset = BestAltOff;
}

/*
void TestAltOffset( void)
{
	double var;

	printf( "\n\n\nTest of altitude offset code...\n");
	One.Init = Two.Init = Yes;
	Three.Init = No;

	printf( "\n               RA      Dec     Alt      Az    SidT");
	printf( "\nexample: 1:    72.1, 20.4333, 63.785, 210.287, 0");
	printf( "\n         2: 359.138,  2.5833,  4.164,  269.58, 0");
	printf( "\n         +- an equal amount from both altitudes...");
	printf( "\n\n1st (deg) Ra ");
	GetDouble( &var);
	One.Ra = var*DegToRad;
	printf( "   Dec ");
	GetDouble( &var);
	One.Dec = var*DegToRad;
	printf( "   Alt ");
	GetDouble( &var);
	One.Alt = var*DegToRad;
	printf( "   Az ");
	GetDouble( &var);
	One.Az = var*DegToRad;
	printf( "   SidT ");
	GetDouble( &var);
	One.SidT = var*DegToRad;

	printf( "\n2nd (deg) Ra ");
	GetDouble( &var);
	Two.Ra = var*DegToRad;
	printf( "   Dec ");
	GetDouble( &var);
	Two.Dec = var*DegToRad;
	printf( "   Alt ");
	GetDouble( &var);
	Two.Alt = var*DegToRad;
	printf( "   Az ");
	GetDouble( &var);
	Two.Az = var*DegToRad;
	printf( "   SidT ");
	GetDouble( &var);
	Two.SidT = var*DegToRad;

	CalcAltOffsetIteratively( One, Two);
	printf( "\nIterative AltOffset %f, One.Alt %f, Two.Alt %f", AltOffset * RadToDeg,
	(One.Alt + AltOffset) * RadToDeg, (Two.Alt + AltOffset) * RadToDeg);

	CalcAltOffset( One, Two);
	printf( "\nFormula AltOffset %f, One.Alt %f, Two.Alt %f", AltOffset * RadToDeg,
	(One.Alt + AltOffset) * RadToDeg, (Two.Alt + AltOffset) * RadToDeg);

	NewLine;
	ContMsgRoutine();
}
*/
