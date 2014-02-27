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

/* LX200 input... */

void InitLX200Input( void)
{
	if( LX200ComPort < 0 || LX200ComPort > 4)
		BadExit( "LX200ComPort must be 0, 1, 2, 3, or 4");
	if( LX200ComPort && LX200ComPort == EncoderComPort &&
	!(strncmp( EncoderString, "NoEncoders", 10) == 0))
		BadExit( "LX200ComPort can't equal EncoderComPort");

	strcpy( LX200_Object_Name, "LX200 Name Unset\000");

	LX200MinQualityFind = 0;
	LX200LeftButton = LX200RightButton = False;

	InitSerial( LX200ComPort, LX200BaudRate, Parity, DataBits, StopBits);

	LX200_Ix = 0;
	LX200_Cmd_Count = 0;
	Beg_LX200_Ix = 0;
	LX200_Motor_Cmd = NoAck;
	/* start with guiding speed: for Guidecam.exe which never sends a speed setting */
	LX200_Speed_Cmd = SetMotionRateGuide;
	LX200_OpenEndedSlewOn = False;
	LX200_Focus_Speed_Cmd = FocusSetFast;
	LX200_Focus_Cmd = FocusQuit;
	for( Ix = 0; Ix < LX200QueueSize; Ix++)
		LX200Queue[Ix] = Blank;
	for( Ix = 0; Ix < LX200_Cmd_Array_Size; Ix++)
		LX200_Cmd_Array[Ix] = 0;
	LX200_Cmd_Array_Ix = 0;

	strcpy( LX200_Cmd_Str[NoAck], "NA");
	strcpy( LX200_Cmd_Str[Ack], "AK");
	strcpy( LX200_Cmd_Str[AlignAltaz], "AA");
	strcpy( LX200_Cmd_Str[AlignACK], "ACK");
	strcpy( LX200_Cmd_Str[AlignLand], "AL");
	strcpy( LX200_Cmd_Str[AlignPolar], "AP");
	strcpy( LX200_Cmd_Str[ReticleCommand], "BR");
	strcpy( LX200_Cmd_Str[GetDistance], "D ");
	strcpy( LX200_Cmd_Str[FocusOut], "F+");
	strcpy( LX200_Cmd_Str[FocusIn], "F-");
	strcpy( LX200_Cmd_Str[FocusQuit], "FQ");
	strcpy( LX200_Cmd_Str[FocusSetFast], "FF");
	strcpy( LX200_Cmd_Str[FocusSetSlow], "FS");
	strcpy( LX200_Cmd_Str[GetRa], "GR");
	strcpy( LX200_Cmd_Str[GetDec], "GD");
	strcpy( LX200_Cmd_Str[GetAlt], "GA");
	strcpy( LX200_Cmd_Str[GetAz], "GZ");
	strcpy( LX200_Cmd_Str[GetSidT], "GS");
	strcpy( LX200_Cmd_Str[GetLocalT24], "GL");
	strcpy( LX200_Cmd_Str[GetLocalT12], "Ga");
	strcpy( LX200_Cmd_Str[GetSiteName], "GN");
	strcpy( LX200_Cmd_Str[GetMinQualityFind], "Gq");
	strcpy( LX200_Cmd_Str[GetDate], "GC");
	strcpy( LX200_Cmd_Str[GetClockStatus], "Gc");
	strcpy( LX200_Cmd_Str[GetLat], "Gt");
	strcpy( LX200_Cmd_Str[GetLongitude], "Gg");
	strcpy( LX200_Cmd_Str[GetTz], "GG");
	strcpy( LX200_Cmd_Str[GetField], "GF");
	strcpy( LX200_Cmd_Str[TimeQuartz], "TQ");
	strcpy( LX200_Cmd_Str[LI_command], "LI");
	strcpy( LX200_Cmd_Str[SetNGCLibrary], "Lo");
	strcpy( LX200_Cmd_Str[SetStarLibrary], "Ls");
	strcpy( LX200_Cmd_Str[MoveDirRateNorth], "Mn");
	strcpy( LX200_Cmd_Str[MoveDirRateSouth], "Ms");
	strcpy( LX200_Cmd_Str[MoveDirRateEast], "Me");
	strcpy( LX200_Cmd_Str[MoveDirRateWest], "Mw");
	strcpy( LX200_Cmd_Str[StartSlew], "MS");
	strcpy( LX200_Cmd_Str[StopSlew], "Q ");
	strcpy( LX200_Cmd_Str[StopMotionNorth], "Qn");
	strcpy( LX200_Cmd_Str[StopMotionSouth], "Qs");
	strcpy( LX200_Cmd_Str[StopMotionEast], "Qe");
	strcpy( LX200_Cmd_Str[StopMotionWest], "Qw");
	strcpy( LX200_Cmd_Str[SetMotionRateGuide], "RG");
	strcpy( LX200_Cmd_Str[SetMotionRateCenter], "RC");
	strcpy( LX200_Cmd_Str[SetMotionRateFind], "RM");
	strcpy( LX200_Cmd_Str[SetMotionRateSlew], "RS");
	strcpy( LX200_Cmd_Str[SetRa], "Sr");
	strcpy( LX200_Cmd_Str[SetDec], "Sd");
	strcpy( LX200_Cmd_Str[SetField], "SF");
	strcpy( LX200_Cmd_Str[SetCurrentHigherLimit], "Sh");
	strcpy( LX200_Cmd_Str[SetSidT], "SS");
	strcpy( LX200_Cmd_Str[SetLocalT], "SL");
	strcpy( LX200_Cmd_Str[SetDate], "SC");
	strcpy( LX200_Cmd_Str[SetGMTOffset], "SG");
	strcpy( LX200_Cmd_Str[SetSiteNumber_S], "SM");
	strcpy( LX200_Cmd_Str[SetLat], "St");
	strcpy( LX200_Cmd_Str[SetLongitude], "Sg");
	strcpy( LX200_Cmd_Str[SetBrightMagLimitFindOperation], "Sb");
	strcpy( LX200_Cmd_Str[SetFaintMagLimitFindOperation], "Sf");
	strcpy( LX200_Cmd_Str[LargeSizeLimitFindOperation], "Sl");
	strcpy( LX200_Cmd_Str[SmallSizeLimitFindOperation], "Ss");
	strcpy( LX200_Cmd_Str[NextMinQualityFind], "Sq");
	strcpy( LX200_Cmd_Str[SetTypeStringForFind], "Sy");
	strcpy( LX200_Cmd_Str[Sw_command], "Sw");
	strcpy( LX200_Cmd_Str[Sync], "CM");
	strcpy( LX200_Cmd_Str[SetSiteNumber_W], "W1");
	strcpy( LX200_Cmd_Str[ToggleLongFormat], "U ");
	strcpy( LX200_Cmd_Str[GetFirmwareIDString], "GVF");
	strcpy( LX200_Cmd_Str[GetFirmwareDate], "GVD");
	strcpy( LX200_Cmd_Str[GetProductName], "GVP");
	strcpy( LX200_Cmd_Str[ReadGuideArcsecSec], "gr");
	strcpy( LX200_Cmd_Str[SendGuideArcsecSec], "gs");
	strcpy( LX200_Cmd_Str[SetHandpadMode], "hm");
	strcpy( LX200_Cmd_Str[HandpadLeftKey], "hl");
	strcpy( LX200_Cmd_Str[HandpadRightKey], "hr");
	strcpy( LX200_Cmd_Str[SetInit1], "i1");
	strcpy( LX200_Cmd_Str[SetInit2], "i2");
	strcpy( LX200_Cmd_Str[SetInit3], "i3");
	strcpy( LX200_Cmd_Str[ReadMsArcsecSec], "mr");
	strcpy( LX200_Cmd_Str[SendMsArcsecSec], "ms");
	strcpy( LX200_Cmd_Str[SendFieldR], "fs");
	strcpy( LX200_Cmd_Str[SetObjectName], "SO");
	strcpy( LX200_Cmd_Str[PecOnOff], "$Q");
	strcpy( LX200_Cmd_Str[MoveHomePosition], "hP");
	strcpy( LX200_Cmd_Str[FROn], "r+");
	strcpy( LX200_Cmd_Str[FROff], "r-");
	strcpy( LX200_Cmd_Str[GetFfspeed], "gf");
	strcpy( LX200_Cmd_Str[GetFFspeed], "gF");
	strcpy( LX200_Cmd_Str[SetFfspeed], "sf");
	strcpy( LX200_Cmd_Str[SetFFspeed], "sF");
	strcpy( LX200_Cmd_Str[GetFocusPos], "gp");
	strcpy( LX200_Cmd_Str[LX200_Clear_Display], "xx");
	strcpy( LX200_Cmd_Str[LX200_StringCommand], "sc");
	strcpy( LX200_Cmd_Str[LX200_Unfinished_Cmd], "Un");
	strcpy( LX200_Cmd_Str[LX200_Unknown_Cmd0], "U0");
	strcpy( LX200_Cmd_Str[LX200_Unknown_Cmd1], "U1");
	strcpy( LX200_Cmd_Str[LX200_Unknown_Cmd2], "U2");
	strcpy( LX200_Cmd_Str[LX200_Ignored_Cmd], "Ig");
}

void InsertLX200CmdIntoArray( const LX200_COMMAND_TYPE LX200_Cmd_To_Add)
{
	LX200_Cmd_Array_Ix++;
	LX200_Cmd_Array_Ix %= LX200_Cmd_Array_Size;
	LX200_Cmd_Array[LX200_Cmd_Array_Ix] = LX200_Cmd_To_Add;
	LX200_Cmd_Count++;
}

