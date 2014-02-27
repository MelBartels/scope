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

Flag WriteGuideStartup( void)
{
	char B[3];
	int Ix;

	/* read in file, saving lines from third on */
	Input = fopen( GuideStartupMarFilePtr, "r");
	if( Input == NULL)
	{
		PressKeyToContMsg( strcat( "WriteGuideStartup(): Could not open ", GuideStartupMarFilePtr));
		return False;
	}
	else
	{
		Output = fopen( ScopeStartupMarFilePtr, "w");
		if( Output == NULL)
			BadExit( strcat( "WriteGuideStartup(): Could not open ", ScopeStartupMarFilePtr));

		/* fill Buffer */
		for( Ix = 0; Ix < 3; Ix++)
			fread( &B[Ix], 1, 1, Input);

		/* read/write until 'ra ' by writing out left most char of buffer, then left shifting buffer,
		then reading in right most char of buffer */
		while( B[0] != 'r' || B[1] != 'a' || B[2] != ' ')
		{
			fprintf( Output, "%c", B[0]);
			B[0] = B[1];
			B[1] = B[2];
			fread( &B[2], 1, 1, Input);
		}
		/* write Ra, Dec */
		fprintf( Output, "ra       %f\n", 360. - Current.Ra*RadToDeg);
		fprintf( Output, "2  dec      %f\n", Current.Dec*RadToDeg);
		/* throw away these two lines since these lines have been replaced with new coordinates */
		FReadToNewLine( Input);
		FReadToNewLine( Input);
		/* read/write until eof */
		while( !feof( Input))
		{
			fread( &B[0], 1, 1, Input);
			fprintf( Output, "%c", B[0]);
		}
		fclose( Output);
		fclose( Input);
		return True;
	}
}

void ReadGuideStartup( void)
{
	double RaDeg, DecDeg;
	struct HMSH RaHMSH;
	struct DMS DecDMS;
	Flag RaFlag = No;
	Flag DecFlag = No;


	Input = fopen( GuideStartupMarFilePtr, "r");
	if( Input == NULL)
		BadExit( strcat( "ReadGuideStartup(): Could not open ", GuideStartupMarFilePtr));

	fscanf( Input, "%s", Name);
	while( !feof( Input) && !RaFlag || !DecFlag)
	{
		if( (strncmpi( Name, "ra", sizeof( Name))) == 0)
		{
			FReadDouble( Input, &RaDeg);
			RaDeg = 360. - RaDeg;
			RaFlag = Yes;
		}
		else if( (strncmpi( Name, "dec", sizeof( Name))) == 0)
		{
			FReadDouble( Input, &DecDeg);
			DecFlag = Yes;
		}
		fscanf( Input, "%s", Name);
	}
	fclose( Input);
	if( RaFlag && DecFlag)
	{
		In.Ra = RaDeg*DegToRad;
		In.Dec = DecDeg*DegToRad;
		DisplayIn( "from Guide", NameBlanks);

		/* write to OutGuideFile, preserving a record of all positions returned from Guide */
		GetHMSH( In.Ra*RadToHundSec + .5, &RaHMSH);
		GetDMS( In.Dec*RadToArcsec + .5, &DecDMS);
		if( In.Dec < 0)
			if( DecDMS.Deg > 0)
				DecDMS.Deg = -DecDMS.Deg;
			else
				if( DecDMS.Min > 0)
					DecDMS.Min = -DecDMS.Min;
				else
					DecDMS.Sec = -DecDMS.Sec;

		fprintf( OutGuideFilePtr, "%3d %3d %3d   %3d %3d %3d   from_guide\n", RaHMSH.Hr, RaHMSH.Min,
		RaHMSH.Sec, DecDMS.Deg, DecDMS.Min, DecDMS.Sec);
	}
}

