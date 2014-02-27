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

/* Guide Area... */

void InitGuide( void)
{
	GuideFlag = Yes;
	GuideRadTick = GuideArcsecSec*ArcsecToRad/ClockTicksSec;
	AccumGuide.A = AccumGuide.Z = 0;
}

void StopGuide( void)
{
	GuideFlag = Off;
	AccumGuide.A = AccumGuide.Z = 0;
}

void InitSaveGuideArray( void)
{
	GuideIx.A = GuideIx.Z = 0;
	for( Ix = 0; Ix < MaxGuideIx; Ix++)
	{
		Guides[Ix].A.PECIx = Guides[Ix].Z.PECIx = 0;
		Guides[Ix].A.TenthsArcsec = Guides[Ix].Z.TenthsArcsec = 0;
	}
	GuideDir.A = GuideDir.Z = NoRotation;
	PECIxCrossZero.A = PECIxCrossZero.Z = 0;

	GuideArrayFlag = WritingToGuideArray;
	WriteHandpadStatus();
}

void SaveGuideArray( void)
{
	SaveGuideAlt();
	SaveGuideAz();
	GuideArrayFlag = No;
	WriteHandpadStatus();
}

void SaveGuideAlt( void)
{
	int Ix;

	Output = fopen( GuideAltFile, "w");
	if( Output == NULL)
		BadExit( strcat( "Could not open ", GuideAltFile));

	for( Ix = 0; Ix < GuideIx.A; Ix++)
		fprintf( Output, "%d %d\n", (int) Guides[Ix].A.PECIx, Guides[Ix].A.TenthsArcsec);

	fclose( Output);
}

void SaveGuideAz( void)
{
	int Ix;

	Output = fopen( GuideAzFile, "w");
	if( Output == NULL)
		BadExit( strcat( "Could not open ", GuideAzFile));

	for( Ix = 0; Ix < GuideIx.Z; Ix++)
		fprintf( Output, "%d %d\n", (int) Guides[Ix].Z.PECIx, Guides[Ix].Z.TenthsArcsec);

	fclose( Output);
}

/* each entry in the guide array contains the PEC index and accumulated guiding corrections; it is
possible for successive entries in the guide array to skip over PEC indexes since the motor may have
rotated more than the angular distance between PEC indexes */

void WriteToGuideArray( void)
{
	static struct AZInt HoldPECIx;
	static struct AZInt Reps;
	static Flag soundon;

	if( soundon)
		nosound();

	if( PECIx.A == HoldPECIx.A)
	/* average in new entry */
	{
		Reps.A++;
		Guides[GuideIx.A].A.TenthsArcsec =
		(Guides[GuideIx.A].A.TenthsArcsec * Reps.A +	AccumGuide.A*RadToTenthsArcsec) / (Reps.A+1);
	}
	else
	/* write new entry in the guide array */
	{
		/* make sound when PECIx.A goes through zero */
		if(( Dir.A == CW && (PECIx.A < HoldPECIx.A))
		|| ( Dir.A == CCW && (PECIx.A > HoldPECIx.A)))
		{
			sound( 1000);
			soundon = Yes;
		}
		/* write an entry in the guide array */
		Guides[GuideIx.A].A.PECIx = PECIx.A;
		Guides[GuideIx.A].A.TenthsArcsec = AccumGuide.A*RadToTenthsArcsec;
		HoldPECIx.A = PECIx.A;
		Reps.A = 0;
		GuideIx.A++;
	}

	if( PECIx.Z == HoldPECIx.Z)
	/* average in new entry */
	{
		Reps.Z++;
		Guides[GuideIx.Z].Z.TenthsArcsec =
		(Guides[GuideIx.Z].Z.TenthsArcsec * Reps.Z +	AccumGuide.Z*RadToTenthsArcsec) / (Reps.Z+1);
	}
	else
	/* write new entry in the guide array */
	{
		/* make sound when PECIx.Z goes through zero */
		if(( Dir.Z == CW && (PECIx.Z < HoldPECIx.Z))
		|| ( Dir.Z == CCW && (PECIx.Z > HoldPECIx.Z)))
		{
			sound( 500);
			soundon = Yes;
		}
		/* write an entry in the guide array */
		Guides[GuideIx.Z].Z.PECIx = PECIx.Z;
		Guides[GuideIx.Z].Z.TenthsArcsec = AccumGuide.Z*RadToTenthsArcsec;
		HoldPECIx.Z = PECIx.Z;
		Reps.Z = 0;
		GuideIx.Z++;
	}

	if( GuideIx.A >= MaxGuideIx || GuideIx.Z >= MaxGuideIx)
	{
		sound( 250);
		delay( 50);
		nosound();
		GuideArrayFlag = ReadyToSaveGuideArray;
	}
}