Flag ReadLX200Input( void)
{
	if( LX200CmdProcessingUnderway)
		return False;
	else
		return ReadLX200InputSubr();
}

Flag ReadLX200InputSubr( void)
{
	Byte B;
	static Flag DataReady;
	static Flag ReadData;
	static double ReadDataSidT;
	double TimeDiff;

	if( ReadSerial( LX200ComPort, &B))
	{
		ReadData = Yes;
		/* get rest of buffered serial data */
		do
		{
			LX200_Ix++;
			LX200_Ix %= LX200QueueSize;
			LX200Queue[LX200_Ix] = B;
			if( LX200Queue[LX200_Ix] == LX200_ACK)
			{
				WriteSerial( LX200ComPort, 'A');
				InsertLX200CmdIntoArray( Ack);
				/* start fresh, ignoring any previous commands */
				Beg_LX200_Ix = LX200_Ix+1;
				Beg_LX200_Ix %= LX200QueueSize;
				ReadData = No;
			}
		} while( ReadSerial( LX200ComPort, &B));
	}
	else
		ReadData = No;

	/* since multiple commands can be streamed at one time, wait until input port is quiet before
	processing commands and returning any requested data */
	if( ReadData == Yes)
	{
		ReadDataSidT = Current.SidT;
		DataReady = True;
	}
	/* else LX200 port quiet */
	else
		if( DataReady == True)
		{
			TimeDiff = Current.SidT - ReadDataSidT;
			if( TimeDiff <= -OneRev)
				TimeDiff += OneRev;
			/* after 1 minute, if nothing further received, and commands do not end with '#', process
			it as unfinished; ignore the time test if the DOS clock is being reset via the CMOS real
			time clock - this causes the TimeDiff as seen by this function to be very large when
			finishing long slews */
			if( TimeDiff > MinToRad)
			{
				InsertLX200CmdIntoArray( LX200_Unfinished_Cmd);
				DataReady = No;
				/* get ready for next command... */
				Beg_LX200_Ix = LX200_Ix+1;
				Beg_LX200_Ix %= LX200QueueSize;
			}
			else
				/* wait one clock tick, or 55 ms to process, all commands must end with a '#' */
				if( (TimeDiff >= ClockTickToRad || MoveHsUnderway) && LX200Queue[LX200_Ix] == '#')
				{
					ProcessLX200Cmds();
					DataReady = No;
					/* get ready for next command... */
					Beg_LX200_Ix = LX200_Ix+1;
					Beg_LX200_Ix %= LX200QueueSize;
				}
		}
	return DataReady;
}

void LX200_Write_Ra( void)
{
	int Hr, Min, TenthsMin;

	if( SlewState != SlewDone)
	{
		SlewCurrent = Current;
		SetCurrentToSlew = True;
		HPEventGetEquat();
	}

	GetHMSH( RadToHundSec*Current.Ra + 0.5, &Current.RaHMSH);

	if( LX200_LongFormat)
		sprintf( StrBuf, "%02d:%02d:%02d#", Current.RaHMSH.Hr, Current.RaHMSH.Min, Current.RaHMSH.Sec);
	else
	{
		Hr = Current.RaHMSH.Hr;
		Min = Current.RaHMSH.Min;
		TenthsMin = (Current.RaHMSH.Sec + 3)/6;
		if( TenthsMin > 9)
		{
			TenthsMin = 0;
			Min++;
			if( Min > 59)
			{
				Min = 0;
				Hr++;
			}
		}
		sprintf( StrBuf, "%02d:%02d.%1d#", Hr, Min, TenthsMin);
	}
	if( MoveHsUnderway)
		WriteSerialStringPauseUntilXmtFinished( LX200ComPort, StrBuf, strlen( StrBuf));
	else
		WriteSerialString( LX200ComPort, StrBuf, strlen( StrBuf));

	if( SetCurrentToSlew)
		Current = SlewCurrent;
}

void LX200_Write_Dec( void)
{
	int Deg, Min;
	char Sign;

	if( SlewState != SlewDone)
	{
		SlewCurrent = Current;
		SetCurrentToSlew = True;
		HPEventGetEquat();
	}

	GetDMS( RadToArcsec*Current.Dec + 0.5, &Current.DecDMS);

	if( Current.DecDMS.Sign == Plus)
		Sign = '+';
	else
		Sign = '-';
	if( LX200_LongFormat)
		sprintf( StrBuf, "%c%02d%c%02d:%02d#", Sign, Current.DecDMS.Deg, DegSym, Current.DecDMS.Min,
		Current.DecDMS.Sec);
	else
	{
		Deg = Current.DecDMS.Deg;
		Min = Current.DecDMS.Min;
		if( Current.DecDMS.Sec > 29)
		{
			Min++;
			if( Min > 59)
			{
				Min = 0;
				Deg++;
			}
		}
		sprintf( StrBuf, "%c%02d%c%02d#", Sign, Deg, DegSym, Min);
	}
	if( MoveHsUnderway)
		WriteSerialStringPauseUntilXmtFinished( LX200ComPort, StrBuf, strlen( StrBuf));
	else
		WriteSerialString( LX200ComPort, StrBuf, strlen( StrBuf));

	if( SetCurrentToSlew)
		Current = SlewCurrent;
}

void LX200_Write_Alt( void)
{
	static struct DMS V;
	char Sign;
	static double HoldAlt;

	if( HoldAlt != Current.Alt)
	{
		GetDMS( RadToArcsec*Current.Alt + 0.5, &V);
		HoldAlt = Current.Alt;
	}
	if( V.Sign == Plus)
		Sign = '+';
	else
		Sign = '-';
	if( LX200_LongFormat)
		sprintf( StrBuf, "%c%02d%c%02d:%02d#", Sign, V.Deg, DegSym, V.Min, V.Sec);
	else
	{
		if( V.Sec > 29)
		{
			V.Min++;
			if( V.Min > 59)
				V.Deg++;
		}
		sprintf( StrBuf, "%c%02d%c%02d#", Sign, V.Deg, DegSym, V.Min);
	}
	if( MoveHsUnderway)
		WriteSerialStringPauseUntilXmtFinished( LX200ComPort, StrBuf, strlen( StrBuf));
	else
		WriteSerialString( LX200ComPort, StrBuf, strlen( StrBuf));
}

void LX200_Write_Az( void)
{
	static struct DMS V;
	static double HoldAz;

	if( HoldAz != Current.Az)
	{
		GetDMS( RadToArcsec*Current.Az + 0.5, &V);
		HoldAz = Current.Az;
	}
	if( LX200_LongFormat)
		sprintf( StrBuf, "%03d%c%02d:%02d#", V.Deg, DegSym, V.Min, V.Sec);
	else
	{
		if( V.Sec > 29)
		{
			V.Min++;
			if( V.Min > 59)
				V.Deg++;
		}
		sprintf( StrBuf, "%03d%c%02d#", V.Deg, DegSym, V.Min);
	}
	if( MoveHsUnderway)
		WriteSerialStringPauseUntilXmtFinished( LX200ComPort, StrBuf, strlen( StrBuf));
	else
		WriteSerialString( LX200ComPort, StrBuf, strlen( StrBuf));
}

void LX200_Write_SidT( void)
{
	int Hr, Min, TenthsMin;

	GetHMSH( RadToHundSec*Current.SidT + 0.5, &Current.SidTimeHMSH);
	if( LX200_LongFormat)
		sprintf( StrBuf, "%02d:%02d:%02d#", Current.SidTimeHMSH.Hr, Current.SidTimeHMSH.Min,
		Current.SidTimeHMSH.Sec);
	else
	{
		Hr = Current.SidTimeHMSH.Hr;
		Min = Current.SidTimeHMSH.Min;
		TenthsMin = (Current.SidTimeHMSH.Sec + 3)/6;
		if( TenthsMin > 9)
		{
			TenthsMin = 0;
			Min++;
			if( Min > 59)
			{
				Min = 0;
				Hr++;
			}
		}
		sprintf( StrBuf, "%02d:%02d.%1d#", Hr, Min, TenthsMin);
	}
	if( MoveHsUnderway)
		WriteSerialStringPauseUntilXmtFinished( LX200ComPort, StrBuf, strlen( StrBuf));
	else
		WriteSerialString( LX200ComPort, StrBuf, strlen( StrBuf));
}

void LX200_Write_LocalT( void)
{
	gettime( &t);
	sprintf( StrBuf, "%02d:%02d:%02d#", t.ti_hour, t.ti_min, t.ti_sec);
	if( MoveHsUnderway)
		WriteSerialStringPauseUntilXmtFinished( LX200ComPort, StrBuf, strlen( StrBuf));
	else
		WriteSerialString( LX200ComPort, StrBuf, strlen( StrBuf));
}

/* gets site name (XYZ): GM GN GO GP correspond to 1 through 4; returns XYZ# */
void LX200_GetSiteName( void)
{
	sprintf( StrBuf, "SCP#");
	if( MoveHsUnderway)
		WriteSerialStringPauseUntilXmtFinished( LX200ComPort, StrBuf, strlen( StrBuf));
	else
		WriteSerialString( LX200ComPort, StrBuf, strlen( StrBuf));
}

