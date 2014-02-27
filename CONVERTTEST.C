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
#include "headertest.h"

/* convert trig */
void CalcLatVars( void)
{
	/* for conversion routines that need site latitude such as CalcAirMass() and CalcDomeAzimuth() */
	ConfigLat = LatitudeDeg * DegToRad;
	ConfigSinLat = sin( ConfigLat);
	ConfigCosLat = cos( ConfigLat);
}

/* this function gets altazimuth coordinates using cfg longitude and cfg latitude;
	if site or sky's altaz coord desired, then set Pos vars:
		1. current local sidereal time (stored in current coord) which is based on:
			system date, time, timezone, daylight savings time,
		2. current equatorial coordinates */

void GetSiteAltaz( void)
{
	double SinSiteAlt;
	double CosSiteAz;

	SitePos.HA = SitePos.SidT - SitePos.Ra;

	SinSiteAlt = sin( SitePos.Dec) * ConfigSinLat + cos( SitePos.Dec) * ConfigCosLat * cos( SitePos.HA);
	BoundsSinCos( &SinSiteAlt);
	SitePos.Alt = asin( SinSiteAlt);

	CosSiteAz = (sin( SitePos.Dec) - ConfigSinLat * SinSiteAlt) / (ConfigCosLat * cos( SitePos.Alt));
	BoundsSinCos( &CosSiteAz);
	SitePos.Az = acos( CosSiteAz);
	if( sin( SitePos.SidT - SitePos.Ra) > 0.)
		SitePos.Az = OneRev - SitePos.Az;
}

// same general equation as GetSiteAltaz(); set SitePos altaz and SidT before calling
void GetSiteEquat( void)
{
	double SinDec;
	double CosHA;

	SinDec = sin( SitePos.Alt) * ConfigSinLat + cos( SitePos.Alt) * ConfigCosLat * cos( SitePos.Az);
	BoundsSinCos( &SinDec);
	SitePos.Dec = asin( SinDec);

	CosHA = (sin( SitePos.Alt) - ConfigSinLat * SinDec) / (ConfigCosLat * cos( SitePos.Dec));
	BoundsSinCos( &CosHA);
	SitePos.HA = acos( CosHA);

	if( sin( SitePos.Az) > 0.)
		SitePos.HA = OneRev - SitePos.HA;
	SitePos.Ra = SitePos.SidT - SitePos.HA;
	BoundsOneRev( &SitePos.Ra);
}

/* convert matrix... */

void InitConvert( void)
{
	SubrFlag = 'U';
	GEMflippedFlag = No;
	FieldRate = 0;
	AirMass = 0;
	DomeAzimuth = 0;

	FirstLinkPos = NULL;
	WriteInitHistoryFile = Yes;

	CalcLatVars();

	/* set mount errors from the config.dat file */
	SetMountErrorsDeg( Z1Deg, Z2Deg, Z3Deg);

	/* set hold, hold sin, and hold cos values of Current.Dec */
	HoldCurrentDec = -9999;
	CheckHoldSinCosCurrentDec();

	/* clear out InitHistoryFile */
	Output = fopen( InitHistoryFile, "w");
	if( Output == NULL)
		BadExit( strcat( "Could not create ", InitHistoryFile));
	fclose( Output);

	strcpy( WhyInit, WHY_INIT_STARTUP);
	AltOffset = 0;

	switch( StartInitState)
	{
		case 0:
			if( DisplayOpeningMsgs)
			{
				printf( "\nUsing LatitudeDeg, LongitudeDeg, Tz, and DST values from %s", ConfigFile);
				printf( "\n\n   would you like to:");
				printf( "\n      1. adopt an equatorial alignment");
				printf( "\n      2. adopt an altazimuth alignment (set 0 azimuth to the pole)");
				printf( "\n      3. adopt an altalt alignment (set 0 azimuth to the pole)");
				printf( "\n      4. adopt no alignment and start from scratch");
				if( One.Init && Two.Init)
					printf( "\n      5. use previous alignment data from %s", ConfigFile);
				printf( "\n   what is your choice? ");
				Response = getch();
				printf( "%c\n", Response);
				switch( Response)
				{
					case '1':
						InitConvertEquat();
						break;
					case '2':
						InitConvertAltaz();
						break;
					case '3':
						InitConvertAltAlt( 0.);
						break;
					case '4':
						One.Init = Two.Init = Three.Init = No;
						break;
					case '5':
					default:
						LoadAlign();
				}
			}
			else
				LoadAlign();
			break;
		case 1:
			InitConvertEquat();
			break;
		case 2:
			InitConvertAltaz();
			break;
		case 3:
			One.Init = Two.Init = Three.Init = No;
			break;
		case 4:
			LoadAlign();
	}
}

/*
 altitude always positive when above site horizon;
 az of 0 means hour angle of 0, az always increases clockwise;
 in northern hemisphere at pole, sky appears to rotate counter-clockwise when looking up at the pole
	 and clockwise when looking down at the ground, in southern hemisphere rotations are reversed;
 hence, in northern hemisphere, tracking causes az to increase, in southern hemisphere, az decreases;
*/

/*
 first init point alt/az of 0/0 associated with hour angle/dec of 0/0;
 second init point west on celestial equator (az=90 in north, az=270 in south);
*/
void InitConvertEquat( void)
{
	double Ra;
	struct Position Temp;

	/* if altazimuth refraction turned on, then turn refraction correction off */
	if( RefractFlag == 1)
		RefractFlag = 0;
	InitState = InitEquat;

	SetCoordDeg( &One, Current.SidT*RadToDeg, 0, 0, 0, Current.SidT*RadToDeg);
	Ra = One.Ra - QtrRev;
	if( Ra < 0)
		Ra += OneRev;
	if( LatitudeDeg >= 0)
		SetCoordDeg( &Two, Ra*RadToDeg, 45, 45, 90, One.SidT*RadToDeg);
	else
		SetCoordDeg( &Two, Ra*RadToDeg, -45, 45, 270, One.SidT*RadToDeg);

	One.Init = Two.Init = Yes;
	Three.Init = No;
	Temp = Current;
	Current = One;
	/* initmatrix() will do all inits that are marked 'yes' with a single call */
	InitMatrix( 1);
	Current = Temp;
	DetectGEMflippedFromCurrentAltaz();
	printf( "current altitude coordinate indicate that GEM flip state is %s", GEMflippedFlag?"ON":"off");
}

