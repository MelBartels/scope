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

/* IACA area... */

/* the Intra-Application Communication Area is a 16 byte area starting at 0040:00F0 that DOS sets
aside to allow programs to communicate with each other;

this area is used to exchange coordinates between the scope control program and graphically oriented
CD-ROM planetarium programs such as Guide;

the planetarium program's coordinates serve as input to the scope, the scope program tells the
planetarium program where the scope is pointing;

coordinates are assumed to be precessed to the current date;

each program, at startup, sets all coordinates initially to zero: this way, when non-zero
coordinates are found, it can be assumed that the other program put them there;

ie,
long iaca_ra = *(long far*) (0x4f0);
long iaca_dec = *(long far*) (0x4f4);
long scope_ra = *(long far*) (0x4f8);
long scope_dec = *(long far*) (0x4fc);
double scope_ra_in_decimal_hours = (double) scope_ra / 1.e+7;
double scope_dec_in_decimal_degrees = (double) scope_dec / 1.e+7; */

void InitIACA( void)
{
	Conv_Factor = 1.e+7;

	IACA_Ra_Decimal_Hrs_Ptr = (long far*) 0x4f0;
	IACA_Dec_Decimal_Degrees_Ptr = (long far*) 0x4f4;
	Scope_Ra_Decimal_Hrs_Ptr = (long far*) 0x4f8;
	Scope_Dec_Decimal_Degrees_Ptr = (long far*) 0x4fc;

	*IACA_Ra_Decimal_Hrs_Ptr = 0;
	*IACA_Dec_Decimal_Degrees_Ptr = 0;
	*Scope_Ra_Decimal_Hrs_Ptr = 0;
	*Scope_Dec_Decimal_Degrees_Ptr = 0;
}

void PutScope( double RaRad, double DecRad)
{
	*Scope_Ra_Decimal_Hrs_Ptr= (long) (RaRad * RadToHr * Conv_Factor);
	*Scope_Dec_Decimal_Degrees_Ptr= (long) (DecRad * RadToDeg * Conv_Factor);
}

int NewIACA( void)
{
	static long HoldRa, HoldDec;

	if( HoldRa != *IACA_Ra_Decimal_Hrs_Ptr)
	{
		HoldRa = *IACA_Ra_Decimal_Hrs_Ptr;
		return True;
	}
	else
		if( HoldDec != *IACA_Dec_Decimal_Degrees_Ptr)
		{
			HoldDec = *IACA_Dec_Decimal_Degrees_Ptr;
			return True;
		}
		else
			return False;
}

void GetIACA( double* RaRad, double* DecRad)
{
	*RaRad = (double) *IACA_Ra_Decimal_Hrs_Ptr / (RadToHr*Conv_Factor);
	*DecRad = (double) *IACA_Dec_Decimal_Degrees_Ptr / (RadToDeg*Conv_Factor);
}

/*
void TestIACA( void)
{
	double Ra, Dec;

	printf( "\n\n\nStarting IACA module test:");
	printf( "\n     (Ra in decimal hours and Dec in decimal degrees)");
	do
	{
		printf( "\nPlease enter Scope Ra ");
		GetDouble( &Ra);
		printf( "   Dec ");
		GetDouble( &Dec);
		PutScope( Ra*HrToRad, Dec*DegToRad);
		printf( "Scope Ra = %f", (double) *Scope_Ra_Decimal_Hrs_Ptr/Conv_Factor);
		printf( " Dec = %f", (double) *Scope_Dec_Decimal_Degrees_Ptr/Conv_Factor);

		GetIACA( &Ra, &Dec);
		printf( "\nIACA Ra = %f Dec = %f", Ra*RadToHr, Dec*RadToDeg);
		printf( "\nInject new IACA Ra Dec? ");

		Response = getch();
		if( Response=='y' || Response=='Y')
		{
			printf( "Enter new IACA Ra ");
			GetDouble( &Ra);
			printf( " Dec ");
			GetDouble( &Dec);
			*IACA_Ra_Decimal_Hrs_Ptr = (long) (Ra*Conv_Factor);
			*IACA_Dec_Decimal_Degrees_Ptr = (long) (Dec*Conv_Factor);
			GetIACA( &Ra, &Dec);
			printf( "IACA Ra = %f Dec = %f", Ra*RadToHr, Dec*RadToDeg);
		}
		printf( "\nAnother go (y/n)? ");
		Response = getch();
	} while( Response=='y' || Response=='Y');
	ContMsgRoutine();
}
*/