/* gets the current minimum quality for the FIND operation, returns  SU#,  EX#,  VG#,  GD#,  FR#,
PR#  or VP# - superior, excellent, very good, good, fair, poor, very poor? */
void LX200_Write_MinQualityFind( void)
{
	switch( LX200MinQualityFind)
	{
		case 0:
			sprintf( StrBuf, "SU#");
			break;
		case 1:
			sprintf( StrBuf, "EX#");
			break;
		case 2:
			sprintf( StrBuf, "VG#");
			break;
		case 3:
			sprintf( StrBuf, "GD#");
			break;
		case 4:
			sprintf( StrBuf, "FR#");
			break;
		case 5:
			sprintf( StrBuf, "PR#");
			break;
		case 6:
			sprintf( StrBuf, "VP#");
			break;
	}
	if( MoveHsUnderway)
		WriteSerialStringPauseUntilXmtFinished( LX200ComPort, StrBuf, strlen( StrBuf));
	else
		WriteSerialString( LX200ComPort, StrBuf, strlen( StrBuf));
}

void LX200_Write_Date( void)
{
	getdate( &d);
	sprintf( StrBuf, "%02d/%02d/%02d#", d.da_mon, d.da_day, d.da_year%100);
	if( MoveHsUnderway)
		WriteSerialStringPauseUntilXmtFinished( LX200ComPort, StrBuf, strlen( StrBuf));
	else
		WriteSerialString( LX200ComPort, StrBuf, strlen( StrBuf));
}

void LX200_Write_Clock_Status( void)
{
	sprintf( StrBuf, "(24)#");
	if( MoveHsUnderway)
		WriteSerialStringPauseUntilXmtFinished( LX200ComPort, StrBuf, strlen( StrBuf));
	else
		WriteSerialString( LX200ComPort, StrBuf, strlen( StrBuf));
}

void LX200_Write_Lat( void)
{
	struct DMS V;
	char Sign;

	GetDMS( LatitudeDeg*3600 + 0.5, &V);
	if( V.Sign == Plus)
		Sign = '+';
	else
		Sign = '-';
	if( LX200_LongFormat)
		sprintf( StrBuf, "%c%02d%c%02d:%02d#", Sign, V.Deg, DegSym, V.Min, V.Sec);
	else
	{
		if( V.Sec > 29)
		{
			V.Min++;
			if( V.Min > 59)
				V.Deg++;
		}
		sprintf( StrBuf, "%c%02d%c%02d#", Sign, V.Deg, DegSym, V.Min);
	}
	if( MoveHsUnderway)
		WriteSerialStringPauseUntilXmtFinished( LX200ComPort, StrBuf, strlen( StrBuf));
	else
		WriteSerialString( LX200ComPort, StrBuf, strlen( StrBuf));
}

void LX200_Write_Long( void)
{
	struct DMS V;

	GetDMS( RadToArcsec*LongitudeRad + 0.5, &V);
	if( LX200_LongFormat)
		sprintf( StrBuf, "%03d%c%02d:%02d#", V.Deg, DegSym, V.Min, V.Sec);
	else
	{
		if( V.Sec > 29)
		{
			V.Min++;
			if( V.Min > 59)
				V.Deg++;
		}
		sprintf( StrBuf, "%03d%c%02d#", V.Deg, DegSym, V.Min);
	}
	if( MoveHsUnderway)
		WriteSerialStringPauseUntilXmtFinished( LX200ComPort, StrBuf, strlen( StrBuf));
	else
		WriteSerialString( LX200ComPort, StrBuf, strlen( StrBuf));
}

/* gets the timezone and returns sHH# */
void LX200_Write_Tz( void)
{
	int TZOffset;
	char s, h1, h2;

	TZOffset = Tz - DST;
	if( TZOffset < 0)
	{
		TZOffset = -TZOffset;
		s = '-';
	}
	else
		s = ' ';
	if( TZOffset > 9)
		h1 = '0' + TZOffset/10;
	else
	{
   	// shift '-' to the right one char
		h1 = s;
		s = ' ';
	}
	h2 = '0' + (TZOffset % 10);

	sprintf( StrBuf, "%c%c%c#", s, h1, h2);
	if( MoveHsUnderway)
		WriteSerialStringPauseUntilXmtFinished( LX200ComPort, StrBuf, strlen( StrBuf));
	else
		WriteSerialString( LX200ComPort, StrBuf, strlen( StrBuf));
}

/* gets the field radius of the FIELD operation and returns NNN# */
void LX200_Write_Field( void)
{
	sprintf( StrBuf, "100#");
	if( MoveHsUnderway)
		WriteSerialStringPauseUntilXmtFinished( LX200ComPort, StrBuf, strlen( StrBuf));
	else
		WriteSerialString( LX200ComPort, StrBuf, strlen( StrBuf));
}

/* if slewing, return <DEL> or 0x7F followed by #, else return only a # */
void LX200_GetDistance( void)
{
	if( SlewState == SlewDone)
		sprintf( StrBuf, "#");
	else
		sprintf( StrBuf, "%c#", DEL);
	if( MoveHsUnderway)
		WriteSerialStringPauseUntilXmtFinished( LX200ComPort, StrBuf, strlen( StrBuf));
	else
		WriteSerialString( LX200ComPort, StrBuf, strlen( StrBuf));
}

void LX200_GetFirmwareIDString( void)
{
	sprintf( StrBuf, "BartelsStepper#");
	if( MoveHsUnderway)
		WriteSerialStringPauseUntilXmtFinished( LX200ComPort, StrBuf, strlen( StrBuf));
	else
		WriteSerialString( LX200ComPort, StrBuf, strlen( StrBuf));
}

void LX200_GetProductName( void)
{
	sprintf( StrBuf, "Scope.exe#");
	if( MoveHsUnderway)
		WriteSerialStringPauseUntilXmtFinished( LX200ComPort, StrBuf, strlen( StrBuf));
	else
		WriteSerialString( LX200ComPort, StrBuf, strlen( StrBuf));
}

/* GVD */
void LX200_GetFirmwareDate( void)
{
	char firmwaredate[14]="";
	char *blank = " ";
	char *endchar = "#";

	strncat(firmwaredate,MainFrame.Title+32,3);
	strcat(firmwaredate,blank);
	strncat(firmwaredate,MainFrame.Title+strlen(MainFrame.Title)-24,2);
	strcat(firmwaredate,blank);
	strncat(firmwaredate,MainFrame.Title+strlen(MainFrame.Title)-20,4);
	strcat(firmwaredate,endchar);

	sprintf( StrBuf, firmwaredate);
	if( MoveHsUnderway)
		WriteSerialStringPauseUntilXmtFinished( LX200ComPort, StrBuf, strlen( StrBuf));
	else
		WriteSerialString( LX200ComPort, StrBuf, strlen( StrBuf));
}

/* XGP Get focuser position */
void LX200_Custom_GetFocusPosition( void )
{
	sprintf( StrBuf, "%+04d#", FocusPosition);
	if( MoveHsUnderway)
		WriteSerialStringPauseUntilXmtFinished( LX200ComPort, StrBuf, strlen( StrBuf));
	else
		WriteSerialString( LX200ComPort, StrBuf, strlen( StrBuf));
}

/* XG(F|f) Get focuser (fast|slow) speed */
void LX200_Custom_GetFocusSpeed( int type)
{
	/* int type 0: slow speed, anything else fast speed */
	if( type)
		sprintf( StrBuf, "%03d#", FocusFastStepsSec);
	else
		sprintf( StrBuf, "%03d#", FocusSlowStepsSec);
	if( MoveHsUnderway)
		WriteSerialStringPauseUntilXmtFinished( LX200ComPort, StrBuf, strlen( StrBuf));
	else
		WriteSerialString( LX200ComPort, StrBuf, strlen( StrBuf));
}

/* XS(F|f) Set focuser (fast|slow) speed */
void LX200_Custom_SetFocusSpeed(int type, int FocusStepsSec)
{
	if( FocusStepsSec < 1)
	{
		/* Fast focus speed must be > 0, setting it to 1 */
		FocusStepsSec = 1;
	}
	if( FocusStepsSec > 500)
	{
		 /* Fast focus speed cannot exceed 500, setting it to 500 */
		FocusStepsSec = 500;
	}

	if (type)
	/* 0 slow, <>0 fast */
	{
		/* limit lowest slow-rate to slow-speed */
		if( FocusStepsSec < FocusSlowStepsSec)
			FocusStepsSec = FocusSlowStepsSec;
		/* record fast */
		FocusFastStepsSec = FocusStepsSec;
		WriteFocusFastSpeed();
	}
	else
	{
		/* limit highest slow-rate to fast */
		if( FocusStepsSec > FocusFastStepsSec)
			FocusStepsSec = FocusFastStepsSec;
		/* record slow */
		FocusSlowStepsSec = FocusStepsSec;
		WriteFocusSlowSpeed();
	}

}

Flag CharIsNumeric( int IxQ)
{
	IxQ %= LX200QueueSize;
	if( LX200Queue[IxQ] >= '0' && LX200Queue[IxQ] <= '9')
		return True;
	return False;
}

/* IxQ is the index in LX200 queue to start reading from */
int LX200_Read_Int_From_Queue( int IxQ)
{
	/* max digits expected for integer + terminating char */
	char N[5];

	for( Ix = 0; Ix < 5; Ix++, IxQ++)
	{
		IxQ %= LX200QueueSize;
		N[Ix] = LX200Queue[IxQ];
	}
	/* the first unrecognized character ends the conversion */
	return atoi( N);
}

