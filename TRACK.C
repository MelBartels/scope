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

void SetMsParmVidMemXY( const int Parm)
{
	if( Parm < DisplayPWM)
	{
		VidMemXY.Y = MsParmsXY.Y + 1 + Parm/7;
		VidMemXY.X = MsParmsXY.X + MsParmWidthX-5 + MsParmWidthX*(Parm%7);
	}
	else
	{
		VidMemXY.Y = MsParmsXY.Y + 2 + ((Parm-DisplayPWM)/2)/8;
		VidMemXY.X = MsParmsXY.X + MsParmPWMWidthX-6 + MsParmPWMWidthX*(((Parm-DisplayPWM)/2)%8);
		if( (Parm-DisplayPWM)%2)
			VidMemXY.X+=3;
	}
}

void UpdateResponseMsParm( void)
{
	SetMsParmVidMemXY( ResponseMsParmIx);
	for( Ix = 0; Ix < 3; Ix++)
		Screen[VidMemXY.Y][VidMemXY.X++].Attr = DisplayText;
	SetMsParmVidMemXY( NewResponseMsParmIx);
	for( Ix = 0; Ix < 3; Ix++)
		Screen[VidMemXY.Y][VidMemXY.X++].Attr = SelectText;
	ResponseMsParmIx = NewResponseMsParmIx;
}

void ProcessMsParmsResponse( void)
{
	int OldMaxPWMIx;

	switch( Response)
	{
		case ExtendedKeyboardStroke:
			Response = getch();
			if( Response == LeftCursor)
			{
				NewResponseMsParmIx--;
				if( NewResponseMsParmIx < 0)
					NewResponseMsParmIx = DisplayPWM + Ms*2 - 1;
				UpdateResponseMsParm();
			}
			else
			{
				if( Response == RightCursor)
				{
					NewResponseMsParmIx++;
					if( NewResponseMsParmIx == DisplayPWM + Ms*2)
						NewResponseMsParmIx = 0;
					UpdateResponseMsParm();
				}
				else
					if( Response == UpCursor || Response == DownCursor)
						ProcessMsParmChange();
			}
			break;
		case PlusKey:
			IncrTrackStep = 1;
			break;
		case MinusKey:
			IncrTrackStep = -1;
			break;
		case 'd':
		case 'D':
			if( Ms <= MaxMs/2)
			{
				OldMaxPWMIx = Ms - 1;
				Ms *= 2;
				CalcVarsRelatingToMs();
				FreeMsArrays();
				CreateMsArrays();
				InitMsArrays();
				for( Ix = OldMaxPWMIx; Ix > 0; Ix--)
				{
					PWM[Ix*2].A = PWM[Ix].A;
					PWM[Ix*2].Z = PWM[Ix].Z;
				}
				for( Ix = 1; Ix < Ms-2; Ix+=2)
				{
					PWM[Ix].A = (PWM[Ix-1].A + PWM[Ix+1].A)/2;
					PWM[Ix].Z = (PWM[Ix-1].Z + PWM[Ix+1].Z)/2;
				}
				PWM[Ms-1].A = PWM[Ms-2].A/2;
				PWM[Ms-1].Z = (PWM[Ms-2].Z+MaxPWM)/2;
				UseComplexPWMFlag = Yes;
				MaxIncrMsPerPWM *= 2;
				MsHsToggleIncrMsPerPWM *= 2;
				DisplayMsParmsTitlesAndValues();
			}
			break;
		case DisplayMaxIncrMsPerPWM:
			if( Response == UpCursor)
			{
				MaxIncrMsPerPWM++;
				DisplayMsParmsTitlesAndValues();
			}
			break;
		case 'q':
		case 'Q':
			MsParmsAllowedFlag = False;
			break;
		/* allow quitting of motor track by rate and accel, which will also quit Ms parms adjust */ 
		case 't':
		case 'T':
			MsParmsAllowedFlag = False;
			TrackByRateFlag = No;
			MsZeroSoundOn = No;
         nosound();
			WriteTrackByRate();
	}
}