/*
 first init points at celestial pole;
 second init points at celestial equator on meridian;
*/
void InitConvertAltaz( void)
{
	struct Position Temp;

	InitState = InitAltaz;

	SetCoordDeg( &One, 0, QtrRev*RadToDeg, LatitudeDeg, 0, Current.SidT*RadToDeg);
	SetCoordDeg( &Two, Current.SidT*RadToDeg, 0, 90-LatitudeDeg, HalfRev*RadToDeg,
	Current.SidT*RadToDeg);

	One.Init = Two.Init = Yes;
	Three.Init = No;
	Temp = Current;
	Current = One;
	InitMatrix( 1);
	Current = Temp;
}

void InitConvertAltAlt( double StartAz)
{
	struct Position Temp;
	struct AZDouble StartEquat;

	InitState = InitAltAlt;

	// get site equatorial coordinates for primary axis aimed at horizon point StartAz
	SitePos.Alt = 0.;
	SitePos.Az = StartAz;
	SitePos.SidT = Current.SidT;
	GetSiteEquat();
	StartEquat.A = SitePos.Dec;
	StartEquat.Z = SitePos.Ra;

	/* primary axis points to horizon, typically at the point under the pole (this axis is the azimuth axis of an altazimuth
		tipped over 90 degrees), while secondary axis swings ala the dec axis inside a yoke mount (otherwise known as the
		altitude axis);
		mount's primary axis (the 'az' axis in a traditional altazimuth) can point at other horizon or azimuth directions other
		than directly under the pole, optimizing for overall travel of the motors during a period of tracking (celestial or earth orbit),
		or, optimizing for lowest maximum instantaneous speed of motors);
		additionally, the third axis can be motorized and given a share of the tracking responsibilities, leading to optimizing
		for no field rotation;
	*/
	// point to horizon where scope's primary pole is pointed: altaz=90,0 and equat=StartEquat as derived above from StartAz
	// (if StartAz=0, then altaz=90,0 and equat=SidT-180deg,90-lat)
	SetCoordDeg( &One, StartEquat.Z*RadToDeg, StartEquat.A, 90., 0., Current.SidT*RadToDeg);
	// point to zenith: altaz=0,0 and equat=SidT,lat
	SetCoordDeg( &Two, Current.SidT*RadToDeg, LatitudeDeg, 0., 0., Current.SidT*RadToDeg);

	One.Init = Two.Init = True;
	Three.Init = False;
	Temp = Current;
	Current = One;
	InitMatrix( 1);
	Current = Temp;
	Current.Ax3 = StartAz;
}

void DetectGEMflippedFromCurrentAltaz( void)
{
	GEMflippedFlag = No;
	if( GEMFlipPossible && Two.Init && !Siderostat)
		// use true, not indicated altitude
		if( Current.Alt + Z3 > QtrRev)
			GEMflippedFlag = Yes;
}

/* append to InitHistoryFile */
void SaveInits( void)
{
	struct time t;

	if( WriteInitHistoryFile)
	{
		gettime( &t);
		Output = fopen( InitHistoryFile, "a");
		if( Output == NULL)
			BadExit( strcat( "Could not open for append ", InitHistoryFile));
		FWritePosition( Output, &One, No);
		fprintf( Output, " %2d:%02d:%02d %s %s\n", t.ti_hour, t.ti_min, t.ti_sec, Init1Str, WhyInit);
		FWritePosition( Output, &Two, No);
		fprintf( Output, " %2d:%02d:%02d %s %s\n", t.ti_hour, t.ti_min, t.ti_sec, Init2Str, WhyInit);
		FWritePosition( Output, &Three, No);
		fprintf( Output, " %2d:%02d:%02d %s %s\n", t.ti_hour, t.ti_min, t.ti_sec, Init3Str, WhyInit);
		fclose( Output);
	}
}

void FReadPosition( FILE* File, struct Position* Pos)
{
	FReadDouble( File, &Pos->Ra);
	Pos->Ra /= RadToDeg;
	FReadDouble( File, &Pos->Dec);
	Pos->Dec /= RadToDeg;
	FReadDouble( File, &Pos->Alt);
	Pos->Alt /= RadToDeg;
	FReadDouble( File, &Pos->Az);
	Pos->Az /= RadToDeg;
	FReadDouble( File, &Pos->SidT);
	Pos->SidT /= RadToDeg;
	if( !feof( File))
		Pos->Init = Yes;
}

void FReadPositionToNewLine( FILE* File, struct Position* Pos)
{
	FReadPosition( File, Pos);
	FReadToNewLine( File);
}

void FWritePosition( FILE* File, struct Position* Pos, Flag CRLF)
{
	fprintf( File, "%f ", Pos->Ra * RadToDeg);
	fprintf( File, "%f ", Pos->Dec * RadToDeg);
	fprintf( File, "%f ", Pos->Alt * RadToDeg);
	fprintf( File, "%f ", Pos->Az * RadToDeg);
	fprintf( File, "%f ", Pos->SidT * RadToDeg);
	if( CRLF)
		fprintf( File, "\n");
}

Flag LoadAlign( void)
{
	/* temporary coordinates */
	struct Position Temp;

	InitState = InitCfgFile;

	if( One.Init)
	{
		/* local Temp */
		Temp = Current;
		Current = One;
		InitMatrix( 1);
		Current = Temp;
	}
	if( One.Init && Two.Init)
		return True;
	else
		return False;
}

void SetMountErrorsDeg( const double z1Deg, const double z2Deg, const double z3Deg)
{
	Z1Deg = z1Deg;
	Z2Deg = z2Deg;
	Z3Deg = z3Deg;
	Z1 = z1Deg*DegToRad;
	Z2 = z2Deg*DegToRad;
	Z3 = z3Deg*DegToRad;
	if( Z1 != 0 || Z2 != 0)
		Z1Z2NonZeroFlag = Yes;
	else
		Z1Z2NonZeroFlag = No;
}

void ZeroArrays( void)
{
	int I, J;

	for( I = 0; I < 4; I++)
		for( J = 0; J < 4; J++)
			QQ[I][J] = VV[I][J] = RR[I][J] = XX[I][J] = YY[I][J] = 0;
}

void CheckHoldSinCosCurrentDec( void)
{
	if( HoldCurrentDec != Current.Dec)
	{
		HoldCosCurrentDec = cos( Current.Dec);
		HoldSinCurrentDec = sin( Current.Dec);
		HoldCurrentDec = Current.Dec;
	}
}