int LX200_Incr_Ix_Until_Terminate_Char( int* Ix)
{
	int Cnt = 0;

	do
	{
		(*Ix)++;
		(*Ix) %= LX200QueueSize;
		Cnt++;
	}while(
	LX200Queue[*Ix] != ':' &&
	LX200Queue[*Ix] != DegSym &&
	LX200Queue[*Ix] != '#' &&
	LX200Queue[*Ix] != '*' &&
	LX200Queue[*Ix] != '.' &&
	LX200Queue[*Ix] != ' ' &&
	LX200Queue[*Ix] != '/');
	return Cnt;
}

/* set number of chars to increment, starting at Ix+3, stopping at '#';
either call this function or execute IncrIx++ */
void LX200_Set_IncrIx( const int Ix, int *IncrIx)
{
	int IxA;

	IxA = Ix+3;
	IxA %= LX200QueueSize;
	*IncrIx = 3;
	while( LX200Queue[IxA] != '#')
	{
		(*IncrIx)++;
		IxA++;
		IxA %= LX200QueueSize;
	}
}

/* 012345678901   01234567890
	  _HH:MM:SS# or  _HH:MM.T# ; values may be 1 or 2 chars long with decimal seconds being up to 3chars long;
	  can read _HH:MM:SS.d# and _HH:MM:SS.dd# and _HH:MM:SS.ddd#, ' ' may substitute for ':' */
void LX200_Read_Ra( const int Ix)
{
	int IxA;
	int Decimals = 0;

	WriteSerial( LX200ComPort, LX200_OK);

	In.RaHMSH.Sign = Plus;
	In.RaHMSH.Hr = In.RaHMSH.Min = In.RaHMSH.Sec = In.RaHMSH.HundSec = 0;

	if( CharIsNumeric( Ix+2))
		IxA = Ix+2;
	else
		IxA = Ix+3;
	In.RaHMSH.Hr = LX200_Read_Int_From_Queue( IxA);
	LX200_Incr_Ix_Until_Terminate_Char( &IxA);
	if( LX200Queue[IxA] == ':')
	{
		IxA++;
		In.RaHMSH.Min = LX200_Read_Int_From_Queue( IxA);
		LX200_Incr_Ix_Until_Terminate_Char( &IxA);
		if( LX200Queue[IxA] == ':' || LX200Queue[IxA] == ' ')
		{
			IxA++;
			In.RaHMSH.Sec = LX200_Read_Int_From_Queue( IxA);
			LX200_Incr_Ix_Until_Terminate_Char( &IxA);
			if( LX200Queue[IxA] == '.')
			{
				IxA++;
				In.RaHMSH.HundSec = LX200_Read_Int_From_Queue( IxA);
			}
			Decimals = LX200_Incr_Ix_Until_Terminate_Char( &IxA);
			if( Decimals == 1)
				In.RaHMSH.HundSec *= 10;
			if( Decimals == 3)
				In.RaHMSH.HundSec /= 10;
		}
		else
			if( LX200Queue[IxA] == '.')
			{
				IxA++;
				In.RaHMSH.Sec = 6 * LX200_Read_Int_From_Queue( IxA);
			}
	}
	CalcRadFromHMSH( &In.Ra, In.RaHMSH);
	DisplayIn( "from LX200", LX200_Object_Name);
	Response = MenuResetEquat;
	GetNewMenuCatSubFromResponse();
	UpdateMenuCatSub();
}

/* 012345678901    01234567890
	  _sDD*MM:SS# or  _sDD*MM# ; check to see if leading blank is dropped; ':' and DegSym (223) may
	  substitude for '*', ' ' may substitute for ':', and values may be 1 or 2 chars long */
void LX200_Read_Dec( const int Ix)
{
	int IxA;

	WriteSerial( LX200ComPort, LX200_OK);

	In.DecDMS.Deg = In.DecDMS.Min = In.DecDMS.Sec = 0;
	In.DecDMS.Sign = Plus;

	IxA = Ix+2;
	IxA %= LX200QueueSize;
	if( LX200Queue[IxA] != '-' && LX200Queue[IxA] != '+')
	{
		IxA = Ix+3;
		IxA %= LX200QueueSize;
	}
	if( LX200Queue[IxA] == '-')
		In.DecDMS.Sign = Minus;
	IxA++;
	In.DecDMS.Deg = LX200_Read_Int_From_Queue( IxA);
	LX200_Incr_Ix_Until_Terminate_Char( &IxA);
	if( LX200Queue[IxA] == DegSym || LX200Queue[IxA] == ':' || LX200Queue[IxA] == '*')
	{
		IxA++;
		In.DecDMS.Min = LX200_Read_Int_From_Queue( IxA);
		LX200_Incr_Ix_Until_Terminate_Char( &IxA);
		if( LX200Queue[IxA] == ':' || LX200Queue[IxA] == ' ')
		{
			IxA++;
			In.DecDMS.Sec = LX200_Read_Int_From_Queue( IxA);
		}
	}
	else
		if( LX200Queue[IxA] == '#')
		{
			IxA++;
			In.DecDMS.Min = LX200_Read_Int_From_Queue( IxA);
		}

	CalcRadFromDMS( &In.Dec, In.DecDMS);
	DisplayIn( "from LX200", LX200_Object_Name);
	Response = MenuResetEquat;
	GetNewMenuCatSubFromResponse();
	UpdateMenuCatSub();
}

/* _HH:MM:SS# (always use 24 hr format) */
void LX200_Read_SidT( const int Ix)
{
	int IxA;
	double SidTDiff, TDiff, CurrentSolarTimeRad, NewCurrentSolarTimeRad;

	WriteSerial( LX200ComPort, LX200_OK);

	In.SidTimeHMSH.Sign = Plus;
	In.SidTimeHMSH.Hr = In.SidTimeHMSH.Min = In.SidTimeHMSH.Sec =
	In.SidTimeHMSH.HundSec = 0;

	if( CharIsNumeric( Ix+2))
		IxA = Ix+2;
	else
		IxA = Ix+3;
	In.SidTimeHMSH.Hr = LX200_Read_Int_From_Queue( IxA);
	LX200_Incr_Ix_Until_Terminate_Char( &IxA);
	if( LX200Queue[IxA] == ':')
	{
		IxA++;
		In.SidTimeHMSH.Min = LX200_Read_Int_From_Queue( IxA);
		LX200_Incr_Ix_Until_Terminate_Char( &IxA);
		if( LX200Queue[IxA] == ':')
		{
			IxA++;
			In.SidTimeHMSH.Sec = LX200_Read_Int_From_Queue( IxA);
		}
	}
	CalcRadFromHMSH( &In.SidT, In.SidTimeHMSH);

	/* get sidereal time difference */
	SidTDiff = Current.SidT - In.SidT;
	/* convert time difference to solar time */
	TDiff = SidTDiff / SidRate;
	CurrentSolarTimeRad = (double) t.ti_hour * HrToRad + (double) t.ti_min * MinToRad +
	(double) t.ti_sec * SecToRad + (double) t.ti_hund * HundSecToRad;
	/* calculate new solar time */
	NewCurrentSolarTimeRad = ValidRad( CurrentSolarTimeRad - TDiff);
	/* change system time */
	t.ti_hour = (int) (NewCurrentSolarTimeRad * RadToHr);
	t.ti_min = (int) ((NewCurrentSolarTimeRad - (double) t.ti_hour * HrToRad) * RadToMin);
	t.ti_sec = (int) ((NewCurrentSolarTimeRad - (double) t.ti_hour * HrToRad - (double) t.ti_min * MinToRad) * RadToSec);
	t.ti_hund = 0;
	settime( &t);
	NewSidT();
}

/* _HH:MM:SS# (always use 24 hr format) */
void LX200_Read_LocalT( const int Ix)
{
	int IxA;
	long Yr;
	int M;
	int D;
	int hr, min, sec;

	WriteSerial( LX200ComPort, LX200_OK);

	if( CharIsNumeric( Ix+2))
		IxA = Ix+2;
	else
		IxA = Ix+3;
	hr = min = sec = 0;
	hr = LX200_Read_Int_From_Queue( IxA);
	LX200_Incr_Ix_Until_Terminate_Char( &IxA);
	if( LX200Queue[IxA] == ':')
	{
		IxA++;
		min = LX200_Read_Int_From_Queue( IxA);
		LX200_Incr_Ix_Until_Terminate_Char( &IxA);
		if( LX200Queue[IxA] == ':')
		{
			IxA++;
			sec = LX200_Read_Int_From_Queue( IxA);
		}
	}
	getdate( &d);
	/* set century for Y2K problem with CMOS clock */
	Century = d.da_year / 100;
	Yr = (long) d.da_year;
	M = (int) d.da_mon;
	D = (int) d.da_day;

	SetRealTimeClock( hr, min, sec);
	SetDOSToCMOS_RTC_f_ptr();
	SetStartBiosClockAndSidTime( Yr, M, D, hr, min, sec, Tz, DST, LongitudeDeg);
}

/* _MM/DD/YY# */
void LX200_Read_CurrentDate( const int Ix)
{
	int IxA;
	int h;
	int m;
	double s;
	long year;
	int mon, day;

	WriteSerial( LX200ComPort, LX200_OK);

	year = 0;
	mon = day = 0;
	/* _MM/DD/YY# */

	if( CharIsNumeric( Ix+2))
		IxA = Ix+2;
	else
		IxA = Ix+3;
	mon = LX200_Read_Int_From_Queue( IxA);
	LX200_Incr_Ix_Until_Terminate_Char( &IxA);
	if( LX200Queue[IxA] == '/')
	{
		IxA++;
		day = LX200_Read_Int_From_Queue( IxA);
		LX200_Incr_Ix_Until_Terminate_Char( &IxA);
		if( LX200Queue[IxA] == '/')
		{
			IxA++;
			year = LX200_Read_Int_From_Queue( IxA);
		}
	}
	if( year > 80)
		Century = 19;
	else
		Century = 20;

	gettime( &t);
	h = (int) t.ti_hour;
	m = (int) t.ti_min;
	s = ((double) t.ti_sec) + (t.ti_hund / 100.);

	SetDateofRealTimeClock( Century, (int) year, mon, day);
	SetDOSToCMOS_RTC_f_ptr();
	SetStartBiosClockAndSidTime( year, mon, day, h, m, s, Tz, DST, LongitudeDeg);
}