void LoadGuideAlts( void)
{
	char Index[ValueSize];

	Input = fopen( GuideAltFile, "r");
	if( Input == NULL)
		printf( "\nDid not find %s", GuideAltFile);
	else
	{
		printf( "\nLoading %s", GuideAltFile);
		GuideIx.A = 0;
	}
	if( Input != NULL)
	{
		fscanf( Input, "%s %s", Index, Value);
		while( !feof( Input))
		{
			Ix = atoi( Index);
			TenthsArcsec = atoi( Value);
			Guides[GuideIx.A].A.PECIx = Ix;
			Guides[GuideIx.A].A.TenthsArcsec = TenthsArcsec;
			GuideIx.A++;
			fscanf( Input, "%s %s", Index, Value);
		}
	}
	fclose( Input);
}

void LoadGuideAzs( void)
{
	char Index[ValueSize];

	Input = fopen( GuideAzFile, "r");
	if( Input == NULL)
		printf( "\nDid not find %s", GuideAzFile);
	else
	{
		printf( "\nLoading %s", GuideAzFile);
		GuideIx.Z = 0;
	}
	if( Input != NULL)
	{
		fscanf( Input, "%s %s", Index, Value);
		while( !feof( Input))
		{
			Ix = atoi( Index);
			TenthsArcsec = atoi( Value);
			Guides[GuideIx.Z].Z.PECIx = Ix;
			Guides[GuideIx.Z].Z.TenthsArcsec = TenthsArcsec;
			GuideIx.Z++;
			fscanf( Input, "%s %s", Index, Value);
		}
	}
	fclose( Input);
}