void ArrayAsignInit( const int Init)
{
	double B;

	if( Init == 1)
		ZeroArrays();

	CheckHoldSinCosCurrentDec();

	/* B is CCW so HA formula backwards */
	B = Current.Ra - Current.SidT;
	/* XX is telescope matrix; convert parameters into rectangular (cartesian) coordinates */
	XX[1][Init] = HoldCosCurrentDec * cos( B);
	XX[2][Init] = HoldCosCurrentDec * sin( B);
	XX[3][Init] = HoldSinCurrentDec;
	/* F is CCW */
	F = OneRev - Current.Az;
	H = Current.Alt + Z3;
	SubrA();
	/* YY is celestial matrix; convert parameters into rectangular (cartesian) coordinates  */
	YY[1][Init] = YY[1][0];
	YY[2][Init] = YY[2][0];
	YY[3][Init] = YY[3][0];
}

void GenerateThirdInit( void)
{
	int I;
	double A;

	/* generate 3rd initialization point from the first two using vector product formula */
	XX[1][3] = XX[2][1]*XX[3][2] - XX[3][1]*XX[2][2];
	XX[2][3] = XX[3][1]*XX[1][2] - XX[1][1]*XX[3][2];
	XX[3][3] = XX[1][1]*XX[2][2] - XX[2][1]*XX[1][2];
	A = sqrt( XX[1][3]*XX[1][3] + XX[2][3]*XX[2][3] + XX[3][3]*XX[3][3]);
	for( I = 1; I <= 3; I++)
	{
		if( A == 0)
			XX[I][3] = MAXDOUBLE;
		else
			XX[I][3] /= A;
	}
	YY[1][3] = YY[2][1]*YY[3][2] - YY[3][1]*YY[2][2];
	YY[2][3] = YY[3][1]*YY[1][2] - YY[1][1]*YY[3][2];
	YY[3][3] = YY[1][1]*YY[2][2] - YY[2][1]*YY[1][2];
	A = sqrt( YY[1][3]*YY[1][3] + YY[2][3]*YY[2][3] + YY[3][3]*YY[3][3]);
	for( I = 1; I <= 3; I++)
	{
		if( A == 0)
			YY[I][3] = MAXDOUBLE;
		else
			YY[I][3] /= A;
	}
}

void TransformMatrix( void)
{
	int I, J, L, M, N;
	double E;

	for( I = 1; I <= 3; I++)
		for( J = 1; J <= 3; J++)
			VV[I][J] = XX[I][J];
	/* get determinate from copied into array VV */
	DeterminateSubr();
	/* save it */
	E = W;
	for( M = 1; M <= 3; M++)
	{
		for( I = 1; I <= 3; I++)
			for( J = 1; J <= 3; J++)
				VV[I][J] = XX[I][J];
		for( N = 1; N <= 3; N++)
		{
			VV[1][M] = 0;
			VV[2][M] = 0;
			VV[3][M] = 0;
			VV[N][M] = 1;
			DeterminateSubr();
			if( E == 0)
				QQ[M][N] = MAXDOUBLE;
			else
				QQ[M][N] = W/E;
		}
	}
	for( I = 1; I <= 3; I++)
		for( J = 1; J <= 3; J++)
			RR[I][J] = 0;
	for( I = 1; I <= 3; I++)
		for( J = 1; J <= 3; J++)
			for( L = 1; L <= 3; L++)
				RR[I][J] += (YY[I][L] * QQ[L][J]);
	for( M = 1; M <= 3; M++)
	{
		for( I = 1; I <= 3; I++)
			for( J = 1; J <= 3; J++)
				VV[I][J] = RR[I][J];
		DeterminateSubr();
		E = W;
		for( N = 1; N <= 3; N++)
		{
			VV[1][M] = 0;
			VV[2][M] = 0;
			VV[3][M] = 0;
			VV[N][M] = 1;
			DeterminateSubr();
			if( E == 0)
				QQ[M][N] = MAXDOUBLE;
			else
				QQ[M][N] = W/E;
		}
	}
}

/* to use, put values to init into Current., then call InitMatrix( x) with x = desired Init;
function performs all possible inits from the beginning: for example, need only call InitMatrix( 1)
once to also init Two and Three */
void InitMatrix( const int Init)
{
	struct Position Temp;

	if( Init == 3 && One.Init && Two.Init)
	{
		Temp = Current;
		Current = One;
		ArrayAsignInit( 1);
		Current = Two;
		ArrayAsignInit( 2);
		Current = Temp;
		ArrayAsignInit( 3);
		TransformMatrix();
		UpdatePostInitVarsFlag = True;
		Three = Current;
		Three.Init = Yes;
		SaveInits();
	}
	else if( Init == 2 && One.Init && Two.Init && Three.Init)
	{
		Temp = Current;
		Current = One;
		ArrayAsignInit( 1);
		Current = Temp;
		ArrayAsignInit( 2);
		Temp = Current;
		Current = Three;
		ArrayAsignInit( 3);
		Current = Temp;
		TransformMatrix();
		UpdatePostInitVarsFlag = True;
		Two = Current;
		Two.Init = Yes;
		SaveInits();
	}
	else if( Init == 2 && One.Init && !Three.Init)
	{
		Temp = Current;
		Current = One;
		ArrayAsignInit( 1);
		Current = Temp;
		ArrayAsignInit( 2);
		GenerateThirdInit();
		TransformMatrix();
		UpdatePostInitVarsFlag = True;
		Two = Current;
		Two.Init = Yes;
		SaveInits();
	}
	else if( Init == 1 && One.Init && Two.Init && Three.Init)
	{
		ArrayAsignInit( 1);
		Temp = Current;
		Current = Two;
		ArrayAsignInit( 2);
		Current = Three;
		ArrayAsignInit( 3);
		Current = Temp;
		TransformMatrix();
		UpdatePostInitVarsFlag = True;
		One = Current;
		One.Init = Yes;
		SaveInits();
	}
	else if( Init == 1 && Two.Init && !Three.Init)
	{
		ArrayAsignInit( 1);
		Temp = Current;
		Current = Two;
		ArrayAsignInit( 2);
		Current = Temp;
		GenerateThirdInit();
		TransformMatrix();
		UpdatePostInitVarsFlag = True;
		One = Current;
		One.Init = Yes;
		SaveInits();
	}
	else if( Init == 1 && !Two.Init)
	{
		ArrayAsignInit( 1);
		One = Current;
		One.Init = Yes;
		SaveInits();
	}
	else
	{
		sprintf( StrBuf, "InitMatrix() failure: Init=%d, One.Init=%d, Two.Init=%d, Three.Init=%d",
		Init, One.Init, Two.Init, Three.Init);
		BadExit( StrBuf);
	}
}