void ProcessLX200Cmds( void)
{
	int Ix, IxA, Ix1, Ix2, Ix3, IncrIx;
	int IxS, IxQ, HoldLX200ComPort;
	int HoldButtons = Buttons;

	Ix = Beg_LX200_Ix;
	IxA = LX200_Ix+1;
	IxA %= LX200QueueSize;

	while( Ix != IxA)
	{
		IncrIx = 1;
		/* all commands start with a ':' */
		if( LX200Queue[Ix] == ':')
		{
			LX200CmdProcessingUnderway = True;
			/* set Ix to first letter of LX200 command following the ':' */
			Ix++;
			Ix %= LX200QueueSize;
			Ix1 = Ix+1;
			Ix1 %= LX200QueueSize;
			Ix2 = Ix1+1;
			Ix2 %= LX200QueueSize;
			Ix3 = Ix2+1;
			Ix3 %= LX200QueueSize;

			switch( LX200Queue[Ix])
			{
				/* customized extensions to the LX200 protocol */
				case '$':
					IncrIx++;
					switch ( LX200Queue[Ix1])
					{
						/* Toggles Smart Drive Pec*/
						case 'Q':
							Response = MenuPECOnOff;
							GetNewMenuCatSubFromResponse();
							UpdateMenuCatSub();
							IncrIx++;
							InsertLX200CmdIntoArray( PecOnOff);
							ProcessMenuPECOnOff();
							break;
						default:
							InsertLX200CmdIntoArray( LX200_Unknown_Cmd1);
					}
					break;
				case 'h':
					IncrIx++;
					switch ( LX200Queue[Ix1])
					{
						/* RB: Goes to home/park position */
						case 'P':
						  IncrIx++;
							InsertLX200CmdIntoArray( MoveHomePosition);
							TrackFlag = Off;
							ProcessMenuTrack();
							Response = MenuMoveHome;
							GetNewMenuCatSubFromResponse();
							UpdateMenuCatSub();
							/* park now */
							ProcessMenuSelection();
							break;
						default:
							InsertLX200CmdIntoArray( LX200_Unknown_Cmd1);
					}
					break;
				case 'r':
					/* RB: Field rotation command section */
					IncrIx++;
					switch ( LX200Queue[Ix1])
					{
						case '+':
						/* FR on*/
							IncrIx++;
							Response = MenuFRMotorTrack;
							GetNewMenuCatSubFromResponse();
							UpdateMenuCatSub();
							InsertLX200CmdIntoArray( FROn);
							FRMotorTrackOn = No;
							ProcessMenuFRMotorTrack();
							break;
						/* FR off*/
						case '-':
							Response = MenuFRMotorTrack;
							GetNewMenuCatSubFromResponse();
							UpdateMenuCatSub();
							IncrIx++;
							InsertLX200CmdIntoArray( FROff);
							FRMotorTrackOn = Yes;
							ProcessMenuFRMotorTrack();
							/* stop slew as meade specifics*/
							LX200_Motor_Cmd = StopSlew;
							AbortLX200_IRQ8Slew();
							break;
						default:
							InsertLX200CmdIntoArray( LX200_Unknown_Cmd1);
					}
					break;

				case 'A':
					IncrIx++;
					switch( LX200Queue[Ix1])
					{
						case 'A':
							IncrIx++;
							InsertLX200CmdIntoArray( AlignAltaz);
							InitConvertAltaz();
							break;
						/* ACK command */
						case 'C':
							IncrIx++;
							switch( LX200Queue[Ix2])
							{
								case 'K':
									IncrIx++;
									InsertLX200CmdIntoArray( AlignACK);
									/* Process ACK response; */
									switch (InitState)
									{
										case InitAltaz:
										case InitAltAlt:
											sprintf( StrBuf, "%s#","A");
										break;
										case InitNone:
											sprintf( StrBuf, "%s#","L");
										break;
										case InitCfgFile:
										case InitEquat:
										default:
											sprintf( StrBuf, "%s#","P");
									}
									WriteSerialString( LX200ComPort, StrBuf, strlen( StrBuf));
									break;
								default:
									InsertLX200CmdIntoArray( LX200_Unknown_Cmd1);
							}
							break;
						case 'L':
							IncrIx++;
							InsertLX200CmdIntoArray( AlignLand);
							/* conclude with tracking off: following function reverses the TrackFlag */
							TrackFlag = On;
							ProcessMenuTrack();
							break;
						case 'P':
							IncrIx++;
							InsertLX200CmdIntoArray( AlignPolar);
							InitConvertEquat();
							break;
						default:
							InsertLX200CmdIntoArray( LX200_Unknown_Cmd1);
					}
					break;
				/* increases (B+) or decreases (B-) reticle brightness, or sets to one of the flashing
				modes (B0, B1, B2 or B3) */
				case 'B':
					IncrIx++;
					switch( LX200Queue[Ix1])
					{
						case '+':
						case '-':
						case '0':
						case '1':
						case '2':
						case '3':
							IncrIx++;
							InsertLX200CmdIntoArray( ReticleCommand);
							break;
						default:
							InsertLX200CmdIntoArray( LX200_Unknown_Cmd1);
					}
					break;
				case 'C':
					IncrIx++;
					switch( LX200Queue[Ix1])
					{
						case 'M':
							IncrIx++;
							InsertLX200CmdIntoArray( Sync);
							Response = MenuResetEquat;
							GetNewMenuCatSubFromResponse();
							UpdateMenuCatSub();
							ProcessMenuSelection();
							LX200_Write_Ra();
							LX200_Write_Dec();
							break;
						default:
							InsertLX200CmdIntoArray( LX200_Unknown_Cmd1);
					}
					break;
				case 'D':
					IncrIx++;
					InsertLX200CmdIntoArray( GetDistance);
					LX200_GetDistance();
					break;
				case 'F':
					IncrIx++;
					switch( LX200Queue[Ix1])
					{
						case '+':
							IncrIx++;
							InsertLX200CmdIntoArray( FocusOut);
							LX200_Focus_Cmd = FocusOut;
							ProcessLX200FocusByMethod();
							break;
						case '-':
							IncrIx++;
							InsertLX200CmdIntoArray( FocusIn);
							LX200_Focus_Cmd = FocusIn;
							ProcessLX200FocusByMethod();
							break;
						case 'Q':
							IncrIx++;
							InsertLX200CmdIntoArray( FocusQuit);
							LX200_Focus_Cmd = FocusQuit;
							ProcessLX200FocusByMethod();
							break;
						case 'F':
							IncrIx++;
							InsertLX200CmdIntoArray( FocusSetFast);
							LX200_Focus_Speed_Cmd = FocusSetFast;
							break;
						case 'S':
							IncrIx++;
							InsertLX200CmdIntoArray( FocusSetSlow);
							LX200_Focus_Speed_Cmd = FocusSetSlow;
							break;
						default:
							InsertLX200CmdIntoArray( LX200_Unknown_Cmd1);
					}
					break;
				case 'G':
					IncrIx++;
					switch( LX200Queue[Ix1])
					{
						case 'A':
							IncrIx++;
							InsertLX200CmdIntoArray( GetAlt);
							LX200_Write_Alt();
							break;
						case 'a':
							IncrIx++;
							InsertLX200CmdIntoArray( GetLocalT12);
							LX200_Write_LocalT();
							break;
						case 'C':
							IncrIx++;
							InsertLX200CmdIntoArray( GetDate);
							LX200_Write_Date();
							break;
						case 'c':
							IncrIx++;
							InsertLX200CmdIntoArray( GetClockStatus);
							LX200_Write_Clock_Status();
							break;
						case 'D':
						case 'd':
							IncrIx++;
							InsertLX200CmdIntoArray( GetDec);
							LX200_Write_Dec();
							break;
						/* command is GF for get FIELD radius of the field operation */
						case 'F':
							IncrIx++;
							InsertLX200CmdIntoArray( GetField);
							LX200_Write_Field();
							break;
						case 'G':
							IncrIx++;
							InsertLX200CmdIntoArray( GetTz);
							LX200_Write_Tz();
							break;
						case 'g':
							IncrIx++;
							InsertLX200CmdIntoArray( GetLongitude);
							LX200_Write_Long();
							break;
						/* :GL# is get time in 24 hr format while :Ga# is get time in 12 hr format, but
						tests	with LX200 scopes show that both return 24 hr format */
						case 'L':
							IncrIx++;
							InsertLX200CmdIntoArray( GetLocalT24);
							LX200_Write_LocalT();
							break;
						/* site name */
						case 'M':
						case 'N':
						case 'O':
						case 'P':
							IncrIx++;
							InsertLX200CmdIntoArray( GetSiteName);
							LX200_GetSiteName();
							break;
						case 'q':
							IncrIx++;
							InsertLX200CmdIntoArray( GetMinQualityFind);
							LX200_Write_MinQualityFind();
							break;
						case 'R':
						case 'r':
							IncrIx++;
							InsertLX200CmdIntoArray( GetRa);
							LX200_Write_Ra();
							break;
						case 'S':
							IncrIx++;
							InsertLX200CmdIntoArray( GetSidT);
							LX200_Write_SidT();
							break;
						case 't':
							IncrIx++;
							InsertLX200CmdIntoArray( GetLat);
							LX200_Write_Lat();
							break;
						case 'V':
							IncrIx++;
							switch( LX200Queue[Ix2])
							{
								case 'F':
									IncrIx++;
									InsertLX200CmdIntoArray( GetFirmwareIDString);
									LX200_GetFirmwareIDString();
									break;
								/* customized extensions to the LX200 protocol */
								case 'P':
									IncrIx++;
									InsertLX200CmdIntoArray( GetProductName);
									LX200_GetProductName();
									break;
								case 'D':
									IncrIx++;
									InsertLX200CmdIntoArray( GetFirmwareDate);
									LX200_GetFirmwareDate();
									break;
								default:
									InsertLX200CmdIntoArray( LX200_Unknown_Cmd2);
							 }
							 break;
						case 'Z':
							IncrIx++;
							InsertLX200CmdIntoArray( GetAz);
							LX200_Write_Az();
							break;
						default:
							InsertLX200CmdIntoArray( LX200_Unknown_Cmd1);
					}
					break;
				case 'L':
					IncrIx++;
					switch( LX200Queue[Ix1])
					{
					/* library object information - might expect (from ACP):
					' Read the scope's current object info and display in
					' "friendly" format. The format of s.ObjectInformation() is:
					' 00000000011111111112222222223333
					' 12345678901234567890123456789012
					' --------------------------------
					'  NGC4594 VG GAL MAG 8.3 SZ  8.9'
					'  NGC0           Coordinates Only
					' STAR100     STARMAG 1.4
					'  JUPITER        MAG-2.7 SZ   46"
					'    M1    EX PNEBMAG 8.4 SZ  6.0'
					'    M20   EX OPNBMAG 6.3 SZ 29.0'
					'    M21   GD OPENMAG 5.9 SZ 13.0'
					'    M28   GD GLOBMAG 6.9 SZ 11.2'
					'    M78   VG DNEBMAG11.3 SZ  8.0'*/
						case 'I':
							IncrIx++;
							InsertLX200CmdIntoArray( LI_command);
							WriteSerial( LX200ComPort, '#');
							break;
						/* sets the NGC object library:  0 is the NGC library, 1 is the IC library and 2
						is the UGC library; value ignored */
						case 'o':
							InsertLX200CmdIntoArray( SetNGCLibrary);
							WriteSerial( LX200ComPort, LX200_OK);
							LX200_Set_IncrIx( Ix, &IncrIx);
							break;
						/* sets the STAR object library type: 0 is the STAR library; 1 is the SAO library,
						and 2 is the GCVS library; value ignored */
						case 's':
							InsertLX200CmdIntoArray( SetStarLibrary);
							WriteSerial( LX200ComPort, LX200_OK);
							LX200_Set_IncrIx( Ix, &IncrIx);
							break;
						default:
							InsertLX200CmdIntoArray( LX200_Unknown_Cmd1);
					}
					break;
				case 'M':
					IncrIx++;
					switch( LX200Queue[Ix1])
					{
						case 'n':
							IncrIx++;
							InsertLX200CmdIntoArray( MoveDirRateNorth);
							LX200_Motor_Cmd = MoveDirRateNorth;
							break;
						case 's':
							IncrIx++;
							InsertLX200CmdIntoArray( MoveDirRateSouth);
							LX200_Motor_Cmd = MoveDirRateSouth;
							break;
						case 'e':
							IncrIx++;
							InsertLX200CmdIntoArray( MoveDirRateEast);
							LX200_Motor_Cmd = MoveDirRateEast;
							break;
						case 'w':
							IncrIx++;
							InsertLX200CmdIntoArray( MoveDirRateWest);
							LX200_Motor_Cmd = MoveDirRateWest;
							break;
						case 'G':
						case 'S':
							IncrIx++;
							InsertLX200CmdIntoArray( StartSlew);
							WriteSerial( LX200ComPort, '0');
							LX200_Speed_Cmd = LX200_Motor_Cmd = StartSlew;
							break;
						default:
							InsertLX200CmdIntoArray( LX200_Unknown_Cmd1);
					}
					break;
				case 'Q':
					IncrIx++;
					switch( LX200Queue[Ix1])
					{
						case '#':
							InsertLX200CmdIntoArray( StopSlew);
							LX200_Motor_Cmd = StopSlew;
							AbortLX200_IRQ8Slew();
							break;
						case 'n':
							IncrIx++;
							InsertLX200CmdIntoArray( StopMotionNorth);
							LX200_Motor_Cmd = StopMotionNorth;
							AbortLX200_IRQ8Slew();
							break;
						case 's':
							IncrIx++;
							InsertLX200CmdIntoArray( StopMotionSouth);
							LX200_Motor_Cmd = StopMotionSouth;
							AbortLX200_IRQ8Slew();
							break;
						case 'e':
							IncrIx++;
							InsertLX200CmdIntoArray( StopMotionEast);
							AbortLX200_IRQ8Slew();
							LX200_Motor_Cmd = StopMotionEast;
							break;
						case 'w':
							IncrIx++;
							InsertLX200CmdIntoArray( StopMotionWest);
							LX200_Motor_Cmd = StopMotionWest;
							AbortLX200_IRQ8Slew();
							break;
						default:
							InsertLX200CmdIntoArray( LX200_Unknown_Cmd1);
					}
					break;
				case 'R':
					IncrIx++;
					switch( LX200Queue[Ix1])
					{
						case 'G':
							IncrIx++;
							InsertLX200CmdIntoArray( SetMotionRateGuide);
							LX200_Speed_Cmd = SetMotionRateGuide;
							break;
						case 'C':
							IncrIx++;
							InsertLX200CmdIntoArray( SetMotionRateCenter);
							LX200_Speed_Cmd = SetMotionRateCenter;
							break;
						case 'M':
							IncrIx++;
							InsertLX200CmdIntoArray( SetMotionRateFind);
							LX200_Speed_Cmd = SetMotionRateFind;
							break;
						case 'S':
							IncrIx++;
							InsertLX200CmdIntoArray( SetMotionRateSlew);
							LX200_Speed_Cmd = SetMotionRateSlew;
							break;
						case 'g':
							InsertLX200CmdIntoArray( ReadMsArcsecSec);
							GuideArcsecSec = LX200_Read_Int_From_Queue( Ix+2);
							IncrIx++;
							IncrIx++;
							LX200_Set_IncrIx( Ix, &IncrIx);
							// update display
							WriteGuideArcsecSec();
						default:
							InsertLX200CmdIntoArray( LX200_Unknown_Cmd1);
					}
					break;
				case 'S':
					IncrIx++;
					switch( LX200Queue[Ix1])
					{
						/* sets the brighter magnitude limit for the FIND operation; example is Sb+08.2
						value ignored */
						case 'b':
							InsertLX200CmdIntoArray( SetBrightMagLimitFindOperation);
							WriteSerial( LX200ComPort, LX200_OK);
							LX200_Set_IncrIx( Ix, &IncrIx);
							break;
						case 'C':
							InsertLX200CmdIntoArray( SetDate);
							LX200_Read_CurrentDate( Ix);
							LX200_Set_IncrIx( Ix, &IncrIx);
							break;
						case 'd':
							InsertLX200CmdIntoArray( SetDec);
							LX200_Read_Dec( Ix);
							LX200_Set_IncrIx( Ix, &IncrIx);
							break;
						/* sets the field radius of the FIELD operation; example SF 010 value ignored */
						case 'F':
							InsertLX200CmdIntoArray( SetField);
							WriteSerial( LX200ComPort, LX200_OK);
							LX200_Set_IncrIx( Ix, &IncrIx);
							break;
						/* sets the fainter magnitude limit for the FIND operation; example is Sf-02.0
						value ignored */
						case 'f':
							InsertLX200CmdIntoArray( SetFaintMagLimitFindOperation);
							WriteSerial( LX200ComPort, LX200_OK);
							LX200_Set_IncrIx( Ix, &IncrIx);
							break;
						/* set GMT offset; ignored */
						case 'G':
							InsertLX200CmdIntoArray( SetGMTOffset);
							WriteSerial( LX200ComPort, LX200_OK);
							LX200_Set_IncrIx( Ix, &IncrIx);
							break;
						/* set longitude; ignored */
						case 'g':
							InsertLX200CmdIntoArray( SetLat);
							WriteSerial( LX200ComPort, LX200_OK);
							LX200_Set_IncrIx( Ix, &IncrIx);
							break;
						/* Sh DD# sets the current 'higher' limit; value ignored */
						case 'h':
							InsertLX200CmdIntoArray( SetCurrentHigherLimit);
							WriteSerial( LX200ComPort, LX200_OK);
							LX200_Set_IncrIx( Ix, &IncrIx);
							break;
						case 'L':
							InsertLX200CmdIntoArray( SetLocalT);
							LX200_Read_LocalT( Ix);
							LX200_Set_IncrIx( Ix, &IncrIx);
							break;
						/* sets the larger size limit for the FIND operation */
						case 'l':
							InsertLX200CmdIntoArray( LargeSizeLimitFindOperation);
							WriteSerial( LX200ComPort, LX200_OK);
							LX200_Set_IncrIx( Ix, &IncrIx);
							break;
						/* set site number; ignored */
						case 'M':
						case 'N':
						case 'O':
						case 'P':
							InsertLX200CmdIntoArray( SetSiteNumber_S);
							WriteSerial( LX200ComPort, LX200_OK);
							LX200_Set_IncrIx( Ix, &IncrIx);
							break;
						/* steps to the next minimum quality for the FIND operation */
						case 'q':
							IncrIx++;
							InsertLX200CmdIntoArray( NextMinQualityFind);
							LX200MinQualityFind++;
							if( LX200MinQualityFind > 6)
								LX200MinQualityFind = 0;
							break;
						case 'r':
							InsertLX200CmdIntoArray( SetRa);
							LX200_Read_Ra( Ix);
							LX200_Set_IncrIx( Ix, &IncrIx);
							break;
						case 'S':
							InsertLX200CmdIntoArray( SetSidT);
							LX200_Read_SidT( Ix);
							LX200_Set_IncrIx( Ix, &IncrIx);
							break;
						/* sets the smaller size limit for the FIND operation */
						case 's':
							InsertLX200CmdIntoArray( SmallSizeLimitFindOperation);
							WriteSerial( LX200ComPort, LX200_OK);
							LX200_Set_IncrIx( Ix, &IncrIx);
							break;
						/* set latitude; ignored */
						case 't':
							InsertLX200CmdIntoArray( SetLat);
							WriteSerial( LX200ComPort, LX200_OK);
							LX200_Set_IncrIx( Ix, &IncrIx);
							break;
						/* max slew rate in deg/sec */
						case 'w':
							IncrIx++;
							InsertLX200CmdIntoArray( Sw_command);
							WriteSerial( LX200ComPort, LX200_OK);
							break;
						/* command is 'SyGPDCO' and gets the ‘type' string for the FIND operation:
						capitalized means on, small letters means off: galaxy, planetary nebula, diffuse,
						globular cluster,	open cluster: ACP sends SyGPdco */
						case 'y':
							IncrIx++;
							InsertLX200CmdIntoArray( SetTypeStringForFind);
							WriteSerial( LX200ComPort, LX200_OK);
							break;
						default:
							InsertLX200CmdIntoArray( LX200_Unknown_Cmd1);
					}
					break;
				case 'T':
					IncrIx++;
					switch( LX200Queue[Ix1])
					{
						case 'Q':
							IncrIx++;
							InsertLX200CmdIntoArray( TimeQuartz);
							Response = MenuZeroDrift;
							GetNewMenuCatSubFromResponse();
							UpdateMenuCatSub();
							ProcessMenuSelection();
							break;
						default:
							InsertLX200CmdIntoArray( LX200_Unknown_Cmd1);
					}
					break;
				case 'U':
					IncrIx++;
					InsertLX200CmdIntoArray( ToggleLongFormat);
					LX200_LongFormat = !LX200_LongFormat;
					break;
				case 'W':
					IncrIx++;
					switch( LX200Queue[Ix1])
					{
						/* set current site number */
						case '1':
						case '2':
						case '3':
						case '4':
							IncrIx++;
							InsertLX200CmdIntoArray( SetSiteNumber_W);
							break;
						default:
							InsertLX200CmdIntoArray( LX200_Unknown_Cmd1);
					}
					break;

				/* customized extensions to the LX200 protocol */
				case 'X':
					IncrIx++;
					switch( LX200Queue[Ix1])
					{
						case 'G':
							IncrIx++;
							switch( LX200Queue[Ix2])
							{
								/* XGf get focuser slow speed */
								case 'f':
									IncrIx++;
									InsertLX200CmdIntoArray( GetFfspeed);
									LX200_Custom_GetFocusSpeed(0);
									break;
								/* XGF get focuser fast speed */
								case 'F':
									IncrIx++;
									InsertLX200CmdIntoArray( GetFFspeed);
									LX200_Custom_GetFocusSpeed(1);
									break;
								case 'G':
									IncrIx++;
									InsertLX200CmdIntoArray( SendGuideArcsecSec);
									sprintf( StrBuf, "%04d#", GuideArcsecSec);
									if( MoveHsUnderway)
										WriteSerialStringPauseUntilXmtFinished( LX200ComPort, StrBuf, strlen( StrBuf));
									else
										WriteSerialString( LX200ComPort, StrBuf, strlen( StrBuf));
									break;
								case 'M':
									IncrIx++;
									InsertLX200CmdIntoArray( SendMsArcsecSec);
									sprintf( StrBuf, "%04d#", MsArcsecSec);
									if( MoveHsUnderway)
										WriteSerialStringPauseUntilXmtFinished( LX200ComPort, StrBuf, strlen( StrBuf));
									else
										WriteSerialString( LX200ComPort, StrBuf, strlen( StrBuf));
									break;
								/* XGP get focuser position */
								case 'P':
									IncrIx++;
									InsertLX200CmdIntoArray( GetFocusPos);
									LX200_Custom_GetFocusPosition();
									break;
								 /* send back field rotation in degrees (e.g. 000.00 to 359.99) */
								 case 'R':
									IncrIx++;
									InsertLX200CmdIntoArray( SendFieldR);
									sprintf( StrBuf, "%06.2f#", FieldR*RadToDeg);
									if( MoveHsUnderway)
										WriteSerialStringPauseUntilXmtFinished( LX200ComPort, StrBuf, strlen( StrBuf));
									else
										WriteSerialString( LX200ComPort, StrBuf, strlen( StrBuf));
									break;
								default:
									InsertLX200CmdIntoArray( LX200_Unknown_Cmd2);
							}
							break;
						case 'H':
							IncrIx++;
							if( LX200Queue[Ix2] >= 'a' && LX200Queue[Ix2] < 'a'+ MaxHandpadFlag)
							{
								IncrIx++;
								HandpadFlag = LX200Queue[Ix2] - 'a';
								InsertLX200CmdIntoArray( SetHandpadMode);
								WriteHandpadStatus();
							}
							else
								if( LX200Queue[Ix2] == 'L')
								{
									IncrIx++;
									InsertLX200CmdIntoArray( HandpadLeftKey);
									Buttons = LeftKey;
									DisplayButtonsStatus();
									ProcessHPEventsModeSwitch();
									Buttons = HoldButtons;
									LX200LeftButton = True;
								}
								else
									if( LX200Queue[Ix2] == 'R')
									{
										IncrIx++;
										InsertLX200CmdIntoArray( HandpadRightKey);
										Buttons = RightKey;
										DisplayButtonsStatus();
										ProcessHPEventsModeSwitch();
										Buttons = HoldButtons;
										LX200RightButton = True;
									}
									else
										InsertLX200CmdIntoArray( LX200_Unknown_Cmd2);
							break;
						case 'I':
							IncrIx++;
							switch( LX200Queue[Ix2])
							{
								case '1':
									IncrIx++;
									InsertLX200CmdIntoArray( SetInit1);
									/* use Current altaz and In equat coordinates to init with */
									Current.Ra = In.Ra;
									Current.Dec = In.Dec;
									strcpy( WhyInit, WHY_INIT_LX200);
									KBEventInitMatrix( 1);
									break;
								case '2':
									IncrIx++;
									InsertLX200CmdIntoArray( SetInit2);
									if( One.Init)
									{
										/* use Current altaz and In equat coordinates to init with */
										Current.Ra = In.Ra;
										Current.Dec = In.Dec;
										strcpy( WhyInit, WHY_INIT_LX200);
										KBEventInitMatrix( 2);
									}
									else
										PressKeyToContMsg( "LX200 Init2 command received but no init 1");
									break;
								case '3':
									IncrIx++;
									InsertLX200CmdIntoArray( SetInit3);
									if( One.Init && Two.Init)
									{
										/* use Current altaz and In equat coordinates to init with */
										Current.Ra = In.Ra;
										Current.Dec = In.Dec;
										strcpy( WhyInit, WHY_INIT_LX200);
										KBEventInitMatrix( 3);
									}
									else
										PressKeyToContMsg( "LX200 Init3 command received but no init 2");
									break;
								default:
									InsertLX200CmdIntoArray( LX200_Unknown_Cmd2);
							}
							break;
						case 'S':
							IncrIx++;
							switch( LX200Queue[Ix2])
							{
								// Set focus slow speed
								case 'f':
									InsertLX200CmdIntoArray( SetFfspeed);
									LX200_Custom_SetFocusSpeed(0, LX200_Read_Int_From_Queue( Ix+3));
									LX200_Set_IncrIx( Ix, &IncrIx);
									break;
								// Set focus fast speed
								case 'F':
									InsertLX200CmdIntoArray( SetFFspeed);
									LX200_Custom_SetFocusSpeed(1, LX200_Read_Int_From_Queue( Ix+3));
									LX200_Set_IncrIx( Ix, &IncrIx);
									break;
								case 'G':
									InsertLX200CmdIntoArray( ReadGuideArcsecSec);
									GuideArcsecSec = LX200_Read_Int_From_Queue( Ix+3);
									LX200_Set_IncrIx( Ix, &IncrIx);
									// update display
									WriteGuideArcsecSec();
									break;
								case 'M':
									InsertLX200CmdIntoArray( ReadMsArcsecSec);
									MsArcsecSec = LX200_Read_Int_From_Queue( Ix+3);
									LX200_Set_IncrIx( Ix, &IncrIx);
									InitMsTickVars( MsArcsecSec);
									WriteMsArcsecSec();
									break;
								/* set Object Name via LX200 */
								case 'N':
									IncrIx++;
										  InsertLX200CmdIntoArray( SetObjectName);
									IxQ = Ix + IncrIx - 1;
									IxQ %= LX200QueueSize;
									for( IxS = 0; IxS < MaxObjectName && LX200Queue[IxQ] != '#'; IxS++)
									{
										 LX200_Object_Name[IxS] = LX200Queue[IxQ];
										 IxQ %= LX200QueueSize;
										 IxQ++;
									}
									for( ;IxS < MaxObjectName; IxS++)
									  LX200_Object_Name[IxS] = EndOfStr;
									LX200_Set_IncrIx( Ix, &IncrIx);
									break;
								default:
									InsertLX200CmdIntoArray( LX200_Unknown_Cmd2);
							}
							break;
						case 'X':
							IncrIx++;
							InsertLX200CmdIntoArray( LX200_Clear_Display);
							BlankOutLX200DisplayAreas();
							break;
						case 'Z':
							IncrIx++;
							InsertLX200CmdIntoArray( LX200_StringCommand);
							IxQ = Ix + IncrIx - 1;
							IxQ %= LX200QueueSize;
							for( IxS = 0; IxS < MaxStrIx && LX200Queue[IxQ] != '#'; IxS++)
							{
								ParsedString.Str[IxS] = LX200Queue[IxQ];
								IxQ %= LX200QueueSize;
								IxQ++;
							}
							for( ;IxS < MaxStrIx; IxS++)
								ParsedString.Str[IxS] = EndOfStr;
							LX200_Set_IncrIx( Ix, &IncrIx);
							LoadScrollFileFromParsedString();
							ScrollFlag = Yes;
							HoldLX200ComPort = LX200ComPort;
							LX200ComPort = 0;
							ProcessScroll();
							LX200ComPort = HoldLX200ComPort;
							break;
						default:
							InsertLX200CmdIntoArray( LX200_Unknown_Cmd1);
					}
					break;
				default:
					InsertLX200CmdIntoArray( LX200_Unknown_Cmd0);
			}
			LX200CmdProcessingUnderway = False;
		}
		Ix += IncrIx;
		Ix %= LX200QueueSize;
	}
}

