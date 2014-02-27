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

void SetEncoderAZandEncoderOffset( void)
{
	if( EncoderState > NotInitialized)
	{
		EncoderAZ.A = Current.Alt;
		EncoderAZ.Z = Current.Az;
		EncoderOffset.A = EncoderAZ.A - OneRev * (double) EncoderCount.A / (double) AltEncoderCountsPerRev;
		EncoderOffset.Z = EncoderAZ.Z - OneRev * (double) EncoderCount.Z / (double) AzEncoderCountsPerRev;
	}
}

void DisplayEncodersNotAvailable( void)
{
	VidMemXY = DisplayXY[DisplayEncoderAlt];
	TextAttr = DisplayText;
	sprintf( StrBuf, "%s ", NotAvailableStr);
	WriteStrBufToScreen_f_ptr();
	VidMemXY = DisplayXY[DisplayEncoderAz];
	sprintf( StrBuf, "%s ", NotAvailableStr);
	WriteStrBufToScreen_f_ptr();
	VidMemXY = DisplayXY[DisplayEncoderCountAlt];
	TextAttr = DisplayText;
	sprintf( StrBuf, "%s", NotAvailableStr);
	WriteStrBufToScreen_f_ptr();
	VidMemXY = DisplayXY[DisplayEncoderCountAz];
	sprintf( StrBuf, "%s", NotAvailableStr);
	WriteStrBufToScreen_f_ptr();
}

// azimuth/Right Ascension should read 0 to 360;
// altitude/Declination should read -180 to 180,
// except when meridian flipped, where altDec should read -90 to 270
void CalcEncoderAZ( void)
{
	EncoderAZ.A = ValidRadPi( EncoderOffset.A + OneRev * (double) EncoderCount.A / (double) AltEncoderCountsPerRev);
	EncoderAZ.Z = ValidRad( EncoderOffset.Z + OneRev * (double) EncoderCount.Z / (double) AzEncoderCountsPerRev);
	if( GEMflippedFlag)
		if( EncoderAZ.A < -QtrRev)
      	EncoderAZ.A += OneRev;
}

/* called from KBEventMoveHs() and SequentialEncoderController() which is called by
SequentialTaskController(): translates encoder counts into coordinates then checks thresholds: if
threshold exceeded then resets Current coordinate, writing event to the encoder reset log file */
void SetEncoderThresholdFlag( void)
{
	double AbsAltError, AbsAzError;

	EncoderThreshold = No;

	/* reset current if encoder vs current discrepancy */
	if( EncoderErrorThreshold > 0)
	{
		/* get alt discrepancy */
		AbsAltError = ValidRadPi( Current.Alt - EncoderAZ.A);
		if( AbsAltError < 0)
			AbsAltError = -AbsAltError;
		if( (TrackFlag && AbsAltError > TrackEncoderErrorThreshold)
		|| (!TrackFlag && AbsAltError > EncoderErrorThreshold))
		{
			EncoderThreshold = Yes;
			if( MakeEncoderResetLogFile)
				WriteEncoderResetLogFileRecord( ALTITUDE);
			Current.Alt = EncoderAZ.A;
			AlignMs_f_ptr();
			SetAccumMsToCurrentAltaz();
		}
		/* get az discrepancy */
		AbsAzError = ValidRadPi( Current.Az - EncoderAZ.Z);
		if( AbsAzError < 0)
			AbsAzError = -AbsAzError;
		if( (TrackFlag && AbsAzError > TrackEncoderErrorThreshold)
		|| (!TrackFlag && AbsAzError > EncoderErrorThreshold))
		{
			EncoderThreshold = Yes;
			if( MakeEncoderResetLogFile)
				WriteEncoderResetLogFileRecord( AZIMUTH);
			Current.Az = EncoderAZ.Z;
			AlignMs_f_ptr();
			SetAccumMsToCurrentAltaz();
		}
	}
}

void ProcessBadEncoderThresholdSlew( void)
{
	/* not StrBuf as it's used in DisplayIn() */
	sprintf( StrBuf2, "EncResSlew%2d", EncoderSlewResetCount++);
	DisplayIn( StrBuf2, NameBlanks);
}

void ProcessBadEncoderThresholdTrack( void)
{
	/* if tracking on, it's assumed that user wishes scope to continue moving to Ra/Dec, ie, scope
	bumped accidently; if tracking off, it's assumed that user is moving scope manually and wishes
	Ra/Dec to be reset based on encoders */
	if( TrackFlag)
	{
		/* not StrBuf as it's used in DisplayIn() */
		sprintf( StrBuf2, "EncResOn%2d",
		EncoderTrackOnResetCount++);
		DisplayIn( StrBuf2, NameBlanks);
	}
	else
	{
		sprintf( StrBuf2, "EncResOff%2d",
		EncoderTrackOffResetCount++);
		DisplayIn( StrBuf2, NameBlanks);
		HPEventGetEquat();
	}
}

void WriteEncoderResetLogFileRecord( Flag Axis)
{
	fprintf( EncoderOutput, "Encoder reset: ");
	if( Axis == ALTITUDE)
		fprintf(EncoderOutput, "Alt %f deg", Current.Alt*RadToDeg - EncoderAZ.A*RadToDeg);
	else
		fprintf(EncoderOutput, "Az %f deg", Current.Az*RadToDeg - EncoderAZ.Z*RadToDeg);
	fprintf( EncoderOutput, "; Track? %c; Time %2d:%02d:%02d\n", TrackFlag?'y':'n', t.ti_hour,
	t.ti_min, t.ti_sec);
}

void ProcessMenuInitEncoders( void)
{
	if( EncoderType)
	{
		ResetEncoders_f_ptr();
		if( EncoderState == Read)
		{
			QueryAndReadEncoders();
			if( EncoderState == Read)
			{
				SetEncoderAZandEncoderOffset();
				PressKeyToContMsg( "Encoders initiated");
			}
			else
				PressKeyToContMsg( "Could not read encoders");
		}
		else
			PressKeyToContMsg( "Could not reset encoders");
	}
	else
		PressKeyToContMsg( "Encoders not available");
}

void ProcessMenuResetToEncoders( void)
{
	if( EncoderState == Read)
	{
		Current.Alt = EncoderAZ.A;
		Current.Az = EncoderAZ.Z;
		SetAccumMsToCurrentAltaz();
		PressKeyToContMsg( "Encoders reset");
		DisplayIn( "encoder reset", NameBlanks);
		HPEventGetEquat();
	}
	else
		PressKeyToContMsg( "Encoders not readable");
}

void DisplayEncoderCounts( void)
{
	VidMemXY = DisplayXY[DisplayEncoderCountAlt];
	sprintf( StrBuf, "%6ld", EncoderCount.A);
	WriteStrBufToScreen_f_ptr();

	VidMemXY = DisplayXY[DisplayEncoderCountAz];
	sprintf( StrBuf, "%6ld", EncoderCount.Z);
	WriteStrBufToScreen_f_ptr();
}

void DisplayEncoderAltaz( void)
{
	VidMemXY = DisplayXY[DisplayEncoderAlt];
	VidMemDeg( EncoderAZ.A);

	VidMemXY = DisplayXY[DisplayEncoderAz];
	VidMemDeg( EncoderAZ.Z);
}