void SubrA( void)
{
	double CosF, CosH, CosZ1, CosZ2, SinF, SinH, SinZ1, SinZ2;

	CosF = cos( F);
	CosH = cos( H);
	SinF = sin( F);
	SinH = sin( H);
	if( Z1Z2NonZeroFlag)
	{
		CosZ1 = cos( Z1);
		CosZ2 = cos( Z2);
		SinZ1 = sin( Z1);
		SinZ2 = sin( Z2);
		YY[1][0] = CosF*CosH*CosZ2 - SinF*CosZ1*SinZ2 + SinF*SinH*SinZ1*CosZ2;
		YY[2][0] = SinF*CosH*CosZ2 + CosF*SinZ2*CosZ1 - CosF*SinH*SinZ1*CosZ2;
		YY[3][0] = SinH*CosZ1*CosZ2 + SinZ1*SinZ2;
	}
	else
	{
		YY[1][0] = CosF*CosH;
		YY[2][0] = SinF*CosH;
		YY[3][0] = SinH;
	}
}

void SubrSwitcher( void)
{
	double CosF, CosH, SinF, SinH;
	double CosZ1, CosZ2, SinZ1, SinZ2;

	CosF = cos( F);
	CosH = cos( H);
	SinF = sin( F);
	SinH = sin( H);

	if( Z1Z2NonZeroFlag == True)
	{
		CosZ1 = cos( Z1);
		CosZ2 = cos( Z2);
		SinZ1 = sin( Z1);
		SinZ2 = sin( Z2);
		switch( SubrFlag)
		{
			case 'S':
				SubrS( CosF, CosH, SinF, SinH);
				break;
			case 'B':
				SubrB( CosF, CosH, SinF, SinH, CosZ1, CosZ2, SinZ1, SinZ2);
				break;
			case 'T':
				SubrT( CosF, CosH, SinF, SinH, CosZ1, CosZ2, SinZ1, SinZ2);
				break;
			case 'L':
				SubrL( CosF, CosH, SinF, SinH, CosZ1, CosZ2, SinZ1, SinZ2);
				break;
			case 'U':
				SubrU( CosF, CosH, SinF, SinH, CosZ1, CosZ2, SinZ1, SinZ2);
				break;
		  default:
				BadExit( "unknown SubrFlag in Convert.SubrSwitcher()");
		}
	}
	else
	{
		YY[1][1] = CosF*CosH;
		YY[2][1] = SinF*CosH;
		YY[3][1] = SinH;
	}
}

// per Taki's eq 5.3-4
void SubrS( double CosF, double CosH, double SinF, double SinH)
{
	YY[1][1] = CosH*CosF + Z2*SinF - Z1*SinH*SinF;
	YY[2][1] = CosH*SinF - Z2*CosF - Z1*SinH*CosF;
	YY[3][1] = SinH;
}

// per Taki's eq 5.3-2
void SubrB( double CosF, double CosH, double SinF, double SinH, double CosZ1, double CosZ2, double SinZ1, double SinZ2)
{
	// 'H' is alt, 'F' is az
	YY[1][1] = (CosH*CosF + SinF*CosZ1*SinZ2 - SinH*SinF*SinZ1*CosZ2)/CosZ2;
	YY[2][1] = (CosH*SinF - CosF*CosZ1*SinZ2 + SinH*CosF*SinZ1*CosZ2)/CosZ2;
	YY[3][1] = (SinH - SinZ1*SinZ2)/(CosZ1*CosZ2);
}

// per Taki's eq 5.3-5/6 (Taki says 2 loops sufficient for Z errors of 1 deg),
// Z1=1, Z2=-1, Z3=1, alt/az=88/100 loops needed 6; Z1=2, Z2=-2, Z3=0, alt/az=90/100 loops needed 22;
// will not converge if .Dec or .Alt = 90 deg and Z12 non-zero and equat init adopted
void SubrT( double CosF, double CosH, double SinF, double SinH, double CosZ1, double CosZ2, double SinZ1, double SinZ2)
{
	double CosF1, SinF1;
	int MaxLoopCount = 25;
	struct AZDouble Last;
	struct AZDouble Err;
	int SubrTCount = 0;
	double HoldF = F;
	double HoldH = H;

	// so as to not make the Err. = invalid later
	Last.A = Last.Z = MAXDOUBLE/2.;

	// start with best guess using Taki's 'subroutine B'
	SubrB( CosF, CosH, SinF, SinH, CosZ1, CosZ2, SinZ1, SinZ2);
	do
	{
		AngleSubr();

		Err.A = fabs( Last.A - H);
		Err.Z = fabs( Last.Z - F);
		Last.A = H;
		Last.Z = F;

		CosF1 = cos( F);
		SinF1 = sin( F);

		YY[1][1] = (CosH*CosF + SinF1*CosZ1*SinZ2 - (SinH-SinZ1*SinZ2)*SinF1*SinZ1/CosZ1)/CosZ2;
		YY[2][1] = (CosH*SinF - CosF1*CosZ1*SinZ2 + (SinH-SinZ1*SinZ2)*CosF1*SinZ1/CosZ1)/CosZ2;
		YY[3][1] = (SinH-SinZ1*SinZ2)/(CosZ1*CosZ2);

		SubrTCount++;
		if( SubrTCount > MaxLoopCount)
		{
			//printf( "too many loops in SubrT()");
			F = HoldF;
			H = HoldH;
			SubrL( CosF, CosH, SinF, SinH, CosZ1, CosZ2, SinZ1, SinZ2);
		}
	}while( Err.A > TenthsArcsecToRad ||  Err.Z > TenthsArcsecToRad);
}

// use apparent alt derivation from Larry Bell, apparent az from Taki's iterative solution
void SubrU( double CosF, double CosH, double SinF, double SinH, double CosZ1, double CosZ2, double SinZ1, double SinZ2)
{
	double ApparentAlt;

	ApparentAlt = GetApparentAlt( CosZ1, CosZ2, SinZ1, SinZ2);

	SubrT( CosF, CosH, SinF, SinH, CosZ1, CosZ2, SinZ1, SinZ2);
	AngleSubr();

	CosH = cos( ApparentAlt);
	SinH = sin( ApparentAlt);
	CosF = cos( F);
	SinF = sin( F);

	YY[1][1] = CosF*CosH;
	YY[2][1] = SinF*CosH;
	YY[3][1] = SinH;
}