void AnalyzeGuideAltArray( void)
{
	int cwcount = 0;
	int ccwcount = 0;
	double xscale, yscale;
	int maxh = 0;
	int guidey;
	int guidepecy;
	int LastGuideIx;
	int IxB;
	double Drift;
	int Offset;
	int FileCount;
	int FileNameSize;
	const int linecolor = RED;
	const int guidecolor = GREEN;
	const int driftcolor = YELLOW;

	CloseMouseControl();
	InitGraphics();
	guidey = 2 * maxy / 4;
	xscale = (double) maxx / (double) MaxGuideIx;

	gotoxy( 1, 1);
	printf( "altitude guiding");

	/* determine direction of motion while Guides was recorded */
	Ix = 1;
	while( Ix < MaxGuideIx && abs( cwcount - ccwcount) < 3)
	{
		if( Guides[Ix+1].A.PECIx > Guides[Ix].A.PECIx)
			cwcount++;
		else
			ccwcount++;
		Ix++;
	}
	if( cwcount > ccwcount)
		GuideDir.A = CW;
	else
		GuideDir.A = CCW;
	if( GuideDir.A == CW)
		printf( " - CW motor rotation\n");
	else
		printf( " - CCW motor rotation\n");

	printf( "top graph plots %s: PECIx=0 is shown by short vertical lines (+-1\")\n", GuideAltFile);
	printf( "first set in bottom graph plots guiding corrections per each PEC cycle\n");
	printf( "while second set is corrected for drift and offset");

	/* draw guide reference line */
	for( Ix = 0; Ix <= maxx; Ix++)
		putpixel( Ix, guidey, linecolor);

	/* set maxh to greatest deviation from zero */
	for( Ix = 0; Ix < GuideIx.A; Ix++)
	{
		if( Guides[Ix].A.TenthsArcsec > maxh)
			maxh = Guides[Ix].A.TenthsArcsec;
		if( Guides[Ix].A.TenthsArcsec < -maxh)
			maxh = -Guides[Ix].A.TenthsArcsec;
	}
	if( maxh == 0)
		yscale = 1;
	else
		yscale = (double) (maxy/8) / (double) maxh;

	/* plot guiding curve */
	for( Ix = 0; Ix < GuideIx.A; Ix++)
		putpixel( Ix * xscale, guidey - Guides[Ix].A.TenthsArcsec * yscale,
		guidecolor);

	/* mark PECIx of 0 */
	for( Ix = 1; Ix < GuideIx.A; Ix++)
		/* check for PECIx of 0 crossing, ie, 198,199,0,1 or 1,0,199,198 but not 80,81,80,81 */
		if( (GuideDir.A == CW && Guides[Ix-1].A.PECIx > Guides[Ix].A.PECIx + PECSize/2) ||
		(GuideDir.A == CCW && Guides[Ix-1].A.PECIx < Guides[Ix].A.PECIx - PECSize/2))
			/* draw vertical line */
			for( guidepecy = guidey - 10*yscale; guidepecy < guidey + 10*yscale; guidepecy++)
				putpixel( Ix * xscale, guidepecy, linecolor);

	/* allocate memory for PEC array */
	P = (struct PECWORK*) malloc( PECSize * sizeof( struct PECWORK));

	/* draw guidepec reference line */
	guidepecy = 3 * maxy / 4;
	for( Ix = 0; Ix <= maxx; Ix++)
		putpixel( Ix, guidepecy, linecolor);

	PECIxCrossZero.A = 0;
	/* run through Guides array again and preserve GuideIx since it is the marker for the end of
	valid Guides array values */
	LastGuideIx = GuideIx.A;
	for( IxB = 1; IxB < LastGuideIx; IxB++)
		/* check for PECIx of 0 crossing, ie, 198,199,0,1 or 1,0,199,198 but not 80,81,80,81 */
		if( (GuideDir.A == CW && Guides[IxB-1].A.PECIx > Guides[IxB].A.PECIx + PECSize/2) ||
		(GuideDir.A == CCW && Guides[IxB-1].A.PECIx < Guides[IxB].A.PECIx - PECSize/2))
		{
			PECIxCrossZero.A++;
			if( PECIxCrossZero.A == 1)
				StartGuidesIxCrossZeroPECIx.A = EndGuidesIxCrossZeroPECIx.A = IxB;
			else
				StartGuidesIxCrossZeroPECIx.A = EndGuidesIxCrossZeroPECIx.A;
			if( PECIxCrossZero.A > 1)
			{
				EndGuidesIxCrossZeroPECIx.A = IxB;

				/* zero out working array */
				for( Ix = 0; Ix < PECSize; Ix++)
				{
					P[Ix].TenthsArcsec = 0;
					P[Ix].Entry = No;
				}

				GuideIx.A = StartGuidesIxCrossZeroPECIx.A;
				/* fill working PEC array with values from Guides[] */
				while( GuideIx.A != EndGuidesIxCrossZeroPECIx.A)
				{
					/* GuideIx.A is index into Guides array (the record of guiding corrections, where the
					PEC index and accumulated guiding amount is stored);
					if motor moving CCW, then PEC index starts at max and decreases as Guides array is
					stepped through ascending from StartGuidesIxCrossZeroPECIx. to EndGuidesIxCrossZeroPECIx. */
					Ix = Guides[GuideIx.A].A.PECIx;
					P[Ix].TenthsArcsec = Guides[GuideIx.A].A.TenthsArcsec;
					P[Ix].Entry = Yes;
					GuideIx.A++;
				}

				/* fill in missing values: if value is unentered, then use value from previous entry */
				if( P[0].Entry == No)
					P[0].TenthsArcsec = 0;
				for( Ix = 1; Ix < PECSize; Ix++)
					if( P[Ix].Entry == No)
						P[Ix].TenthsArcsec = P[Ix-1].TenthsArcsec;

				/* display working PEC array that contains values from Guides[] */
				for( Ix = 0; Ix < PECSize; Ix++)
					putpixel( Ix * xscale, guidepecy - P[Ix].TenthsArcsec * yscale,
					guidecolor);

				/* compensate for drift */
				Drift = P[PECSize-1].TenthsArcsec - P[0].TenthsArcsec;
				for( Ix = 0; Ix < PECSize; Ix++)
					P[Ix].TenthsArcsec -= ((double) Ix / (double) PECSize) * Drift;

				/* eliminate beginning offset */
				Offset = P[0].TenthsArcsec;
				for( Ix = 0; Ix < PECSize; Ix++)
					P[Ix].TenthsArcsec -= Offset;

				/* display working PEC array that contains values from Guides[] compensated for drift */
				for( Ix = 0; Ix < PECSize; Ix++)
					putpixel( (PECSize + Ix) * xscale,
					guidepecy - P[Ix].TenthsArcsec * yscale, driftcolor);

				printf( "\nsave %1d PEC cycle? (y/n) ", PECIxCrossZero.A - 1);
				GetResponseWithLX200Check();
				printf( "%c", Response);
				if( Response == 'Y' || Response == 'y' || Response == Return)
				{
					FileCount = FindNextIncrFilename( PECALT, TXT);
					strcpy( Fname, PECALT);
					FileNameSize = sizeof( PECALT) - 1;
					Fname[FileNameSize] = '0' + FileCount / 10;
					Fname[FileNameSize+1] = '0' + FileCount - (FileCount / 10) * 10;
					Fname[FileNameSize+2] = '\0';
					strcat( Fname, DOT_TXT);
					printf( " %s", Fname);

					Output = fopen( Fname, "w");
					if( Output == NULL)
						BadExit( strcat( "Could not open ", Fname));
					for( Ix = 0; Ix < PECSize; Ix++)
						fprintf( Output, "%d %d\n", Ix, P[Ix].TenthsArcsec);
					fclose( Output);
				}
		}
	}
	printf( "\npress any key to continue");
	getch();
	free( P);
	/* restore GuideIx since it marks the end of valid Guides values */
	GuideIx.A = LastGuideIx;
	CloseGraphics();
	_setcursortype( _NOCURSOR);
	InitMouseControl();
}