void AbortLX200_IRQ8Slew( void)
{
	if( SlewState != SlewDone)
	{
		disable();
		AbortState = LX200Abort;
		enable();
	}
}

void ProcessLX200_Motor_Cmd( void)
{
	struct AZLongV HoldSteps;
	static LX200_COMMAND_TYPE LX200_Hold_Motor_Cmd;
	int timer = 0;

	if( LX200_Speed_Cmd == SetMotionRateGuide && !(LX200_Motor_Cmd == StartSlew))
	{
		if (!GuideFlag)
			InitGuide();
	}
	else
		if (GuideFlag)
			StopGuide();


	if( LX200_Motor_Cmd == StartSlew)
	{
		if( LX200DisplayFlag)
			DisplayLX200CommandsAndCharBuffer();
		LX200_Motor_Cmd = NoAck;
		Response = MenuMoveEquat;
		GetNewMenuCatSubFromResponse();
		UpdateMenuCatSub();
		/* this function sets Current to In (it is assumed that just prior to issuing the StartSlew
		command, Ra and Dec coordinates were read in from the LX200 port), and turns on tracking */
		ProcessMenuSelection();
		/* if no intervention, MoveToCurrentRaDec() eventually called from CheckMiscEvents() which
		is eventually called from ProcessHPEvents() which is eventually called every bios clock tick
		from main() */
	}
	else
		if( LX200_Speed_Cmd == SetMotionRateCenter || LX200_Speed_Cmd == SetMotionRateFind)
		{
			Steps.A = Steps.Z = 0;

			/* can move in one direction only at a time */
			switch( LX200_Motor_Cmd)
			{
				case MoveDirRateNorth:
					Steps.A = MsTick.A;
					Dir.A = CW;
					break;
				case MoveDirRateSouth:
					Steps.A = MsTick.A;
					Dir.A = CCW;
					break;
				case MoveDirRateEast:
					Steps.Z = MsTick.Z;
					Dir.Z = CCW;
					break;
				case MoveDirRateWest:
					Steps.Z = MsTick.Z;
					Dir.Z = CW;
			}
			/* keeps control until Ms mvmt ends; limited in time */
			HoldSteps = Steps;
			LX200_Hold_Motor_Cmd = LX200_Motor_Cmd;
			while( LX200_Hold_Motor_Cmd == LX200_Motor_Cmd
			&& timer++ < LX200MotionTimeoutSec * ClockTicksSec)
			{
				HPEventMoveMs();
				HPEventGetEquat();
				/* ReadLX200Input() calls ProcessLX200Cmds(): resetting of LX200_Motor_Cmd done in
				ProcessLX200Cmds() */
				ReadLX200Input();
				SequentialTaskController();
				/* add undone steps to steps to do for next clock tick */
				Steps.A += HoldSteps.A;
				Steps.Z += HoldSteps.Z;
			}
			AlignMs_f_ptr();
			SetCurrentAltazToAccumMs();
			HPEventGetEquat();
		}
		else
			if( LX200_Speed_Cmd == SetMotionRateGuide)
			{
				if( HandpadFlag == GuideStayRotateOn)
					switch( LX200_Motor_Cmd)
					{
						case MoveDirRateNorth:
							AccumGuide.A -= cos( GuideFRAngle) * GuideRadTick;
							AccumGuide.Z -= sin( GuideFRAngle) * GuideRadTick;
							break;
						case MoveDirRateSouth:
							AccumGuide.A += cos( GuideFRAngle) * GuideRadTick;
							AccumGuide.Z += sin( GuideFRAngle) * GuideRadTick;
							break;
						case MoveDirRateWest:
							AccumGuide.Z -= cos( GuideFRAngle) * GuideRadTick;
							AccumGuide.A += sin( GuideFRAngle) * GuideRadTick;
							break;
						case MoveDirRateEast:
							AccumGuide.Z += cos( GuideFRAngle) * GuideRadTick;
							AccumGuide.A -= sin( GuideFRAngle) * GuideRadTick;
					}
				else
					switch( LX200_Motor_Cmd)
					{
						/* GuideFlag must be set to Yes for guiding corrections to work */
						case MoveDirRateNorth:
							AccumGuide.A -= GuideRadTick;
							break;
						case MoveDirRateSouth:
							AccumGuide.A += GuideRadTick;
							break;
						case MoveDirRateWest:
							AccumGuide.Z -= GuideRadTick;
							break;
						case MoveDirRateEast:
							AccumGuide.Z += GuideRadTick;
					}
				if( TrackByRateFlag)
					TrackByRate();
				else
					MoveToCurrentRaDec();
			}
			else
				if( LX200_Speed_Cmd == SetMotionRateSlew && LX200_OpenEndedSlewOn == False)
				{
					Steps.A = Steps.Z = 0;
					switch( LX200_Motor_Cmd)
					{
						case MoveDirRateNorth:
							Steps.A = LX200SlewHs;
							Dir.A = CW;
							break;
						case MoveDirRateSouth:
							Steps.A = LX200SlewHs;
							Dir.A = CCW;
							break;
						case MoveDirRateEast:
							Steps.Z = LX200SlewHs;
							Dir.Z = CCW;
							break;
						case MoveDirRateWest:
							Steps.Z = LX200SlewHs;
							Dir.Z = CW;
					}
					if( Steps.A > 0 || Steps.Z > 0)
					{
						LX200_OpenEndedSlewOn = True;
						KBEventMoveHs();
						PauseUntilNewSidTime();
						HPEventGetEquat();
						LX200_OpenEndedSlewOn = False;
					}
					else
						if( TrackByRateFlag)
							TrackByRate();
						else
							MoveToCurrentRaDec();
					LX200_Motor_Cmd = NoAck;
				}
}