/* per Larry Bell's derivation
	Z1 rotation done between alt and az rotations so no closed algebraic solution, instead, search iteratively
	'H' is alt, 'F' is az
	apparent coordinates are what the encoders see, and are our goal */

double GetApparentAlt( double CosZ1, double CosZ2, double SinZ1, double SinZ2)
{
	double V1;

	V1 = (sin(H)-SinZ1*SinZ2)*CosZ1*(CosZ2/((SinZ1*SinZ1-1)*(SinZ2*SinZ2-1)));
	if( V1 > 1.)
		V1 = 1.;
	if( V1 < -1.)
		V1 = -1.;
	return asin( V1);
}

void SubrL( double CosF, double CosH, double SinF, double SinH, double CosZ1, double CosZ2, double SinZ1, double SinZ2)
{
	double TrueAz, TanTrueAz;
	double ApparentAlt, ApparentAz, BestApparentAz;
	double EE, FF, GG, HH;
	double GoalSeek, HoldGoalSeek;
	double Incr, MinIncr;
	Flag Dir;
	int SubrLCount;

	TrueAz = F;
	TanTrueAz = tan( TrueAz);

	ApparentAlt = GetApparentAlt( CosZ1, CosZ2, SinZ1, SinZ2);

	EE = cos( ApparentAlt);
	FF = sin( ApparentAlt);
	GG = CosZ2*SinZ1*FF*TanTrueAz - TanTrueAz*SinZ2*CosZ1 - CosZ2*EE;
	HH = SinZ2*CosZ1 - CosZ2*SinZ1*FF - TanTrueAz*CosZ2*EE;

	// start with best guess using Taki's 'subroutine B' for ApparentAz
	SubrB( CosF, CosH, SinF, SinH, CosZ1, CosZ2, SinZ1, SinZ2);
	AngleSubr();
	ApparentAz = F;

	// iteratively solve for best apparent azimuth by searching for a goal of 0 for GoalSeek
	BestApparentAz = ApparentAz;
	HoldGoalSeek = MAXDOUBLE;
	Incr = ArcsecToRad*2.;
	MinIncr = ArcsecToRad;
	Dir = True;
	SubrLCount = 0;
	do
	{
		if( Dir == True)
			ApparentAz += Incr;
		else
			ApparentAz -= Incr;

		GoalSeek = GG*sin( ApparentAz)-HH*cos( ApparentAz);
		// System.out.println( "GoalSeek " + GoalSeek*1000000. + " Dir " + Dir);

		if( fabs( GoalSeek) <= fabs( HoldGoalSeek))
		{
			BestApparentAz = ApparentAz;
			// System.out.println( "BestApparentAz " + BestApparentAz*Units.RadToDeg);
		}
		else
		{
			// GoakSeek getting worse, so reverse direction and cut increment by half
			Incr /= 2.;
			Dir = !Dir;
		}
		HoldGoalSeek = GoalSeek;
		SubrLCount++;
	}while( Incr >= MinIncr);

	CosF = cos( BestApparentAz);
	SinF = sin( BestApparentAz);
	CosH = cos( ApparentAlt);
	SinH = sin( ApparentAlt);

	YY[1][1] = CosF*CosH;
	YY[2][1] = SinF*CosH;
	YY[3][1] = SinH;
}

void DeterminateSubr( void)
{
	W = VV[1][1]*VV[2][2]*VV[3][3] + VV[1][2]*VV[2][3]*VV[3][1]
	  + VV[1][3]*VV[3][2]*VV[2][1] - VV[1][3]*VV[2][2]*VV[3][1]
	  - VV[1][1]*VV[3][2]*VV[2][3] - VV[1][2]*VV[2][1]*VV[3][3];
}

void AngleSubr( void)
{
	double C;

	C = sqrt( YY[1][1]*YY[1][1] + YY[2][1]*YY[2][1]);
	if( C == 0 && YY[3][1] > 0)
		H = QtrRev;
	else if( C == 0 && YY[3][1] < 0)
		H = -QtrRev;
	else if( C != 0)
		H = atan( YY[3][1]/C);
	else
		BadExit( "undetermined H in AngleSubr()");

	if( C == 0)
		/* F should be indeterminate: program listing is F = 1000*DegToRad; */
		F = 0;
	else if( C != 0 && YY[1][1] == 0 && YY[2][1] > 0)
		F = QtrRev;
	else if( C != 0 && YY[1][1] == 0 && YY[2][1] < 0)
		F = OneRev - QtrRev;
	else if( YY[1][1] > 0)
		F = atan( YY[2][1]/YY[1][1]);
	else if( YY[1][1] < 0)
		F = atan( YY[2][1]/YY[1][1]) + HalfRev;
	else
		BadExit( "undetermined F in AngleSubr()");

	while( F > OneRev)
		F -= OneRev;
	while( F < 0)
		F += OneRev;
}

void CheckForPostInitVars( void)
{
	if( UpdatePostInitVarsFlag == True)
		CalcPostInitVars();
}