void ProcessMsParmChange( void)
{
	if( NewResponseMsParmIx >= DisplayPWM)
	{
		Ix = (NewResponseMsParmIx - DisplayPWM)/2;
		if( Response == DownCursor)
		{
			if( (NewResponseMsParmIx - DisplayPWM)%2)
			{
				PWM[Ix].Z--;
				if( PWM[Ix].Z < 0)
					PWM[Ix].Z = 0;
				UseComplexPWMFlag = Yes;
			}
			else
			{
				PWM[Ix].A--;
				if( PWM[Ix].A < 0)
					PWM[Ix].A = 0;
			}
		}
		else
		{
			if( (NewResponseMsParmIx - DisplayPWM)%2)
			{
				PWM[Ix].Z++;
				if( PWM[Ix].Z > MaxPWM)
					PWM[Ix].Z = MaxPWM;
				/* can't ever make the B winding of the zero-ith microstep more than zero */
				PWM[0].Z = 0;
				UseComplexPWMFlag = Yes;
			}
			else
			{
				PWM[Ix].A++;
				if( PWM[Ix].A > MaxPWM)
					PWM[Ix].A = MaxPWM;
			}
		}
		FreeMsArrays();
		CreateMsArrays();
		InitMsArrays();
	}
	else
	{
		switch( NewResponseMsParmIx)
		{
			/* case DisplayMsPowerDownSec:
				if( Response == UpCursor)
					MsPowerDownSec++;
				else
					MsPowerDownSec--;
					if( MsPowerDownSec < 0)
						MsPowerDownSec = 0;
				MsPowerDownCount = MsPowerDownSec/18.2;
				MsPowerDownCountA = MsPowerDownCount;
				MsPowerDownCountZ = MsPowerDownCount;
				break; */
			case DisplayPWMRepsTick:
				if( Response == UpCursor)
					PWMRepsTick++;
				else
				{
					PWMRepsTick--;
					if( PWMRepsTick < 1)
						PWMRepsTick = 1;
				}
				CalcVarsRelatingToStepSizes();
				break;
			case DisplayMsDelayX:
				if( Response == UpCursor)
					MsDelayX++;
				else
				{
					MsDelayX--;
					if( MsDelayX < 1)
						MsDelayX = 1;
				}
				break;
			case DisplayMsPause:
				if( Response == UpCursor)
					MsPause++;
				else
				{
					MsPause--;
					if(MsPause < 0)
						MsPause = 0;
				}
				break;
			case DisplayMs:
				if( Response == UpCursor)
				{
					Ms++;
					if( Ms > MaxMs)
						Ms = MaxMs;
					else
					{
						PWM[Ms].A = PWM[Ms].Z = 0;
						DisplayMsParmsTitlesAndValues();
					}
				}
				else
				{
					Ms--;
					if( Ms < 2)
						Ms = 2;
					else
						DisplayMsParmsTitlesAndValues();
				}
				CalcVarsRelatingToMs();
				FreeMsArrays();
				CreateMsArrays();
				InitMsArrays();
				break;
			case DisplayMsHsToggleIncrMsPerPWM:
				if( Response == UpCursor)
				{
					MsHsToggleIncrMsPerPWM++;
					if( MsHsToggleIncrMsPerPWM > MaxIncrMsPerPWM)
						MsHsToggleIncrMsPerPWM = MaxIncrMsPerPWM;
				}
				else
				{
					MsHsToggleIncrMsPerPWM--;
					if( MsHsToggleIncrMsPerPWM < 0)
						MsHsToggleIncrMsPerPWM = 0;
				}
				break;
			case DisplayMaxIncrMsPerPWM:
				if( Response == UpCursor)
				{
					MaxIncrMsPerPWM++;
					if( MaxIncrMsPerPWM > Ms/2)
						MaxIncrMsPerPWM = Ms/2;
				}
				else
				{
					MaxIncrMsPerPWM--;
					if( MaxIncrMsPerPWM < 0)
						MaxIncrMsPerPWM = 0;
				}
				CalcVarsRelatingToStepSizes();
				break;
			case DisplayMaxPWM:
				if( Response == UpCursor)
					MaxPWM++;
				else
				{
					MaxPWM--;
					for( Ix = 0; Ix < Ms; Ix++)
						if( MaxPWM < PWM[Ix].A)
							MaxPWM = PWM[Ix].A;
				}
				FreeMsArrays();
				CreateMsArrays();
				InitMsArrays();
				break;
		}
	}
	SetMsParmVidMemXY( NewResponseMsParmIx);
	TextAttr = SelectText;
	DisplayMsParmValue( NewResponseMsParmIx);
}