void ProcessLX200FocusByMethod( void)
{
	InitiatedType = LX200Initiated;

	if( LX200_Focus_Cmd == FocusIn)
		FocusControl = FocusMinus;
	else
		if( LX200_Focus_Cmd == FocusOut)
			FocusControl = FocusPlus;
		else
			FocusControl = FocusStop;

	if( FocusMethod == FocusMethod_OnOff_16_17)
		ProcessLX200Focus_OnOff_16_17();
	else
		if( FocusMethod == FocusMethod_OnOff_16_17_Slow1_14)
			ProcessLX200Focus_OnOff_16_17_Slow1_14();
		else
			if( FocusMethod == FocusMethod_Pulse_1_14 || FocusMethod == FocusMethod_Pulse_16_17)
				/* if tracking on, control of focus motor handled by MoveMs() */
				if( TrackFlag)
					;
				else
					if( !InsideProcessHPEventsFocusMethod_Pulse_Dir)
					{
						// necessary to declare command finished otherwise ReadLX200Input() will always
						// return false and no further LX200 command processing will occur once we are
						// inside the while loop of ProcessHPEventsFocusMethod_Pulse_Dir();
						LX200CmdProcessingUnderway = False;
						ProcessHPEventsFocusMethod_Pulse_Dir();
					}
}

void ProcessLX200Focus_OnOff_16_17( void)
{
	FocusControl_OnOff_16_17();
}

void ProcessLX200Focus_OnOff_16_17_Slow1_14( void)
{
	if( LX200_Focus_Speed_Cmd == FocusSetFast)
		ProcessLX200Focus_OnOff_16_17();
	else
		FocusControl_OnOff_16_17_Slow1_14();
}