// can be called after init 2: calculates apparent scope latitude from two perspectives, longitude, offset hour angle, offset azimuth,
// zenith and polar offsets
void CalcPostInitVars( void)
{
	/* temporary coordinates */
	struct Position Temp = Current;

	/* use current time */
	Current.SidT = SidT;

	// aim at equatorial pole to get azimuth offset and polar offset and 2nd latitude value
	// latitude obtained here by getting scope altitude at equatorial zenith and latitude obtained below
	// by getting scope declination at scope zenith
	Current.Dec = QtrRev;
	Current.Ra = 0.;
	GetAltaz();
	AzOff = Current.Az;
	LatEquatPole = Current.Alt;
	SinLatEquatPole = sin( LatEquatPole);
	CosLatEquatPole = cos( LatEquatPole);
	SinLatEquatPoleDividedByCosLatEquatPole = SinLatEquatPole/CosLatEquatPole;
	PolarAlignEquatPole.A = QtrRev - Current.Alt;
	if( PolarAlignEquatPole.A == 0.)
		PolarAlignEquatPole.Z = 0.;
	else
		PolarAlignEquatPole.Z = Current.Az;

	// aim at site zenith (meridian is sidereal time, declination of zenith = site latitude) to get scope offset
	Current.Ra = Current.SidT;
	Current.Dec = LatitudeDeg*DegToRad;
	GetAltaz();
	ZenithOffset.A = QtrRev-Current.Alt;
	if( ZenithOffset.A == 0.)
		ZenithOffset.Z = 0.;
	else
		ZenithOffset.Z = Current.Az;

	// aim at scope zenith to get latitude, longitude, and HAOff
	if( InitState == InitEquat || LatEquatPole == QtrRev || LatEquatPole == -QtrRev)
		Current.Alt = LatitudeDeg*DegToRad;
	else
		Current.Alt = QtrRev;
	Current.Az = 0.;
	GetEquat();

	LatScopeZenith = Current.Dec;

	// LongitudeDeg*DegToRad + Current.SidT = Greenwich Sidereal Time;
	// difference between GST and Current.Ra (== zenith) will be scope longitude
	LongitudeRad = LongitudeDeg*DegToRad + Current.SidT - Current.Ra;
	BoundsOneRev( &LongitudeRad);

	// find hour angle offset = LST( Current.SidT) - scope's meridian,
	// HA = LST - HAOff - Ra, or, HAOff = LST - Ra, by setting for zenith (HA = 0);
	// + offset = scope tilted to West, - offset = scope tilted to East;
	// HAOff varies from - offset to + offset( should be a small amount)
	HAOff = Current.SidT - Current.Ra;
	BoundsHalfRev( &HAOff);
	// for equatorial alignments, make azimuth offset same as hour angle offset
	if( InitState == InitEquat)
		AzOff = HAOff;

	// aim at scope pole to get polar offset from another perspective
	Current.Alt = QtrRev;
	Current.Az = 0.;
	GetEquat();
	PolarAlignAltazPole.A = QtrRev - Current.Dec;
	if( PolarAlignAltazPole.A == 0.)
		PolarAlignAltazPole.Z = 0.;
	else
		PolarAlignAltazPole.Z = Current.Ra;

	CalcAltOffsetIteratively( One, Two);
	AltOffsetFromCalcPostInitVars = AltOffset;

	// restore current coordinates
	Current = Temp;
	UpdatePostInitVarsFlag = False;
	// update field rotation
	CalcFieldR();
}

void GetAltaz( void)
{
	int I, J;
	double B;

	CheckHoldSinCosCurrentDec();

	/* B is CCW so HA formula backwards */
	B = Current.Ra - Current.SidT;
	/* convert to rectangular coordinates and put in XX */
	XX[1][1] = HoldCosCurrentDec * cos( B);
	XX[2][1] = HoldCosCurrentDec * sin( B);
	XX[3][1] = HoldSinCurrentDec;
	YY[1][1] = 0;
	YY[2][1] = 0;
	YY[3][1] = 0;
	/* mutiply XX by transform matrix RR to get equatorial rectangular coordinates */
	for( I = 1; I <= 3; I++)
		for( J = 1; J <= 3; J++)
			YY[I][1] += (RR[I][J] * XX[J][1]);
	/* convert to celestial coordinates */
	AngleSubr();
	/* modify for non-zero Z1Z2Z3 mount error values */
	SubrSwitcher();
	AngleSubr();

	Current.Alt = H;
	// convert azimuth from CCW to CW
	Current.Az = OneRev - F;
	// if meridianFlipped, then restore 'true' or actual altaz coordinates since input equat and the subsequent
	// coordinate translation is not aware of meridian flip and always results in not flipped coordinate values
	if( GEMflippedFlag)
		TranslateAltazAcrossPole();
	// adjust altitude: this should occur after meridian flip adjustment - see meridian flip note below
	Current.Alt -= Z3;
}

void TranslateAltazAcrossPole( void)
{
	Current.Alt = HalfRev - Current.Alt;
	Current.Az += HalfRev;
	BoundsOneRev( &Current.Az);
}

/*
meridian flip:

if GEM (German Equatorial Mount) is flipped, then flipped Ra differs from original setting circle
Ra by 12 hrs; altitude reading is mirrored across the pole, that is, an alt of 80 is actually 100
(mirrored across 90) as read from the original setting circle orientation;
ie,
northern hemisphere (Az increases as scope tracks, Az=0 when on meridian):
not flipped:
	1 hr west of meridian (Ra < SidT), coord are Ra:(SidT-1hr), Dec:45, Alt:45, Az:15;
	1 hr east of meridian (Ra > SidT), coord are Ra:(SidT+1hr), Dec:45, Alt:45, Az:345 (should be flipped);
same coord flipped (scope assumed to have moved the flip, but aimed back at original equat coord):
	1 hr west of meridian (Ra < SidT), coord are Ra:(SidT-1hr), Dec:45, Alt:135, Az:195 (should be un-flipped);
	1 hr east of meridian (Ra > SidT), coord are Ra:(SidT+1hr), Dec:45, Alt:135, Az:165;
southern hemisphere (Az decreases as scope tracks, Az=0 when on meridian):
not flipped:
	1 hr west of meridian (Ra < SidT), coord are Ra:(SidT-1hr), Dec:-45, Alt:45, Az:345;
	1 hr east of meridian (Ra > SidT), coord are Ra:(SidT+1hr), Dec:-45, Alt:45, Az:15 (should be flipped);
same coord flipped (scope assumed to have moved the flip, but aimed back at original equat coord):
	1 hr west of meridian (Ra < SidT), coord are Ra:(SidT-1hr), Dec:-45, Alt:135, Az:165 (should be un-flipped);
	1 hr east of meridian (Ra > SidT), coord are Ra:(SidT+1hr), Dec:-45, Alt:135, Az:195;

z3 or altitude offset error:
if scope aimed at 70 but setting circles say 60, then z3 = 10;
meridian flipped position: scope aimed at 110 with setting circles indicate 100;

*/