void DisplayMsParmsTitlesAndValues( void)
{
	TextAttr = DefaultText;
	DisplayMsParmsTitles();
	TextAttr = DisplayText;
	for( MsParmIx = 0; MsParmIx < DisplayPWM + Ms*2; MsParmIx++)
		DisplayMsParmValue( MsParmIx);
}

void DisplayMsParmsTitles( void)
{
	int IxC;

	TextAttr = DefaultText;
	VidMemXY = MsParmsXY;
	sprintf( StrBuf, "+- moves motors, left/right arrow selects & up/down changes value, 'q' quits");
	WriteStrBufToScreen_f_ptr();
	VidMemXY.Y++;
	VidMemXY.X = MsParmsXY.X + 0*MsParmWidthX;
	/* sprintf( StrBuf, "PwDnSc"); */
	sprintf( StrBuf, "PWM");
	WriteStrBufToScreen_f_ptr();
	VidMemXY.X = MsParmsXY.X + 1*MsParmWidthX;
	sprintf( StrBuf, "MsDlyX");
	WriteStrBufToScreen_f_ptr();
	VidMemXY.X = MsParmsXY.X + 2*MsParmWidthX;
	sprintf( StrBuf, "MsPaus");
	WriteStrBufToScreen_f_ptr();
	VidMemXY.X = MsParmsXY.X + 3*MsParmWidthX;
	sprintf( StrBuf, "Ms");
	WriteStrBufToScreen_f_ptr();
	VidMemXY.X = MsParmsXY.X + 4*MsParmWidthX;
	sprintf( StrBuf, "HsMsTg");
	WriteStrBufToScreen_f_ptr();
	VidMemXY.X = MsParmsXY.X + 5*MsParmWidthX;
	sprintf( StrBuf, "MsIncr");
	WriteStrBufToScreen_f_ptr();
	VidMemXY.X = MsParmsXY.X + 6*MsParmWidthX;
	sprintf( StrBuf, "MaxPWM");
	WriteStrBufToScreen_f_ptr();

	VidMemXY.X = MsParmsXY.X;
	VidMemXY.Y++;
	for( Ix = 0, IxC = 0; Ix < MaxMs; Ix++, IxC+=MsParmPWMWidthX)
	{
		if( Ix && !(Ix%8))
		{
			WriteStrBufToScreen_f_ptr();
			VidMemXY.Y++;
			VidMemXY.X = MsParmsXY.X;
			IxC = 0;
		}
		if( Ix < Ms)
			sprintf( &StrBuf[IxC], "%2d:      ", Ix);
		else
			sprintf( &StrBuf[IxC], "         ", Ix);
	}
	WriteStrBufToScreen_f_ptr();
}

void DisplayMsParmValue( const int Parm)
{
	SetMsParmVidMemXY( Parm);
	if( Parm < DisplayPWM)
	{
		switch( Parm)
		{
			/* case DisplayMsPowerDownSec:
				sprintf( StrBuf, "%3d", MsPowerDownSec);
				break; */
			case DisplayPWMRepsTick:
				sprintf( StrBuf, "%3d", PWMRepsTick);
				break;
			case DisplayMsDelayX:
				sprintf( StrBuf, "%3d", MsDelayX);
				break;
			case DisplayMsPause:
				sprintf( StrBuf, "%3d", MsPause);
				break;
			case DisplayMs:
				sprintf( StrBuf, "%3d", Ms);
				break;
			case DisplayMsHsToggleIncrMsPerPWM:
				sprintf( StrBuf, "%3d", MsHsToggleIncrMsPerPWM);
				break;
			case DisplayMaxIncrMsPerPWM:
				sprintf( StrBuf, "%3d", MaxIncrMsPerPWM);
				break;
			case DisplayMaxPWM:
				sprintf( StrBuf, "%3d", MaxPWM);
		}
	}
	else
		if( (Parm-DisplayPWM)%2)
			sprintf( StrBuf, "%3d", PWM[(Parm-DisplayPWM)/2].Z);
		else
			sprintf( StrBuf, "%3d", PWM[(Parm-DisplayPWM)/2].A);
	WriteStrBufToScreen_f_ptr();
}