void AnalyzeGuideAzArray( void)
{
	int cwcount = 0;
	int ccwcount = 0;
	double xscale, yscale;
	int maxh = 0;
	int guidey;
	int guidepecy;
	int LastGuideIx;
	int IxB;
	double Drift;
	int Offset;
	int FileCount;
	int FileNameSize;
	const int linecolor = RED;
	const int guidecolor = GREEN;
	const int driftcolor = YELLOW;

	CloseMouseControl();
	InitGraphics();
	guidey = 2 * maxy / 4;
	xscale = (double) maxx / (double) MaxGuideIx;

	gotoxy( 1, 1);
	printf( "azimuth guiding");

	/* determine direction of motion while Guides was recorded */
	Ix = 1;
	while( Ix < MaxGuideIx && abs( cwcount - ccwcount) < 3)
	{
		if( Guides[Ix+1].Z.PECIx > Guides[Ix].Z.PECIx)
			cwcount++;
		else
			ccwcount++;
		Ix++;
	}
	if( cwcount > ccwcount)
		GuideDir.Z = CW;
	else
		GuideDir.Z = CCW;
	if( GuideDir.Z == CW)
		printf( " - CW motor rotation\n");
	else
		printf( " - CCW motor rotation\n");

	printf( "top graph plots %s: PECIx=0 is shown by short vertical lines (+-1\")\n", GuideAzFile);
	printf( "first set in bottom graph plots guiding corrections per each PEC cycle\n");
	printf( "while second set is corrected for drift and offset");

	/* draw guide reference line */
	for( Ix = 0; Ix <= maxx; Ix++)
		putpixel( Ix, guidey, linecolor);

	/* set maxh to greatest deviation from zero */
	for( Ix = 0; Ix < GuideIx.Z; Ix++)
	{
		if( Guides[Ix].Z.TenthsArcsec > maxh)
			maxh = Guides[Ix].Z.TenthsArcsec;
		if( Guides[Ix].Z.TenthsArcsec < -maxh)
			maxh = -Guides[Ix].Z.TenthsArcsec;
	}
	if( maxh == 0)
		yscale = 1;
	else
		yscale = (double) (maxy/8) / (double) maxh;

	/* plot guiding curve */
	for( Ix = 0; Ix < GuideIx.Z; Ix++)
		putpixel( Ix * xscale, guidey - Guides[Ix].Z.TenthsArcsec * yscale,
		guidecolor);

	/* mark PECIx of 0 */
	for( Ix = 1; Ix < GuideIx.Z; Ix++)
		/* check for PECIx of 0 crossing, ie, 198,199,0,1 or 1,0,199,198 but not 80,81,80,81 */
		if( (GuideDir.Z == CW && Guides[Ix-1].Z.PECIx > Guides[Ix].Z.PECIx + PECSize/2) ||
		(GuideDir.Z == CCW && Guides[Ix-1].Z.PECIx < Guides[Ix].Z.PECIx - PECSize/2))
			/* draw vertical line */
			for( guidepecy = guidey - 10*yscale; guidepecy < guidey + 10*yscale; guidepecy++)
				putpixel( Ix * xscale, guidepecy, linecolor);

	/* allocate memory for PEC array */
	P = (struct PECWORK*) malloc( PECSize * sizeof( struct PECWORK));

	/* draw guidepec reference line */
	guidepecy = 3 * maxy / 4;
	for( Ix = 0; Ix <= maxx; Ix++)
		putpixel( Ix, guidepecy, linecolor);

	PECIxCrossZero.Z = 0;
	/* run through Guides array again and preserve GuideIx since it is the marker for the end of
	valid Guides array values */
	LastGuideIx = GuideIx.Z;
	for( IxB = 1; IxB < LastGuideIx; IxB++)
		/* check for PECIx of 0 crossing, ie, 198,199,0,1 or 1,0,199,198 but not 80,81,80,81 */
		if( (GuideDir.Z == CW && Guides[IxB-1].Z.PECIx > Guides[IxB].Z.PECIx + PECSize/2) ||
		(GuideDir.Z == CCW && Guides[IxB-1].Z.PECIx < Guides[IxB].Z.PECIx - PECSize/2))
		{
			PECIxCrossZero.Z++;
			if( PECIxCrossZero.Z == 1)
				StartGuidesIxCrossZeroPECIx.Z = EndGuidesIxCrossZeroPECIx.Z = IxB;
			else
				StartGuidesIxCrossZeroPECIx.Z = EndGuidesIxCrossZeroPECIx.Z;
			if( PECIxCrossZero.Z > 1)
			{
				EndGuidesIxCrossZeroPECIx.Z = IxB;

				/* zero out working array */
				for( Ix = 0; Ix < PECSize; Ix++)
				{
					P[Ix].TenthsArcsec = 0;
					P[Ix].Entry = No;
				}

				GuideIx.Z = StartGuidesIxCrossZeroPECIx.Z;
				/* fill working PEC array with values from Guides[] */
				while( GuideIx.Z != EndGuidesIxCrossZeroPECIx.Z)
				{
					/* GuideIx.Z is index into Guides array (the record of guiding corrections, where the
					PEC index and accumulated guiding amount is stored);
					if motor moving CCW, then PEC index starts at max and decreases as Guides array is
					stepped through ascending from StartGuidesIxCrossZeroPECIx. to EndGuidesIxCrossZeroPECIx. */
					Ix = Guides[GuideIx.Z].Z.PECIx;
					P[Ix].TenthsArcsec = Guides[GuideIx.Z].Z.TenthsArcsec;
					P[Ix].Entry = Yes;
					GuideIx.Z++;
				}

				/* fill in missing values: if value is unentered, then use value from previous entry */
				if( P[0].Entry == No)
					P[0].TenthsArcsec = 0;
				for( Ix = 1; Ix < PECSize; Ix++)
					if( P[Ix].Entry == No)
						P[Ix].TenthsArcsec = P[Ix-1].TenthsArcsec;

				/* display working PEC array that contains values from Guides[] */
				for( Ix = 0; Ix < PECSize; Ix++)
					putpixel( Ix * xscale, guidepecy - P[Ix].TenthsArcsec * yscale,
					guidecolor);

				/* compensate for drift */
				Drift = P[PECSize-1].TenthsArcsec - P[0].TenthsArcsec;
				for( Ix = 0; Ix < PECSize; Ix++)
					P[Ix].TenthsArcsec -= ((double) Ix / (double) PECSize) * Drift;

				/* eliminate beginning offset */
				Offset = P[0].TenthsArcsec;
				for( Ix = 0; Ix < PECSize; Ix++)
					P[Ix].TenthsArcsec -= Offset;

				/* display working PEC array that contains values from Guides[] compensated for drift */
				for( Ix = 0; Ix < PECSize; Ix++)
					putpixel( (PECSize + Ix) * xscale,
					guidepecy - P[Ix].TenthsArcsec * yscale, driftcolor);

				printf( "\nsave %1d PEC cycle? (y/n) ", PECIxCrossZero.Z - 1);
				GetResponseWithLX200Check();
				printf( "%c", Response);
				if( Response == 'Y' || Response == 'y' || Response == Return)
				{
					FileCount = FindNextIncrFilename( PECAZ, TXT);
					strcpy( Fname, PECAZ);
					FileNameSize = sizeof( PECAZ) - 1;
					Fname[FileNameSize] = '0' + FileCount / 10;
					Fname[FileNameSize+1] = '0' + FileCount - (FileCount / 10) * 10;
					Fname[FileNameSize+2] = '\0';
					strcat( Fname, DOT_TXT);
					printf( " %s", Fname);

					Output = fopen( Fname, "w");
					if( Output == NULL)
						BadExit( strcat( "Could not open ", Fname));
					for( Ix = 0; Ix < PECSize; Ix++)
						fprintf( Output, "%d %d\n", Ix, P[Ix].TenthsArcsec);
					fclose( Output);
				}
		}
	}
	printf( "\npress any key to continue");
	getch();
	free( P);
	/* restore GuideIx since it marks the end of valid Guides values */
	GuideIx.Z = LastGuideIx;
	CloseGraphics();
	_setcursortype( _NOCURSOR);
	InitMouseControl();
}