void GetEquat( void)
{
	int I, J;
	double HoldAlt, HoldAz;

	HoldAlt = Current.Alt;
	HoldAz = Current.Az;
	Current.Alt += Z3;
	// return to equivalent not flipped altaz values for purposes of coordinate translation
	if( GEMflippedFlag)
		TranslateAltazAcrossPole();
	H = Current.Alt;
	// convert from CW to CCW az
	F = OneRev - Current.Az;
	Current.Alt = HoldAlt;
	Current.Az = HoldAz;

	SubrA();
	XX[1][1] = YY[1][0];
	XX[2][1] = YY[2][0];
	XX[3][1] = YY[3][0];
	YY[1][1] = 0;
	YY[2][1] = 0;
	YY[3][1] = 0;
	for( I = 1; I <= 3; I++)
		for( J = 1; J <= 3; J++)
			YY[I][1] += (QQ[I][J] * XX[J][1]);
	AngleSubr();
	F += Current.SidT;
	Current.Ra = F;
	ValidRa( &Current);
	Current.Dec = H;
}

void CalcFieldR( void)
{
	double A;
	double SinHA;

	CheckForPostInitVars();

	Current.HA = Current.SidT - HAOff - Current.Ra;
	SinHA = sin( Current.HA);

	CheckHoldSinCosCurrentDec();
	A = SinLatEquatPoleDividedByCosLatEquatPole * HoldCosCurrentDec - HoldSinCurrentDec * cos( Current.HA);

	if( A < 0)
		FieldR = atan( SinHA/A) + M_PI/2;
	else
		if( A == 0)
			if( SinHA < 0)
				FieldR = -M_PI/2;
			else if( SinHA == 0)
				FieldR = 0;
			else
				FieldR = M_PI/2;
		else
			FieldR = atan( SinHA/A);
	if( FieldR < 0)
		FieldR += OneRev;
}

/* rate of field rotation in degrees per minute */
void CalcFieldRate( void)
{
	static double PrevFieldR;
	static double PrevSidT;

	if( Current.SidT > PrevSidT && PrevFieldR != FieldR)
	{
		FieldRate = (FieldR-PrevFieldR)*RadToDeg*MinToRad/(Current.SidT-PrevSidT);
		if( FieldRate > 99.)
			FieldRate = 99;
		else
			if( FieldRate < .01 && FieldRate > -.01)
				FieldRate = 0;
			else
				if( FieldRate < -99.)
					FieldRate = -99;
			PrevFieldR = FieldR;
			PrevSidT = Current.SidT;
	}
}

void CalcAirMass( void)
{
	double AirMassHA;
	double ZenithDistance, SecZenithDistance;

	AirMassHA = Current.SidT - Current.Ra;
	BoundsHalfRev( &AirMassHA);
	CheckHoldSinCosCurrentDec();
	SinAltAirMass = HoldSinCurrentDec * ConfigSinLat +
	HoldCosCurrentDec * ConfigCosLat * cos( AirMassHA);

	if( SinAltAirMass >= -1 && SinAltAirMass <= 1)
	{
		AltAirMass = asin( SinAltAirMass);

		ZenithDistance = QtrRev - AltAirMass;
		if( ZenithDistance > QtrRev)
			ZenithDistance = QtrRev;
		else
			if( ZenithDistance < 0)
				ZenithDistance = 0;

		SecZenithDistance = 1/cos( ZenithDistance);
		AirMass = SecZenithDistance - .0018161*(SecZenithDistance-1) -
		.002875*pow( SecZenithDistance-1, 2) - .0008083*pow( SecZenithDistance-1, 3);
		if( AirMass > 9.9)
			AirMass = 9.9;
		if( AirMass < 1)
      	AirMass = 1;
	}
}

/* relies on CalcAirMass() to be run just before */
void CalcDomeAzimuth( void)
{
	double CosDomeAzimuth;

	CosDomeAzimuth = (HoldSinCurrentDec - ConfigSinLat * SinAltAirMass) /
	(ConfigCosLat * cos( AltAirMass));

	if( CosDomeAzimuth >= -1 && CosDomeAzimuth <= 1)
	{
		DomeAzimuth = acos( CosDomeAzimuth);
		if( sin( Current.SidT - Current.Ra) > 0)
			DomeAzimuth = OneRev - DomeAzimuth;
	}
	else
		DomeAzimuth = 0;
}

/* if scope polar aligned and cfg site vars and computer date/time accurate, then calculate refraction by:
	1. before calling one of these functions get altitude from GetSiteAltaz()
	2. call one of these functions, depending on which direction the conversion is headed
		a. functions calculate two equatorial coordinates, one without and one with refracted altitude
		b. since scope polar aligned, differences in equat coordinates are differences in encoder positions to correct for */

void CalcSiteRefractScopeToSky( void)
{
	double HoldSiteRa, HoldSiteDec, HoldSiteAlt;

	GetSiteEquat();
	HoldSiteDec = SitePos.Dec;
	HoldSiteRa = SitePos.Ra;
	HoldSiteAlt = SitePos.Alt;

	CalcRefractScopeToSky( SitePos.Alt);
	SitePos.Alt -= Refract;
	GetSiteEquat();
	RefractDec = HoldSiteDec - SitePos.Dec;
	RefractRa = HoldSiteRa - SitePos.Ra;

	SitePos.Dec = HoldSiteDec;
	SitePos.Ra = HoldSiteRa;
	SitePos.Alt = HoldSiteAlt;
}

void CalcSiteRefractSkyToScope( void)
{
	double HoldSiteRa, HoldSiteDec, HoldSiteAlt;

	GetSiteEquat();
	HoldSiteDec = SitePos.Dec;
	HoldSiteRa = SitePos.Ra;
	HoldSiteAlt = SitePos.Alt;

	CalcRefractSkyToScope( SitePos.Alt);
	SitePos.Alt += Refract;
	GetSiteEquat();
	RefractDec = SitePos.Dec - HoldSiteDec;
	RefractRa = SitePos.Ra - HoldSiteRa;

	SitePos.Dec = HoldSiteDec;
	SitePos.Ra = HoldSiteRa;
	SitePos.Alt = HoldSiteAlt;
}

/* have equat coordinates and updated SidT, need to get alt-alt-az coordinates with matched field rotation angle:
	our altaz scope is tipped over 90 degrees so that the azimuth axis is aimed at a point on the horizon, typically
	directly under the pole;
	the mount is called an altalt in this situation, where the azimuth axis is renamed to one of the altalt axes;
	if the aim point on the horizon under the pole is allowed to point at any horizon point, then the mount becomes
	a 3 axis mount, or an altaltaz mount, where the 'az' in altaltaz refers to the horizon point's azimuth from the pole,
	ie, if pointed directly under pole, then az=0, if pointed to eastern horizon, then az=90;
	in terms of the traditional altaz mount that becomes an altalt mount by tipping over at 90 deg, it might be better
	to name this 3 axis mount 'altazaz', but the mount is more accurately named altaltaz because this is descriptive of the
	mount's motion with respect to the ground and sky;
	az axis of altaltaz travels at very approx sin(lat) * tracking distance per unit of time (in this case, 1 minute of time
	or .25 deg tracking distance) */