void TrackByRate( void)
{
	/* add microsteps (double) to do for this clock tick */
	TrackHoldMsTick.A += TrackMsTick.A;
	TrackHoldMsTick.Z += TrackMsTick.Z;
	/* acceleration: a way to smoothly increase # of microsteps per tick so as to accelerate
	through a range of speeds */
	TrackAccumMsAccel.A += TrackMsAccel.A;
	TrackAccumMsAccel.Z += TrackMsAccel.Z;
	TrackHoldMsTick.A += TrackAccumMsAccel.A;
	TrackHoldMsTick.Z += TrackAccumMsAccel.Z;
	if( IncrTrackStep != 0)
	{
		TrackHoldMsTick.A += 1;
		TrackHoldMsTick.Z += 1;
		if( IncrTrackStep < 0)
			TrackDir.A = TrackDir.Z = CCW;
		IncrTrackStep = 0;
	}
	/* set direction */
	Dir.A = TrackDir.A;
	Dir.Z = TrackDir.Z;
	/* set (long) steps to move */
	Steps.A = TrackHoldMsTick.A;
	Steps.Z = TrackHoldMsTick.Z;
	/* subtract (long) steps to move, preserving fractional steps to move in HoldMsTick */
	TrackHoldMsTick.A -= Steps.A;
	TrackHoldMsTick.Z -= Steps.Z;
	/* move */
	MoveMs_f_ptr();
	/* add steps not moved */
	TrackHoldMsTick.A += Steps.A;
	TrackHoldMsTick.Z += Steps.Z;
	TrackDir.A = TrackHoldDir.A;
	TrackDir.Z = TrackHoldDir.Z;
}

void TrackEncoder( void)
{
	const int MaxTries = 9;
	int count = 0;
	int step;
	double StepsPerSec;
	int rate;

	TE = (struct AZLong*) malloc( MsInWindings * sizeof( struct AZLong));
	if( TE == NULL)
		BadExit( "Problem with malloc of TE in TrackEncoder()");

	WriteWindow( MsgFrame);

	gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 1);
	printf( "moving motors to start position...");
	/* attempt to move both motors to microstep '0' on winding 'A' */
	do
	{
		Steps.A = (MsIx.A/MaxPWM) % MsInWindings;
		if( Steps.A && Dir.A == CW)
			Steps.A = MsInWindings - Steps.A;
		Steps.Z = (MsIx.Z/MaxPWM) % MsInWindings;
		if( Steps.Z && Dir.Z == CW)
			Steps.Z = MsInWindings - Steps.Z;
		MoveMs_f_ptr();
		count++;
	}while( count < MaxTries && (Steps.A || Steps.Z));

	/* get rate of motion */
	gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 2);
	printf( "Please enter speed in microsteps per second ");
	if( !GetDouble( &StepsPerSec))
		/* if user aborts, then set it up so that no more execution occurs */
		count = MaxTries;
	else
		/* split rate in half bec. encoders read halfway through */
		rate = (int) (.5 + ClockTicksSec / (2.*StepsPerSec));

	if( count < MaxTries)
	{
		/* move MsInWindings microsteps, taking encoder readings as we go */
		for( step = 0; step < MsInWindings; step++)
		{
			Steps.A = Steps.Z = 1;
			for( Ix = 0; Ix < rate; Ix++)
				MoveMs_f_ptr();
			QueryEncoders_f_ptr();
			for( Ix = 0; Ix < rate; Ix++)
				MoveMs_f_ptr();
			if( EncoderState == ReadReady)
				ReadEncoders_f_ptr();
			TE[step].A = EncoderCount.A;
			TE[step].Z = EncoderCount.Z;
			gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 3);
			printf( "microstep %d: encA %ld encZ %ld", step, TE[step].A, TE[step].Z);
		}
		gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 4);
		printf( "writing %s file", TrackEncodersFile);
		Output = fopen( TrackEncodersFile, "w");
		if( Input == NULL)
			BadExit( strcat( "Could not open ", TrackEncodersFile));
		for( step = 0; step < MsInWindings; step++)
			fprintf( Output, "%d %ld %ld\n", step, TE[step].A, TE[step].Z);
		fclose( Output);
		gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 5);
		ContMsgRoutine();
		RemoveWindow( MsgFrame);
	}
	else
	{
		RemoveWindow( MsgFrame);
		PressKeyToContMsg( "could not move motors to start position");
	}
	free( TE);
	PauseUntilNewSidTime();
	HPEventGetEquat();
}