void GetAltAltAz( double MatchFieldRotation)
{
	struct Position StartPos;
	double AzGuess;
	double Incr;
	double FRDiff;
	double LastFRDiff;
	double FRAccuracy;
	struct AZDouble DeltaAZ;
	double DeltaAx3;
	int Iter;

	WriteInitHistoryFile = False;
	StartPos = Current;
	FRAccuracy = TenthsArcsecToRad;
	if( LastDeltaAx3 == 0.)
		Incr = .5 * sin( LatitudeDeg*DegToRad) * MinToRad;
	else
		Incr = LastDeltaAx3;
	LastFRDiff = MAXDOUBLE;
	AzGuess = Current.Ax3;
	Iter = 0;
	do
	{
		AzGuess += Incr;
		// re-initialize altalt with new azimuth as horizon direction that primary axis points to
		InitConvertAltAlt( AzGuess);
		Current.Ra = StartPos.Ra;
		Current.Dec = StartPos.Dec;
		// get altaz (actually altalt aligned mount) based on re-init with new azimuth
		GetAltaz();
		CalcFieldR();
		FRDiff = fabs( MatchFieldRotation - FieldR);
		BoundsHalfRev( &FRDiff);
		// getting worse, so reverse direction and cut increment by half
		if( FRDiff > LastFRDiff)
			Incr = -Incr/2.;
		LastFRDiff = FRDiff;
		Iter++;
	}while( FRDiff > FRAccuracy);

	WriteInitHistoryFile = True;
	DeltaAx3 = Current.Ax3 - StartPos.Ax3;
	BoundsHalfRev( &DeltaAx3);
	LastDeltaAx3 = DeltaAx3;
	DeltaAZ.A = Current.Alt-StartPos.Alt;
	BoundsHalfRev( &DeltaAZ.A);
	DeltaAZ.Z = Current.Az-StartPos.Az;
	BoundsHalfRev( &DeltaAZ.Z);
	printf( "\nholding field rotation angle=                       %f", MatchFieldRotation*RadToDeg);
	printf( "\ndelta deg ax3 (rotation about sky's zenith)=        %f", DeltaAx3*RadToDeg);
	printf( "\ndelta deg az (primary altalt axis aimed at horiz)=  %f", DeltaAZ.Z*RadToDeg);
	printf( "\ndelta deg alt (right angle to primary altalt axis)= %f", DeltaAZ.A*RadToDeg);
	printf( "\niterations %d\n", Iter);
}

void TestAltAltAzTrack( void)
{
	struct AZDouble UserDeg;
	double UserAx3;
	double HoldFieldR;
	int Tracks;
	int UserTracks = 10;

	printf( "\nTest of AltAltAz tracking\n");
	printf( "\nplease enter Ax3 deg (horizon point that the primary axis is aimed towards)\n");
	GetDouble( &UserAx3);
	UserAx3 *= DegToRad;
	printf( "\nplease enter altitude deg\n");
	GetDouble( &UserDeg.A);
	printf( "\nplease enter azimuth deg\n");
	GetDouble( &UserDeg.Z);

	NewSidT();
	InitConvertAltAlt( UserAx3);
	SitePos.Alt = UserDeg.A*DegToRad;
	SitePos.Az = UserDeg.Z*DegToRad;
	SitePos.SidT = Current.SidT;
	GetSiteEquat();
	Current.Ra = SitePos.Ra;
	Current.Dec = SitePos.Dec;
	GetAltaz();
	CalcFieldR();
	HoldFieldR = FieldR;

	printf( "\ndistances to move for 10 consecutive one minute intervals...");
	for( Tracks = 0; Tracks < UserTracks; Tracks++)
	{
		Current.SidT += MinToRad;
		GetAltAltAz( HoldFieldR);
		printf( "...");
	}
	ContMsgRoutine();
	_exit( 0);
}

void TestConvert( void)
{
	struct Position Hold;

	printf( "\n\n\nTest of Convert code...\n");
	One.Init = Two.Init = Three.Init = No;
	strcpy( WhyInit, WHY_INIT_TEST);
	SetMountErrorsDeg( -.04, .4, -1.63);
	SetCoordDeg( &Current, 79.172, 45.998, 39.9, 360-39.9, 39.2*SidRate/4);
	InitMatrix( 1);
	SetCoordDeg( &Current, 37.96, 89.264, 36.2, 360-94.6, 40.3*SidRate/4);
	InitMatrix( 2);
	SetCoordDeg( &Current, 326.05, 9.88, 0, 0, 47*SidRate/4);
	GetAltaz();
	ShowCoord( &Current);
	printf( "\nShould be Alt: 42.16   Az: %7.3f", 360-202.54);
	SetCoordDeg( &Current, 71.53, 17.07, 0, 0, 62*SidRate/4);
	GetAltaz();
	ShowCoord( &Current);
	printf( "\nShould be Alt: 40.31   Az: %7.3f", 360-359.98);
	SetCoordDeg( &Current, 0, 0, 35.5, 360-24.1, 71.9*SidRate/4);
	GetEquat();
	ShowCoord( &Current);
	printf( "\nShould be Ra: 87.99   Dec: 32.51\n");

	printf( "\ntesting hysteresis");
	SetCoordDeg( &Current, 0, 0, 60, 100, 47*SidRate/4);
	GetEquat();
	ShowCoord( &Current);
	Hold = Current;
	GetAltaz();
	ShowCoord( &Current);
	printf( "\nalt error arcsec %f, az error arcsec %f\n",
			 (Current.Alt-Hold.Alt)*RadToArcsec, (Current.Az-Hold.Az)*RadToArcsec);

	// make sure that analysis.taki is copied to analysis.dat prior to running
	printf( "\ntesting BestZ123: calculating...");
	ComputeBestZ123FromAnalysisFile();
	printf( "\nBestZ1Deg %3.3f BestZ2 %3.3f BestZ3Deg %3.3f RMS(arcmin) %3.3f\n",
			BestZ1*RadToDeg, BestZ2*RadToDeg, BestZ3*RadToDeg, BestPointErrRMS*RadToArcmin);

	ContMsgRoutine();
	_exit( 0);
}
