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

void WriteFocusPosition( void)
{
	static int PrevFocusPosition = -9999;

	if( FocusPosition != PrevFocusPosition)
	{
		PrevFocusPosition = FocusPosition;
		TextAttr = DisplayText;
		VidMemXY = DisplayXY[DisplayFocusPosition];
		sprintf( StrBuf, "%5d", FocusPosition);
		WriteStrBufToScreen_f_ptr();
	}
}

void WriteHandpadStatus( void)
{
	TextAttr = DisplayText;
	VidMemXY = DisplayXY[DisplayHandpad];

	sprintf( StrBuf, HPStr[HandpadFlag]);
	WriteStrBufToScreen_f_ptr();

	if( HandpadFlag == GuideOn && GuideFlag ||
		 HandpadFlag == GuideStayOn && GuideFlag ||
		 HandpadFlag == GuideStayRotateOn && GuideFlag ||
		 HandpadFlag == GuideDragOn && GuideFlag ||
		 HandpadFlag == ScrollTour && ScrollUnderway ||
		 HandpadFlag == ScrollAutoTour && ScrollUnderway ||
		 HandpadFlag == HandpadAux && HandpadAuxControlOnFlag ||
		 HandpadFlag == HandpadFR && HandpadFRMotorControlOnFlag ||
		 HandpadFlag == HandpadFocus && HandpadFocusMotorControlOnFlag)

		SprintfOn;
	else
		Sprintf2Blanks;
	WriteStrBufToScreen_f_ptr();

	VidMemXY.X++;
	if( HPUpdateDriftFlag)
		sprintf( StrBuf, "DUon ");
	else
		sprintf( StrBuf, "DUoff");
	WriteStrBufToScreen_f_ptr();

	VidMemXY.X++;
	/* if writing to guide file */
	if (GuideArrayFlag == WritingToGuideArray)
		sprintf( StrBuf, "Saving");
	else
		sprintf( StrBuf, "      ");
	WriteStrBufToScreen_f_ptr();
}

void WriteAccumGuide( void)
{
	TextAttr = CurrentText;
	VidMemXY = DisplayXY[DisplayAccumGuide];
	sprintf( StrBuf, "AccumGuide Alt: %3.1f\" Az: %3.1f\" %s", AccumGuide.A*RadToArcsec, AccumGuide.Z*RadToArcsec,
				GuideArrayFlag==WritingToGuideArray?"Saving":GuideArrayFlag==ReadyToSaveGuideArray?"Saved ":"      ");
	WriteStrBufToScreen_f_ptr();
}

void WriteRemoveAccumGuide( void)
{
	DrawScreenLine( DisplayLine1Y);
}

void WriteTrackByRate( void)
{
	TextAttr = CurrentText;
	VidMemXY = DisplayXY[DisplayTrackByRate];
	if( TrackByRateFlag)
	{
		sprintf( StrBuf, " Microsteps/sec: %2.1f %2.1f Accel (ms/tick): %2.1f %2.1f press 't' to stop ",
		TrackMsPerSec.A, TrackMsPerSec.Z, TrackMsAccel.A, TrackMsAccel.Z);
		WriteStrBufToScreen_f_ptr();
	}
	else
		DrawScreenLine( DisplayLine1Y);
}

void WriteTrackStatus( void)
{
	VidMemXY = DisplayXY[DisplayTrack];
	if( TrackFlag)
	{
		TextAttr = DisplayText;
		sprintf( StrBuf, "On ");
	}
	else
	{
		TextAttr = DisplayText;
		sprintf( StrBuf, "Off");
	}
	WriteStrBufToScreen_f_ptr();
}

void WriteFRMotorTrackStatus( void)
{
	VidMemXY = DisplayXY[DisplayFRMotorTrack];
	if( FRMotorTrackOn)
	{
		TextAttr = DisplayText;
		sprintf( StrBuf, "On ");
	}
	else
	{
		TextAttr = DisplayText;
		sprintf( StrBuf, "Off");
	}
	WriteStrBufToScreen_f_ptr();
}

/* writing of PEC values to screen done in SequentialLowPriController(): this function called from
 CheckAltPECSynch(), CheckAzPECSynch(), ProcessMenuPECOnOff(), ProcessMenuPECSynchAlt(),
 ProcessMenuPECSynchAz(), and InitKBEvent() */
void WritePECSynchStatus( void)
{
	int i;

	if( PECFlag)
	{
		if( DisplayAltAutoSynchFlag)
		{
			/* if synch occurs late while moving CW, the discrepancy will be positive */
			i = PECIxOffset.A - LastPECIxOffset.A;
			if( i > PECSize/2)
				i -= PECSize;
			if( i < -PECSize/2)
				i += PECSize;
			if( i)
				TextAttr = CurrentText;
			else
				TextAttr = DisplayText;
			sprintf( StrBuf, "%3d", i);
			VidMemXY = DisplayXY[DisplayPEC];
			VidMemXY.X += 16;
			WriteStrBufToScreen_f_ptr();
		}
		if( DisplayAzAutoSynchFlag)
		{
			i = PECIxOffset.Z - LastPECIxOffset.Z;
			if( i > PECSize/2)
				i -= PECSize;
			if( i < -PECSize/2)
				i += PECSize;
			if( i)
				TextAttr = CurrentText;
			else
				TextAttr = DisplayText;
			sprintf( StrBuf, "%3d", i);
			VidMemXY = DisplayXY[DisplayPEC];
			VidMemXY.X += 20;
			WriteStrBufToScreen_f_ptr();
		}
	}
	else
	{
		VidMemXY.Y = DisplayXY[DisplayPEC].Y;
		for( Ix = 0, VidMemXY.X = DisplayXY[DisplayPEC].X; Ix < 23; Ix++)
		{
			WriteCharToScreen_f_ptr( ' ');
			VidMemXY.X++;
		}
	}
}

void WriteMsArcsecSec( void)
{
	TextAttr = DisplayText;
	VidMemXY = DisplayXY[DisplayMsSpeed];
	sprintf( StrBuf, "%4d", MsArcsecSec);
	WriteStrBufToScreen_f_ptr();
}

void WriteGuideArcsecSec( void)
{
	TextAttr = DisplayText;
	VidMemXY = DisplayXY[DisplayGuideSpeed];
	sprintf( StrBuf, "%3d", GuideArcsecSec);
	WriteStrBufToScreen_f_ptr();
}

void WriteFocusFastSpeed( void)
{
	TextAttr = DisplayText;
	VidMemXY = DisplayXY[DisplayFocusFast];
	sprintf( StrBuf, "%3d", FocusFastStepsSec);
	WriteStrBufToScreen_f_ptr();
}

void WriteFocusSlowSpeed( void)
{
	TextAttr = DisplayText;
	VidMemXY = DisplayXY[DisplayFocusSlow];
	sprintf( StrBuf, "%3d", FocusSlowStepsSec);
	WriteStrBufToScreen_f_ptr();
}

Flag InputAltaz( struct Position* P, int X, int Y)
{
	struct Position Hold;

	Hold.Alt = P->Alt;
	Hold.Az = P->Az;
	gotoxy( X, Y++);
	printf( "Alt ");
	if( !GetDouble( &P->Alt))
	{
		P->Alt = Hold.Alt;
		return False;
	}
	gotoxy( X, Y++);
	printf( "Az  ");
	if( !GetDouble( &P->Az))
	{
		P->Alt = Hold.Alt;
		P->Az = Hold.Az;
		return False;
	}
	P->Alt /= RadToDeg;
	P->Az /= RadToDeg;
	return True;
}

Flag InputEquat( struct Position* P, int X, int Y)
{
	gotoxy( X, Y++);
	printf( "RaHr   ");
	if( !GetInt( &P->RaHMSH.Hr))
		return False;
	gotoxy( X, Y++);
	printf( "RaMin  ");
	if( !GetInt( &P->RaHMSH.Min))
		return False;
	gotoxy( X, Y++);
	printf( "RaSec  "); 
	if( !GetInt( &P->RaHMSH.Sec))
		return False;
	gotoxy( X, Y++); 
	printf( "DecDeg ");
	if( !GetInt( &P->DecDMS.Deg))
		return False; 
	gotoxy( X, Y++);
	printf( "DecMin ");
	if( !GetInt( &P->DecDMS.Min))
		return False;
	gotoxy( X, Y++);
	printf( "DecSec ");
	if( !GetInt( &P->DecDMS.Sec))
		return False;

	P->RaHMSH.Sign = Plus;
	if( P->RaHMSH.Hr < 0)
	{
		P->RaHMSH.Sign = Minus;
		P->RaHMSH.Hr = -P->RaHMSH.Hr;
	}
	if( P->RaHMSH.Min < 0)
	{
		P->RaHMSH.Sign = Minus;
		P->RaHMSH.Min = -P->RaHMSH.Min;
	}
	if( P->RaHMSH.Sec < 0)
	{
		P->RaHMSH.Sign = Minus;
		P->RaHMSH.Sec = -P->RaHMSH.Sec;
	}
	P->RaHMSH.HundSec = 0;

	P->DecDMS.Sign = Plus;
	if( P->DecDMS.Deg < 0)
	{
		P->DecDMS.Sign = Minus;
		P->DecDMS.Deg = -P->DecDMS.Deg; 
	}
	if( P->DecDMS.Min < 0)
	{
		P->DecDMS.Sign = Minus;
		P->DecDMS.Min = -P->DecDMS.Min;
	}
	if( P->DecDMS.Sec < 0)
	{
		P->DecDMS.Sign = Minus;
		P->DecDMS.Sec = -P->DecDMS.Sec;
	}

	CalcRadFromHMSH( &P->Ra, P->RaHMSH);
	CalcRadFromDMS( &P->Dec, P->DecDMS);
	return True;
}

void DisplayDrift( void)
{
	TextAttr = DisplayText;
	VidMemXY = DisplayXY[DisplayDriftAlt];
	VidMemDeg( Drift.Alt*ArcsecPerDeg);
	VidMemXY = DisplayXY[DisplayDriftAz];
	VidMemDeg( Drift.Az*ArcsecPerDeg);
	VidMemXY = DisplayXY[DisplayDriftRa];
	VidMemRaSHMS( &Drift);
	VidMemXY = DisplayXY[DisplayDriftDec];
	VidMemDecDMS( &Drift);
}

void DisplayLX200CommandsAndCharBuffer( void)
{
	for( Ix = 0; Ix < LX200_Cmd_Array_Size; Ix++)
		sprintf( StrBuf+Ix*LX200_CMD_STR_SIZE, " %s", LX200_Cmd_Str[LX200_Cmd_Array[Ix]]);
	TextAttr = DisplayText;
	VidMemXY = DisplayXY[DisplayLX200];
	WriteStrBufToScreen_f_ptr();
	sprintf( StrBuf, " %04d", LX200_Cmd_Count);
	WriteStrBufToScreen_f_ptr();
	/* highlight current command: 2 chars */
	VidMemXY.X = DisplayXY[DisplayLX200].X + 1 +	LX200_CMD_STR_SIZE * LX200_Cmd_Array_Ix;
	Screen[VidMemXY.Y][VidMemXY.X++].Attr = CurrentText;
	Screen[VidMemXY.Y][VidMemXY.X].Attr = CurrentText;
	/* display LX200 char buffer */
	for( Ix = 0; Ix < LX200QueueSize; Ix++)
		sprintf( StrBuf+Ix, "%c", (char) LX200Queue[Ix]);
	VidMemXY = DisplayXY[DisplayLX200_2];
	TextAttr = DisplayText;
	WriteStrBufToScreen_f_ptr();
	/* highlight last received char from LX200 port */
	VidMemXY.X = 2 + LX200_Ix;
	Screen[VidMemXY.Y][VidMemXY.X].Attr = CurrentText;
}

void DisplayButtonsStatus( void)
{
	sprintf( StrBuf, "%c%c%c%c%c%c%c%c",
	MsSpeed? 'm': 'H',
	Buttons & LeftKey? 'L': ' ',
	Buttons & RightKey? 'R': ' ',
	Buttons & UpKey? 'U': ' ',
	Buttons & DownKey? 'D': ' ',
	Buttons & CWKey? 'C': ' ',
	Buttons & CCWKey? 'W': ' ',
	UpDownButtonsReversedFlag? 'x': ' ');

	VidMemXY = DisplayXY[DisplayButtons];
	TextAttr = DisplayText;
	WriteStrBufToScreen_f_ptr();
}

void DisplayFRFocusAuxActionStatus( void)
{
	char FRStr[4];
	char AuxStr[4];
	char FocusStr[4];

	if( DisplayBiDirOut != HoldDisplayBiDirOut)
	{
		HoldDisplayBiDirOut = DisplayBiDirOut;
		if( DisplayBiDirOut & DisplayFRCCWBit)
		{
			if( ReverseFRMotor)
				strcpy( FRStr, "Fr+");
			else
				strcpy( FRStr, "Fr-");
		}
		else if( DisplayBiDirOut & DisplayFRCWBit)
		{
			if( ReverseFRMotor)
				strcpy( FRStr, "Fr-");
			else
				strcpy( FRStr, "Fr+");
		}
		else
			strcpy( FRStr, "   ");
		if( DisplayBiDirOut & DisplayAux1Bit)
			strcpy( AuxStr, "A1 ");
		else if( DisplayBiDirOut & DisplayAux14Bit)
			strcpy( AuxStr, "A14");
		else if( DisplayBiDirOut & DisplayAux16Bit)
			strcpy( AuxStr, "A16");
		else if( DisplayBiDirOut & DisplayAux17Bit)
			strcpy( AuxStr, "A17");
		else
			strcpy( AuxStr, "   ");
		if( DisplayBiDirOut & DisplayFocusInBit || DisplayBiDirOut & DisplayFocusOutBit)
		{
			FocusStr[0] = 'F';
			if( FocusFastDisplayFlag)
				FocusStr[1] = 'f';
			else
				FocusStr[1] = 's';
			if( DisplayBiDirOut & DisplayFocusInBit)
				if( ReverseFocusMotor)
					FocusStr[2] = '+';
				else
					FocusStr[2] = '-';
			if( DisplayBiDirOut & DisplayFocusOutBit)
				if( ReverseFocusMotor)
					FocusStr[2] = '-';
				else
					FocusStr[2] = '+';
			FocusStr[3] = EndOfStr;
		}
		else
			strcpy( FocusStr, "   ");

		sprintf( StrBuf, "%s%s%s", FRStr, AuxStr, FocusStr);
		VidMemXY = DisplayXY[DisplayFRFocusAuxAction];
		TextAttr = DisplayText;
		WriteStrBufToScreen_f_ptr();
	}
	DisplayBiDirOut = 0;
}

void ProcessUserHotkeys( void)
{
	struct XY HoldXY;

	WriteWindow( MsgFrame);
	TextAttr = DefaultText;
	_setcursortype( _NOCURSOR);
	HoldXY.X = MsgFrame.Left+2;
	HoldXY.Y = MsgFrame.Top;
	VidMemXY = HoldXY;
	sprintf( StrBuf, "             User selectable Hotkeys");
	WriteStrBufToScreen_f_ptr();
	HoldXY.Y = HoldXY.Y+2;
	VidMemXY = HoldXY;
	sprintf( StrBuf, "Use Cursor Left/Right and Up/Down to select function");
	WriteStrBufToScreen_f_ptr();
	HoldXY.Y++;
	VidMemXY = HoldXY;
	sprintf( StrBuf, "Hotkey highlighted function using F9-F12");
	WriteStrBufToScreen_f_ptr();
	HoldXY.Y++;
	HoldXY.Y++;
	VidMemXY = HoldXY;
	sprintf( StrBuf, "     Press any other key to continue...");
	WriteStrBufToScreen_f_ptr();

	do
	{
	  CheckLX200Events();
	  Response = getch();

	  if( Response == ExtendedKeyboardStroke)
	  {
		 Response = getch();
		 if( Response == LeftCursor || Response == RightCursor ||
			  Response == UpCursor || Response == DownCursor)
		 {
			SetNewMenuCatSub();
			UpdateMenuCatSub();
		 }
	  }
	}while( Response == LeftCursor || Response == RightCursor || Response == UpCursor || Response == DownCursor);

	if( Response == F9)
	{
		HotkeyF9 = MenuArray[CurrentMenuCat].SubTitles[CurrentMenuSub].MenuItem;
		HotkeyF9Set = True;
		RemoveWindow( MsgFrame);
		PressKeyToContMsg( "Hotkey F9 set");
	}
	else
		if( Response == F10)
		{
			HotkeyF10 = MenuArray[CurrentMenuCat].SubTitles[CurrentMenuSub].MenuItem;
			HotkeyF10Set = True;
			RemoveWindow( MsgFrame);
			PressKeyToContMsg( "Hotkey F10 set");
		}
		else
			if( Response == F11)
			{
				HotkeyF11 = MenuArray[CurrentMenuCat].SubTitles[CurrentMenuSub].MenuItem;
				HotkeyF11Set = True;
				RemoveWindow( MsgFrame);
				PressKeyToContMsg( "Hotkey F11 set");
			}
			else
				if( Response == F12)
				{
					HotkeyF12 = MenuArray[CurrentMenuCat].SubTitles[CurrentMenuSub].MenuItem;
					HotkeyF12Set = True;
					RemoveWindow( MsgFrame);
					PressKeyToContMsg( "Hotkey F12 set");
				}
				else
					RemoveWindow( MsgFrame);
}

void ProcessHotkeys( void)
{
	struct XY HoldXY;
	Flag QuitFlag = False;

	NewMenuCat = 0;
	NewMenuSub = 0;
	do
	{
		UpdateMenuCatSub();
		WriteWindow( HotkeyFrame);
		TextAttr = DefaultText;
		switch( NewMenuCat)
		{
			case 0:
				DisplayHotkeyFile();
				break;
			case 1:
				DisplayHotkeyMotor();
				break;
			case 2:
				DisplayHotkeyHandpad();
				break;
			case 3:
				DisplayHotkeyEC();
				break;
			case 4:
				DisplayHotkeyInit();
				break;
			case 5:
				DisplayHotkeyCoord();
				break;
			case 6:
				DisplayHotkeyMove();
				break;
			case 7:
				DisplayHotkeyReset();
				break;
			case 8:
				DisplayHotkeyTest();
		}
		HoldXY.X = HotkeyFrame.Left+1;
		HoldXY.Y = HotkeyFrame.Bottom-4;
		VidMemXY = HoldXY;
		sprintf( StrBuf, "'<' or '>' to select hotkeys for a different menu category, ");
		WriteStrBufToScreen_f_ptr();
		HoldXY.X = HotkeyFrame.Left+1;
		HoldXY.Y = HotkeyFrame.Bottom-3;
		VidMemXY = HoldXY;
		sprintf( StrBuf, "               any other key to quit...");
		WriteStrBufToScreen_f_ptr();
		GetResponseWithLX200Check();
		QuitFlag = True;
		if( Response == ExtendedKeyboardStroke)
		{
			Response = getch();
			if( Response == LeftCursor || Response == RightCursor)
			{
				QuitFlag = False;
				SetNewMenuCatSub();
			}
		}
		RemoveWindow( HotkeyFrame);
	}while( !QuitFlag);
}

Flag EnterHsParms( void)
{
	int X, Y;

	X = MsgFrame.Left + 3;
	Y = MsgFrame.Top + 1;

	gotoxy( X, Y++);
	printf( "HsTimerFlag [%1d] ", HsTimerFlag);
	if( !GetFlag( &HsTimerFlag))
		return False;
	gotoxy( X, Y++);
	printf( "MaxDelay [%d] ", MaxDelay);
	if( !GetInt( &MaxDelay))
		return False;
	gotoxy( X, Y++);
	printf( "MinDelay [%d] ", MinDelay);
	if( !GetInt( &MinDelay))
		return False;
	gotoxy( X, Y++);
	printf( "HsDelayX [%d] ", HsDelayX);
	if( !GetInt( &HsDelayX))
		return False;
	gotoxy( X, Y++);
	printf( "HsRampX [%d] ", HsRampX);
	if( !GetInt( &HsRampX))
		return False;
	gotoxy( X, Y++);
	printf( "InterruptHs [%d] ", InterruptHs);
	if( !GetInt( &InterruptHs))
		return False;
	gotoxy( X, Y++);
	printf( "HoldReps [%d] ", HoldReps);
	if( !GetInt( &HoldReps))
		return False;
	X = MsgFrame.Left + 24;
	Y = MsgFrame.Top + 1;
	gotoxy( X, Y++);
	printf( "HsOverVoltageControl [%d] ", HsOverVoltageControl);
	if( !GetInt( &HsOverVoltageControl))
		return False;
	return True;
}

void ProcessMenuQuit( void)
{
	if( ConfirmQuit)
	{
		WriteWindow( MsgFrame);
		VidMemXY.X = MsgFrame.Left + 5;
		VidMemXY.Y = MsgFrame.Top + 2;
		gotoxy( VidMemXY.X+=5, VidMemXY.Y);
		printf( "Really want to quit (y/n)? ");
		GetResponseWithLX200Check();
		printf( "%c", Response);
		delay( 500);
		if( Response == 'Y' || Response == 'y' || Response == Return)
			QuitFlag = Yes;
		else
			QuitFlag = No;
		RemoveWindow( MsgFrame);
		}
	else
		QuitFlag = Yes;
}

void ProcessMenuSite( void)
{
	double HoldLat, HoldLong;
	int HoldDST;
	double HoldTz;
	long Yr;
	int M;
	int D;
	int h;
	int m;
	double s;
	struct Position Temp;

	HoldLat = LatitudeDeg;
	HoldLong = LongitudeDeg;
	HoldTz = Tz;
	HoldDST = DST;

	WriteWindow( MsgFrame);
	VidMemXY.X = MsgFrame.Left + 2;
	VidMemXY.Y = MsgFrame.Top + 2;
	gotoxy( VidMemXY.X, VidMemXY.Y);
	printf( "latitude (%3.3f)", LatitudeDeg);

	VidMemXY.X = MsgFrame.Left + 2;
	VidMemXY.Y = MsgFrame.Top + 3;
	gotoxy( VidMemXY.X, VidMemXY.Y);
	printf( "longitude (%3.3f)", LongitudeDeg);

	VidMemXY.X = MsgFrame.Left + 2;
	VidMemXY.Y = MsgFrame.Top + 4;
	gotoxy( VidMemXY.X, VidMemXY.Y);
	printf( "timezone (%3.3f)", Tz);

	VidMemXY.X = MsgFrame.Left + 2;
	VidMemXY.Y = MsgFrame.Top + 5;
	gotoxy( VidMemXY.X, VidMemXY.Y);
	printf( "daylight savings time (%d)", DST);

	VidMemXY.X = MsgFrame.Left + 2;
	VidMemXY.Y = MsgFrame.Top + 2;
	gotoxy( VidMemXY.X, VidMemXY.Y);
	printf( "latitude (%3.3f)? ", LatitudeDeg);
	if( GetDouble( &HoldLat)!=UserEscaped)
	{
		VidMemXY.X = MsgFrame.Left + 2;
		VidMemXY.Y = MsgFrame.Top + 3;
		gotoxy( VidMemXY.X, VidMemXY.Y);
		printf( "longitude (%3.3f)? ", LongitudeDeg);
		if( GetDouble( &HoldLong)!=UserEscaped)
		{
			VidMemXY.X = MsgFrame.Left + 2;
			VidMemXY.Y = MsgFrame.Top + 4;
			gotoxy( VidMemXY.X, VidMemXY.Y);
			printf( "timezone (%3.3f)? ", Tz);
			if( GetDouble( &HoldTz)!=UserEscaped)
			{
				VidMemXY.X = MsgFrame.Left + 2;
				VidMemXY.Y = MsgFrame.Top + 5;
				gotoxy( VidMemXY.X, VidMemXY.Y);
				printf( "daylight savings time (%d)? ", DST);
				if( GetInt( &HoldDST)!=UserEscaped)
				{
					LatitudeDeg = HoldLat;
					LongitudeDeg = HoldLong;
					Tz = HoldTz;
					DST = HoldDST;
					getdate( &d);
					Yr = (long) d.da_year;
					M = (int) d.da_mon;
					D = (int) d.da_day;
					gettime( &t);
					h = (int) t.ti_hour;
					m = (int) t.ti_min;
					s = ((double) t.ti_sec) + (t.ti_hund / 100.);
					SetStartBiosClockAndSidTime( Yr, M, D, h, m, s, Tz, DST, LongitudeDeg);
					Current.SidT = SidT;
					if( One.Init)
					{
						Temp = Current;
						Current = One;
						InitMatrix( 1);
						Current = Temp;
					}
				}
			}
		}
	}
	RemoveWindow( MsgFrame);
}

void ProcessMenuDosShell( void)
{
	/* shells to DOS */
	gettext( 1, 1, 80, 25, WinBuffer);
	clrscr();
	_setcursortype( _NORMALCURSOR);
	chdir( DefaultDir);
	SpawnReturn = spawnl( P_WAIT, "c:\\command.com", "", NULL);
	if( SpawnReturn == -1)
		BadExit( "Error from spawnl");
	chdir( DefaultDir);
	_setcursortype( _NOCURSOR);
	clrscr();
	puttext( 1, 1, 80, 25, WinBuffer);
}

void ProcessMenuGuide( void)
{
	if( WriteGuideStartup())
	{
		gettext( 1, 1, 80, 25, WinBuffer);
		clrscr();
		_setcursortype( _NORMALCURSOR);
		chdir( DefaultDir);
		system( "guide.bat");
		chdir( DefaultDir);
		_setcursortype( _NOCURSOR);
		clrscr();
		puttext( 1, 1, 80, 25, WinBuffer);
		ReadGuideStartup();
		/* if tracking on, scope will slew to the new Current Ra and Dec via next call of
		ProcessHPEvents() which calls MoveToCurrentRaDec() */
		if( TrackFlag && Two.Init)
		{
			Current.Ra = In.Ra;
			Current.Dec = In.Dec;
		}
	}
}

void ClearMsgFrameLine( const int Y)
{
	VidMemXY.Y = Y;
	for( VidMemXY.X = MsgFrame.Left+1; VidMemXY.X < MsgFrame.Right-1; VidMemXY.X++)
		WriteCharToScreen( ' ');
}

void ProcessMenuStepsizes( void)
{
	Flag AltRedo, AzRedo;
	Flag AltOk, AzOk;
	double AltSize, AzSize;

	WriteWindow( MsgFrame);
	TextAttr = DefaultText;
	gotoxy( MsgFrame.Left+2, MsgFrame.Top+2);
	printf( "current altitude value is %3.5f arcseconds", AltFullStepSizeArcsec);
	gotoxy( MsgFrame.Left+2, MsgFrame.Top+3);
	printf( "current azimuth value is %3.5f arcseconds", AzFullStepSizeArcsec);
	do
	{
		AltRedo = No;
		AltSize = AltFullStepSizeArcsec;
		ClearMsgFrameLine( MsgFrame.Top+4);
		gotoxy( MsgFrame.Left+2, MsgFrame.Top+5);
		printf( "new altitude motor fullstep size ");
		AltOk = GetDouble( &AltSize);
		if( AltOk)
			if( AltSize <= 0)
			{
				gotoxy( MsgFrame.Left+2, MsgFrame.Top+6);
				printf( "must be > 0 ");
				ContMsgRoutine();
				ClearMsgFrameLine( MsgFrame.Top+5);
				AltRedo = Yes;
			}
	}while( AltRedo);

	if( AltOk)
		do
		{
			AzRedo = No;
			AzSize = AzFullStepSizeArcsec;
			ClearMsgFrameLine( MsgFrame.Top+5);
			gotoxy( MsgFrame.Left+2, MsgFrame.Top+6);
			printf( "new azimuth motor fullstep size ");
			AzOk = GetDouble( &AzSize);
			if( AzOk)
				if( AzSize <= 0)
				{
					gotoxy( MsgFrame.Left+2, MsgFrame.Top+7);
					printf( "must be > 0 ");
					ContMsgRoutine();
					ClearMsgFrameLine( MsgFrame.Top+6);
					AzRedo = Yes;
				}
		}while( AzRedo);

	if( AltOk && AzOk)
	{
		if( AltOk == UserEnteredNumber)
			AltFullStepSizeArcsec = AltSize;
		if( AzOk == UserEnteredNumber)
			AzFullStepSizeArcsec = AzSize;
		CalcVarsRelatingToStepSizes();
		CalcBacklashVars();
		CalcHsMsgSteps();
		SetAccumMsToCurrentAltaz();
		WriteMsArcsecSec();
	}

	RemoveWindow( MsgFrame);
}

void ProcessMenuHandpadDesign( void)
{
	WriteWindow( HandpadFrame);
	TextAttr = DefaultText;
	VidMemXY.X = HandpadFrame.Left + 3;
	VidMemXY.Y = HandpadFrame.Top + 3;
	sprintf( StrBuf, "change from %s to %s (y/n)?", !HandpadDesign?"standard":"guide", HandpadDesign?"standard":"guide");
	WriteStrBufToScreen_f_ptr();
	GetResponseWithLX200Check();
	if( Response == 'y' || Response == 'Y')
		if( HandpadDesign)
			HandpadDesign = 0;
		else
			HandpadDesign = 1;
	RemoveWindow( HandpadFrame);
}

void ProcessMenuHandpad( void)
{
	HandpadFlag++;
	if( HandpadFlag == MaxHandpadFlag)
		HandpadFlag = HandpadOff;
	WriteHandpadStatus();
}

void ProcessMenuSelectHandpadMode( void)
{
	int row, col;

	WriteWindow( HandpadFrame);
	TextAttr = DefaultText;
	row = col = 0;
	for( Ix = 0; Ix < MaxHandpadFlag; Ix++)
	{
		VidMemXY.X = HandpadFrame.Left + 4 + col*28;
		VidMemXY.Y = HandpadFrame.Top + row;
		sprintf( StrBuf, "%c. %s", Ix + 'a', HPStr[Ix]);
		WriteStrBufToScreen_f_ptr();
		row++;
		if( row >= MaxHandpadFlag/2 + MaxHandpadFlag%2)
		{
			col++;
			row = 0;
		}
	}
	Ix--;
	VidMemXY.X = HandpadFrame.Left + 2;
	VidMemXY.Y = HandpadFrame.Top + 1 + MaxHandpadFlag/2 + MaxHandpadFlag%2;
	sprintf( StrBuf, ">>> select a-%c or any other key to escape <<<", Ix+'a');
	WriteStrBufToScreen_f_ptr();
	GetResponseWithLX200Check();
	if( Response >= 'a' && Response <= Ix+'a')
		HandpadFlag = Response - 'a';
	RemoveWindow( HandpadFrame);
	WriteHandpadStatus();
}

void ProcessMenuHandpadLeft( void)
{
	int HoldButtons = Buttons;

	Buttons = LeftKey;
	DisplayButtonsStatus();
	ProcessHPEventsModeSwitch();
	Buttons = HoldButtons;
	KeyboardLeftButton = True;
}

void ProcessMenuHandpadRight( void)
{
	int HoldButtons = Buttons;

	Buttons = RightKey;
	DisplayButtonsStatus();
	ProcessHPEventsModeSwitch();
	Buttons = HoldButtons;
	KeyboardRightButton = True;
}

void ProcessMenuHandpadSpeed( void)
{
	if( MsSpeed == MsKey)
		MsSpeed = Off;
	else
		MsSpeed = MsKey;
	DisplayButtonsStatus();
}

void ProcessMenuHPUpdateDrift( void)
{
	HPUpdateDriftFlag = !HPUpdateDriftFlag;
	WriteHandpadStatus();
}

void ProcessMenuReverseUpDownButtons( void)
{
	UpDownButtonsReversedFlag = !UpDownButtonsReversedFlag;
	if( UpDownButtonsReversedFlag)
		PressKeyToContMsg( "hand paddle up, down buttons are reversed");
	else
		PressKeyToContMsg( "hand paddle up, down buttons back to normal");
}

void ProcessMenuTrack( void)
{
	TrackFlag = !TrackFlag;
	if( TrackFlag)
	{
		if( !One.Init || !Two.Init)
			TrackFlag = No;
	}
	if( !TrackFlag)
		TrackOff();
	WriteTrackStatus();
	/* if tracking being turned off, save current to input so that upon resumption of tracking, scope
	can slew to last tracking position */
	if( !TrackFlag && Two.Init)
	{
		In = Current;
		DisplayIn( "last Track", NameBlanks);
	}
}

void ProcessMenuFRMotorTrack( void)
{
	FRMotorTrackOn = !FRMotorTrackOn;
	if( FRMotorTrackOn)
		if( FocusMethod == FocusMethod_Pulse_1_14 || FocusMethod == FocusMethod_OnOff_16_17_Slow1_14)
		{
			PressKeyToContMsg( "Focus method precludes use of FR motor");
			FRMotorTrackOn = No;
		}
		else
			if( FRStepSize == 0)
			{
				PressKeyToContMsg( "Field rotation motor step size must be > 0");
				FRMotorTrackOn = No;
			}
	WriteFRMotorTrackStatus();
}

void ProcessMenuFRStepsize( void)
{
	double stepsize;

	WriteWindow( MsgFrame);
	VidMemXY.X = MsgFrame.Left + 2;
	VidMemXY.Y = MsgFrame.Top + 3;
	gotoxy( VidMemXY.X, VidMemXY.Y);
	printf( "FR motor stepsize in arcseconds is %3.3f", FRStepSizeArcsec);
	gotoxy( VidMemXY.X, VidMemXY.Y+2);
	printf( "Please enter new value ");
	if( GetDouble( &stepsize))
	{
		FRStepSizeArcsec = stepsize;
		InitFR();
	}
	RemoveWindow( MsgFrame);
}

void ProcessMenuMsSpeed( void)
{
	WriteWindow( MsgFrame);
	VidMemXY.X = MsgFrame.Left + 5;
	VidMemXY.Y = MsgFrame.Top + 2;
	gotoxy( VidMemXY.X+=5, VidMemXY.Y);
	printf( "Please enter new MsArcsecSec,");
	gotoxy( VidMemXY.X+=5, VidMemXY.Y+=2);
	printf( "(max = %5d)", MaxMsSpeed);
	gotoxy( VidMemXY.X+=17, VidMemXY.Y);
	GetInt( &MsArcsecSec);
	if( MsArcsecSec > MaxMsSpeed)
		MsArcsecSec = MaxMsSpeed;
	RemoveWindow( MsgFrame);
	InitMsTickVars( MsArcsecSec);
	WriteMsArcsecSec();
}

void ProcessMenuGuideSpeed( void)
{
	WriteWindow( MsgFrame);
	VidMemXY.X = MsgFrame.Left + 3;
	VidMemXY.Y = MsgFrame.Top + 2;
	gotoxy( VidMemXY.X, VidMemXY.Y);
	printf( "Please enter new GuideArcsecSec");
	gotoxy( VidMemXY.X += 33, VidMemXY.Y);
	GetInt( &GuideArcsecSec);
	RemoveWindow( MsgFrame);
	WriteGuideArcsecSec();
}

void ProcessMenuLX200Control( void)
{
	if( HoldLX200ComPort)
	{
		WriteWindow( MsgFrame);
		gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 2);
		printf( "1. turn off LX200 commands");
		gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 3);
		printf( "2. turn on LX200 commands");
		gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 4);
		printf( "3. toggle LX200 display");
		gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 5);
		printf( "   please select 1, 2, 3, or any other key to abort ");
		GetResponseWithLX200Check();
		switch( Response)
		{
			case '1':
				if( LX200ComPort)
				{
					CloseSerial( LX200ComPort);
					LX200ComPort = 0;
					BlankOutLX200DisplayAreas();
					DisplayLX200CommandStatus();
				}
				else
				{
					gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 6);
					printf( "LX200 already turned off.   ");
					gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 7);
					ContMsgRoutine();
				}
				break;
			case '2':
				if( HoldLX200ComPort && !LX200ComPort)
				{
					LX200ComPort = HoldLX200ComPort;
					InitLX200Input();
					BlankOutLX200DisplayAreas();
					DisplayLX200CommandStatus();
				}
				else
				{
					gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 6);
					printf( "LX200 already turned on.   ");
					gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 7);
					ContMsgRoutine();
				}
				break;
			case '3':
				if( LX200ComPort)
				{
					LX200DisplayFlag = !LX200DisplayFlag;
					if( !LX200DisplayFlag)
						BlankOutLX200DisplayAreas();
				}
				else
				{
					gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 6);
					printf( "LX200 turned off.   ");
					gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 7);
					ContMsgRoutine();
				}
		}
		RemoveWindow( MsgFrame);
	}
	else
		PressKeyToContMsg( "no LX200 comm port defined");
}

void BlankOutLX200DisplayAreas( void)
{
	/* blank out display areas */
	for( Ix = 0; Ix < LX200_Cmd_Array_Size * (LX200_CMD_STR_SIZE + 1) + 3; Ix++)
		sprintf( StrBuf+Ix, "%c", Blank);
	VidMemXY = DisplayXY[DisplayLX200];
	TextAttr = DisplayText;
	WriteStrBufToScreen_f_ptr();

	for( Ix = 0; Ix < LX200QueueSize; Ix++)
		sprintf( StrBuf+Ix, "%c", Blank);
	VidMemXY = DisplayXY[DisplayLX200_2];
	TextAttr = DisplayText;
	WriteStrBufToScreen_f_ptr();
}

void DisplayLX200CommandStatus( void)
{
	if( LX200ComPort)
		sprintf( StrBuf, "LX200 commands on");
	else
		sprintf( StrBuf, "LX200 commands off");
	VidMemXY = DisplayXY[DisplayLX200];
	TextAttr = DisplayText;
	WriteStrBufToScreen_f_ptr();
}

void ProcessMenuTestSerial( void)
{
	int comport;

	WriteWindow( MsgFrame);
	gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 2);
	printf( "please select comm port 1, 2, 3, or 4 to test: ");
	if( GetInt( &comport))
	{
		if( comport >= 1 && comport <= 4)
		{
			RemoveWindow( MsgFrame);
			gettext( 1, 1, 80, 25, WinBuffer);
			clrscr();
			_setcursortype( _NORMALCURSOR);
			TestSerial( comport);
			_setcursortype( _NOCURSOR);
			puttext( 1, 1, 80, 25, WinBuffer);
		}
		else
		{
			gotoxy( MsgFrame.Left + 3, MsgFrame.Top + 4);
			printf( "comm port must be 1 or 2");
			gotoxy( MsgFrame.Left + 3, MsgFrame.Top + 6);
			ContMsgRoutine();
			RemoveWindow( MsgFrame);
		}
	}
	else
		RemoveWindow( MsgFrame);
}

void ProcessMenuTestPPort( void)
{
	gettext( 1, 1, 80, 25, WinBuffer);
	clrscr();
	_setcursortype( _NORMALCURSOR);
	TestPPort();
	_setcursortype( _NOCURSOR);
	puttext( 1, 1, 80, 25, WinBuffer);
}

void Start2MotorTrackWithDefaultValues( void)
{
	TrackByRateFlag = Yes;
	TrackMsPerSec.A = TrackMsPerSec.Z = 0;
	TrackMsAccel.A = TrackMsAccel.Z = 0;
	MsZeroSoundOn = No;
	TrackDir.A = TrackDir.Z = CW;
	TrackHoldDir.A = TrackDir.A;
	TrackHoldDir.Z = TrackDir.Z;
	TrackMsTick.A = TrackMsTick.Z = 0;
	TrackHoldMsTick.A = TrackHoldMsTick.Z = 0;
	TrackAccumMsAccel.A = TrackAccumMsAccel.Z = 0;
	IncrTrackStep = 0;
	WriteTrackByRate();
}

void ProcessMenu2MotorTrack( void)
{
	int MaxMsPerSec = PWMRepsTick * MsPerHs * ClockTicksSec;

	TrackByRateFlag = No;

	WriteWindow( MsgFrame);
	VidMemXY.X = MsgFrame.Left + 1;
	VidMemXY.Y = MsgFrame.Top + 2;
	gotoxy( VidMemXY.X, VidMemXY.Y);
	printf( "Enter 'A' motor microsteps/sec (max=%d): ", MaxMsPerSec);
	if( GetDouble( &TrackMsPerSec.A))
	{
		VidMemXY.X = MsgFrame.Left + 1;
		VidMemXY.Y = MsgFrame.Top + 3;
		gotoxy( VidMemXY.X, VidMemXY.Y);
		printf( "Enter 'Z' motor microsteps/sec (max=%d): ", MaxMsPerSec);
		if( GetDouble( &TrackMsPerSec.Z))
		{
			VidMemXY.X = MsgFrame.Left + 1;
			VidMemXY.Y = MsgFrame.Top + 4;
			gotoxy( VidMemXY.X, VidMemXY.Y);
			printf( "Enter 'A' motor accel (Ms/tick): ");
			if( GetDouble( &TrackMsAccel.A))
			{
				VidMemXY.X = MsgFrame.Left + 1;
				VidMemXY.Y = MsgFrame.Top + 5;
				gotoxy( VidMemXY.X, VidMemXY.Y);
				printf( "Enter 'Z' motor accel (Ms/tick): ");
				if( GetDouble( &TrackMsAccel.Z))
				{
					VidMemXY.X = MsgFrame.Left + 1;
					VidMemXY.Y = MsgFrame.Top + 6;
					gotoxy( VidMemXY.X, VidMemXY.Y);
					printf( "Make sound when microstep number = 0 (y/n)? ");
					GetResponseWithLX200Check();
					if( Response == 'Y' || Response == 'y' || Response == Return || Response == 'N' || Response == 'n')
					{
						if( Response == 'Y' || Response == 'y' || Response == Return)
							MsZeroSoundOn = Yes;

						TrackByRateFlag = Yes;

						if( TrackMsPerSec.A < 0)
						{
							TrackMsPerSec.A = -TrackMsPerSec.A;
							TrackDir.A = CCW;
						}
						else
							TrackDir.A = CW;
						TrackHoldDir.A = TrackDir.A;
						TrackMsTick.A = TrackMsPerSec.A / ClockTicksSec;

						if( TrackMsPerSec.Z < 0)
						{
							TrackMsPerSec.Z = -TrackMsPerSec.Z;
							TrackDir.Z = CCW;
						}
						else
							TrackDir.Z = CW;
						TrackHoldDir.Z = TrackDir.Z;
						TrackMsTick.Z = TrackMsPerSec.Z / ClockTicksSec;

						TrackHoldMsTick.A = TrackHoldMsTick.Z = 0;
						TrackAccumMsAccel.A = TrackAccumMsAccel.Z = 0;
						IncrTrackStep = 0;
					}
				}
			}
		}
	}
	RemoveWindow( MsgFrame);

	if( TrackByRateFlag)
		WriteTrackByRate();
}

void ProcessMenuScreen( void)
{
	WriteWindow( MsgFrame);
	gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 2);
	printf( "(a) standard display");
	gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 3);
	printf( "(b) jumbo display");
	gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 4);
	printf( "(c) no updating");
	gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 5);
	printf( "   select: ");
	GetResponseWithLX200Check();
	RemoveWindow( MsgFrame);
	if( Response == 'a' || Response == 'A')
		WriteStrBufToScreen_f_ptr = WriteStrBufToScreen;
	if( Response == 'b' || Response == 'B')
	{
		WriteStrBufToScreen_f_ptr = NULL_FUNCTION;
		JumboDisplayFlag = Yes;
		gettext( 1, 1, MaxX, MaxY, WinBuffer);
		Clrscr();
		DisplayJumboRa( &Current);
		DisplayJumboDec( &Current);
	}
	if( Response == 'c' || Response == 'C')
		WriteStrBufToScreen_f_ptr = NULL_FUNCTION;
}

void ProcessMenuChangeHsParms( void)
{
	WriteWindow( MsgFrame);
	EnterHsParms();
	RemoveWindow( MsgFrame);
	FreeHsArrays();
	CreateHsArrays();
	InitHsArrays();
}

void ProcessMenuChangeMsParms( void)
{
	ClearMenuArea();
	MsParmsXY.X = MenuStartX;
	MsParmsXY.Y = MenuCatsY;
	ResponseMsParmIx = NewResponseMsParmIx = 0;
	DisplayMsParmsTitlesAndValues();
	SetMsParmVidMemXY( ResponseMsParmIx);
	UpdateResponseMsParm();
	MsParmsAllowedFlag = True;

	if( !TrackByRateFlag && !TrackFlag)
		Start2MotorTrackWithDefaultValues();
}

void ProcessMenuAutoMsParms( void)
{
	double HoldPWMRepsTick = 30;
	double MotorCurrentPercent = 50;
	double ActualTotalPPortWrites, DesiredTotalPPortWrites;
	int iters = 0;

	WriteWindow( MsgFrame);
	gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 2);
	printf( "enter desired PWMRepsTick ");
	if( GetDouble( &HoldPWMRepsTick))
	{
		gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 3);
		printf( "enter desired motor current percent (1-100%) ");
		if( GetDouble( &MotorCurrentPercent))
		{
			gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 4);
			printf( "press any key to stop");

			while( True)
			{
				/* re-calculate every SizeofPWMRepsTickArray: let average PWM reps tick	settle */
				if( !iters++)
				{
					ActualTotalPPortWrites = (double) MaxPWM * MsDelayX + MsPause;
					DesiredTotalPPortWrites = ActualTotalPPortWrites *
						(double) AvgPWMRepsTick / HoldPWMRepsTick;
					MsPause = (int) (DesiredTotalPPortWrites * (1-MotorCurrentPercent/100));
					MsDelayX = (int) ((DesiredTotalPPortWrites - MsPause) / MaxPWM);
					if( MsDelayX < 1)
						MsDelayX = 1;
					CalcVarsRelatingToMs();
					FreeMsArrays();
					CreateMsArrays();
					InitMsArrays();
				}
				if( iters > SizeofPWMRepsTickArray)
					iters = 0;

				if( KeyStroke)
					break;
				VidMemXY.X = MsgFrame.Left + 8;
				VidMemXY.Y = MsgFrame.Top + 5;
				sprintf( StrBuf, "MsDelayX = %3d, MsPause = %4d", MsDelayX, MsPause);
				WriteStrBufToScreen_f_ptr();

				Steps.A = Steps.Z = 0;
				MoveMs_f_ptr();
			}
			getch();
		}
	}
	RemoveWindow( MsgFrame);
}

void ProcessMenuChangeBacklashParms( void)
{
	WriteWindow( MsgFrame);
	gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 1);
	printf( "AltBacklashArcmin currently is %3.2f", AltBacklashArcmin);
	gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 2);
	printf( "enter new value ");
	if( GetDouble( &AltBacklashArcmin))
	{
		gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 3);
		printf( "AzBacklashArcmin currently is %3.2f", AzBacklashArcmin);
		gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 4);
		printf( "enter new value ");
		if( GetDouble( &AzBacklashArcmin))
			CalcBacklashVars();
	}
	RemoveWindow( MsgFrame);
}

void ProcessMenuZeroBacklash( void)
{
	AltBacklashArcmin = AzBacklashArcmin = 0;
	CalcBacklashVars();
	PressKeyToContMsg( "backlash zeroed out");
}

void ProcessMenuAutoGEMFlipOnOff( void)
{
	WriteWindow( MsgFrame);

	VidMemXY.X = MsgFrame.Left + 3;
	VidMemXY.Y = MsgFrame.Top + 3;
	gotoxy( VidMemXY.X, VidMemXY.Y);
	printf( "AutoGEMFlip is %s. Turn it %s (y/n)? ", AutoGEMFlip? "on":"off", AutoGEMFlip? "off":"on");
	GetResponseWithLX200Check();
	printf( "%c", Response);
	delay( 250);
	if( Response == 'Y' || Response == 'y' || Response == Return)
		AutoGEMFlip = !AutoGEMFlip;
	RemoveWindow( MsgFrame);
}

void ProcessMenuGuideFRAngle( void)
{
	double GuideFRAngleDeg;

	WriteWindow( MsgFrame);
	VidMemXY.X = MsgFrame.Left + 3;
	VidMemXY.Y = MsgFrame.Top + 2;
	gotoxy( VidMemXY.X, VidMemXY.Y);
	printf( "Please enter new handpad guide angle");
	gotoxy( VidMemXY.X += 41, VidMemXY.Y);
	if( GetDouble( &GuideFRAngleDeg))
	{
		GuideFRAngle = GuideFRAngleDeg * DegToRad;
		GuideFRAngleOffset = FieldR - GuideFRAngle;
	}
	RemoveWindow( MsgFrame);
}

void ProcessMenuPECOnOff( void)
{
	PECFlag = !PECFlag;
	if( PECFlag)
		TurnPECOn();
	else
		TurnPECOff();
}

void TurnPECOn( void)
{
	if( AutoAltPECSyncOnFlag)
	{
		/* to prevent auto synch of PEC while in middle of microswitch range - PEC should only be
		auto sync'd at beginning of microswitch range when moving in CW direction */
		SetAutoAltPECSynchSignal();
		if( AutoAltPECSyncLowHighFlag)
			AutoAltPECSyncSignal = !AutoAltPECSyncSignal;
		LastAutoAltPECSyncSignal = AutoAltPECSyncSignal;
	}
	else
		SetPECIxOffsetA();
	if( AutoAzPECSyncOnFlag)
	{
		/* to prevent auto synch of PEC while in middle of microswitch range - PEC should only be
		auto sync'd at beginning of microswitch range when moving in CW direction */
		SetAutoAzPECSynchSignal();
		if( AutoAzPECSyncLowHighFlag)
			AutoAzPECSyncSignal = !AutoAzPECSyncSignal;
		LastAutoAzPECSyncSignal = AutoAzPECSyncSignal;
	}
	else
		SetPECIxOffsetZ();
	GetPECIx();
	WritePECSynchStatus();
}

void TurnPECOff( void)
{
	PECFlag = No;
	GetPECIx();
	WritePECSynchStatus();
}

void ProcessMenuPECSynchAlt( void)
{
	if( PECFlag)
	{
		LastPECIxOffset.A = PECIxOffset.A;
		SetPECIxOffsetA();
		GetPECIx();
		DisplayAltPEC();
	}
}

void ProcessMenuPECSynchAz( void)
{
	if( PECFlag)
	{
		LastPECIxOffset.Z = PECIxOffset.Z;
		SetPECIxOffsetZ();
		GetPECIx();
		DisplayAzPEC();
	}
}

void ProcessMenuPECMedianSmooth( void)
{
	int Num;

	WriteWindow( MsgFrame);
	VidMemXY.X = MsgFrame.Left + 2;
	VidMemXY.Y = MsgFrame.Top + 2;
	gotoxy( VidMemXY.X, VidMemXY.Y);
	printf( "Number of values to use for median smoothing ");
	gotoxy( VidMemXY.X+=45, VidMemXY.Y);
	if( GetInt( &Num))
		MedianSmoothPEC( Num);
	RemoveWindow( MsgFrame);
}

void ProcessMenuInputEquat( void)
{
	struct XY HoldXY;

	WriteWindow( MsgFrame);
	HoldXY.X = MsgFrame.Left + 5;
	HoldXY.Y = MsgFrame.Top + 2;
	gotoxy( HoldXY.X, HoldXY.Y);
	printf( "Please enter Input ");
	HoldXY.X = wherex();
	gotoxy( HoldXY.X, HoldXY.Y);
	InputEquat( &In, HoldXY.X, HoldXY.Y);
	RemoveWindow( MsgFrame);
	DisplayIn( "input equat", NameBlanks);
}

void ProcessMenuInputEquatToAltaz( void)
{
	struct Position Temp = Current;

	if( Two.Init)
	{
		Current.Ra = In.Ra;
		Current.Dec = In.Dec;
		GetAltaz();
		In.Alt = Current.Alt;
		In.Az = Current.Az;
		DisplayIn( "input eq->alt", NameBlanks);
		Current = Temp;
	}
	else
		PressKeyToContMsg( "Must have done at least 2 inits");
}

void ProcessMenuOffsetEquat( void)
{
	struct XY HoldXY;

	WriteWindow( MsgFrame);
	HoldXY.X = MsgFrame.Left + 3;
	HoldXY.Y = MsgFrame.Top + 2;
	gotoxy( HoldXY.X, HoldXY.Y);
	printf( "Enter Offset from Current ");
	HoldXY.X = wherex();
	gotoxy( HoldXY.X, HoldXY.Y);
	InputEquat( &Offset, HoldXY.X, HoldXY.Y);
	RemoveWindow( MsgFrame);
	In.Ra = Current.Ra + Offset.Ra;
	In.Dec = Current.Dec + Offset.Dec;
	ValidRa( &In);
	GetHMSH( In.Ra*RadToHundSec + .5, &In.RaHMSH);
	GetDMS( In.Dec*RadToArcsec + .5, &In.DecDMS);
	DisplayIn( "offset equat", NameBlanks);
}

void ProcessMenuDriftEquat( void)
{
	struct XY HoldXY;

	WriteWindow( MsgFrame);
	HoldXY.X = MsgFrame.Left + 3;
	HoldXY.Y = MsgFrame.Top + 2;
	gotoxy( HoldXY.X, HoldXY.Y);
	printf( "Please enter Drift /hr ");
	HoldXY.X = wherex();
	gotoxy( HoldXY.X, HoldXY.Y);
	Drift.RaHMSH.HundSec = 0;
	Drift.RaHMSH.Sign = Plus;
	InputEquat( &Drift, HoldXY.X, HoldXY.Y);
	/*Drift entered in degrees converted to radians by InputAltaz */
	RemoveWindow( MsgFrame);
	CalcRadFromHMSH( &Drift.Ra, Drift.RaHMSH);
	CalcRadFromDMS( &Drift.Dec, Drift.DecDMS);
	CalcCommonDriftVars();
	DisplayDrift();
}

void ProcessMenuInputAltaz( void)
{
	struct XY HoldXY;

	WriteWindow( MsgFrame);
	HoldXY.X = MsgFrame.Left + 5;
	HoldXY.Y = MsgFrame.Top + 2;
	gotoxy( HoldXY.X, HoldXY.Y);
	printf( "Please enter Input ");
	HoldXY.X = wherex();
	gotoxy( HoldXY.X, HoldXY.Y);
	InputAltaz( &In, HoldXY.X, HoldXY.Y);
	RemoveWindow( MsgFrame);
	DisplayIn( "input altaz", NameBlanks);
}

void ProcessMenuOffsetAltaz( void)
{
	struct XY HoldXY;

	WriteWindow( MsgFrame);
	HoldXY.X = MsgFrame.Left + 3;
	HoldXY.Y = MsgFrame.Top + 2;
	gotoxy( HoldXY.X, HoldXY.Y);
	printf( "Enter Offset from Current ");
	HoldXY.X = wherex();
	gotoxy( HoldXY.X, HoldXY.Y);
	InputAltaz( &Offset, HoldXY.X, HoldXY.Y);
	RemoveWindow( MsgFrame);
	In.Alt = Current.Alt + Offset.Alt;
	In.Az = Current.Az + Offset.Az;
	DisplayIn( "offset altaz", NameBlanks);
}

void ProcessMenuDriftAltaz( void)
{
	struct XY HoldXY;

	WriteWindow( MsgFrame);
	HoldXY.X = MsgFrame.Left + 3;
	HoldXY.Y = MsgFrame.Top + 2;
	gotoxy( HoldXY.X, HoldXY.Y);
	printf( "Please enter Drift \"/min ");
	HoldXY.X = wherex();
	gotoxy( HoldXY.X, HoldXY.Y);
	InputAltaz( &Drift, HoldXY.X, HoldXY.Y);
	/*Drift entered in degrees converted to radians by InputAltaz */
	Drift.Alt /= 3600.;
	Drift.Az /= 3600.;
	RemoveWindow( MsgFrame);
	/* "/min */
	CalcCommonDriftVars();
	DisplayDrift();
}

void ProcessMenuZeroDrift( void)
{
	Drift.RaHMSH.Sign = Drift.DecDMS.Sign = Plus;
	Drift.RaHMSH.Hr = Drift.RaHMSH.Min = Drift.RaHMSH.Sec = Drift.RaHMSH.HundSec = 0;
	Drift.DecDMS.Deg = Drift.DecDMS.Min = Drift.DecDMS.Sec = 0;
	CalcRadFromHMSH( &Drift.Ra, Drift.RaHMSH);
	CalcRadFromDMS( &Drift.Dec, Drift.DecDMS);
   CometDrift.Ra = CometDrift.Dec = 0.0;
	Drift.Alt = 0;
	Drift.Az = 0;
	CalcCommonDriftVars();
	DisplayDrift();
}

void ProcessMenuResetDrift( void)
{
  RemoveCometDriftFromDrift();
  DisplayDrift();
}

void ProcessMenuResetAccumDrift( void)
{
	AccumDrift.A = AccumDrift.Z = 0;
}

void ProcessMenuMoveEquat( void)
{
	/* scope will slew or track to the new Current Ra and Dec next time through function via
	ProcessHPEvents() then MoveToCurrentRaDec() */
	if( Two.Init)
	{
		if( !TrackFlag)
		{
			TrackFlag = Yes;
			WriteTrackStatus();
		}
		Current.Ra = In.Ra;
		Current.Dec = In.Dec;
		SlewBeep = SlewReady;
	}
}

void ProcessMenuMoveAltaz( void)
{
	/* Delta = new - old */
	Delta.A = In.Alt - Current.Alt;
	Delta.Z = In.Az - Current.Az;
	SetDirDistanceStepsThenMove();
	PauseUntilNewSidTime();
	HPEventGetEquat();
}

void ProcessMenuMoveHome( void)
{
	/* Delta = new - old */
	Delta.A = HomeAltDeg*DegToRad - Current.Alt;
	Delta.Z = HomeAzDeg*DegToRad - Current.Az;
	SetDirDistanceStepsThenMove();
	PauseUntilNewSidTime();
	DetectGEMflippedFromCurrentAltaz();
	HPEventGetEquat();
}

void ProcessMenuResetEquat( void)
{
	if( Two.Init)
	{
		/* set current equat to input equat */
		Current.Ra = In.Ra;
		Current.Dec = In.Dec;
		/* get current altaz */
		GetAltaz();
		CheckSiderostatAltaz();
		/* set scope to current altaz */
		SetAccumMsToCurrentAltaz();
		/* get new current equat */
		HPEventGetEquat();
		/* redo encoder vars */
		SetEncoderAZandEncoderOffset();
	}
	else
   	PressKeyToContMsg( "must initialize positions #1 and #2 first");
}

void ProcessMenuResetAltaz( void)
{
	Current.Alt = In.Alt;
	Current.Az = In.Az;
	SetAccumMsToCurrentAltaz();
	HPEventGetEquat();
	SetEncoderAZandEncoderOffset();
}

void ProcessMenuResetHome( void)
{
	Current.Alt = HomeAltDeg*DegToRad;
	Current.Az = HomeAzDeg*DegToRad;
	DetectGEMflippedFromCurrentAltaz();
	SetAccumMsToCurrentAltaz();
	HPEventGetEquat();
	SetEncoderAZandEncoderOffset();
}

void ProcessMenuGEMFlipPossible( void)
{
	GEMFlipPossible = !GEMFlipPossible;
	DisplayGEMFlipStatus();
}

/* flip a german equatorial mount across the meridian:
the Ra tracking movement will continue to move in the same direction but the Dec movement will be
reversed; this is handled in low level GetAltaz() and GetEquat() routines so as to not disturb
Current.Alt and Current.Az values */

void DisplayGEMFlipStatus( void)
{
	sprintf( StrBuf, "%s", GEMFlipPossible? GEMflippedFlag? "On  W->E": "Off E->W" : "N/A     ");
	VidMemXY = DisplayXY[DisplayGEMFlip];
	TextAttr = DisplayText;
	WriteStrBufToScreen_f_ptr();
}

void ProcessMenuGEMMeridianFlip( void)
{
	struct XY HoldXY;

	if( TrackFlag)
	{
		TrackFlag = No;
		WriteTrackStatus();
	}
	GEMflippedFlag = !GEMflippedFlag;

	/* if called from ProcessMenuResponse() with keypress of [ or { */
	if (Response == '[' || Response == '{')
		ProcessMenuResetEquat();
	else
	{
		WriteWindow( MsgFrame);
		HoldXY.X = MsgFrame.Left + 3;
		HoldXY.Y = MsgFrame.Top + 2;
		gotoxy( HoldXY.X, HoldXY.Y++);
		printf( "Tracking has been turned off.");
		gotoxy( HoldXY.X, HoldXY.Y++);
		printf( "The scope has been flipped across the meridian.");
		gotoxy( HoldXY.X, HoldXY.Y++);
		printf( "Reset to equatorial coordinates (y/n)? ");
		GetResponseWithLX200Check();
		printf( "%c", Response);
		if( Response == 'Y' || Response == 'y')
			ProcessMenuResetEquat();
		RemoveWindow( MsgFrame);
	}
}

void ProcessMenuFRReset( void)
{
	if( FRStepSize > 0 && Two.Init)
	{
		CalcFieldR();
		FRMotorAngle = FieldR;
	}
}

void ProcessMenuHomeCoord( void)
{
	struct Position HoldDeg;

	WriteWindow( MsgFrame);

	HoldDeg.Alt = HomeAltDeg;
	HoldDeg.Az = HomeAzDeg;
	gotoxy( MsgFrame.Left + 3, MsgFrame.Top + 2);
	printf( "Home Altitude Degrees ");
	if( !GetDouble( &HomeAltDeg))
		HomeAltDeg = HoldDeg.Alt;
	else
	{
		gotoxy( MsgFrame.Left + 3, MsgFrame.Top + 4);
		printf( "Home Azimuth Degrees ");
		if( !GetDouble( &HomeAzDeg))
		{
			HomeAzDeg = HoldDeg.Az;
			HomeAltDeg = HoldDeg.Alt;
		}
	}
	RemoveWindow( MsgFrame);
}

Flag ProcessMenuCheckAnalysis( void)
{
	if( !Two.Init)
	{
		PressKeyToContMsg( "Must have done at least 2 inits");
		return False;
	}
	if( !LoadAnalysisFileIntoMemory())
	{
		PressKeyToContMsg( "could not find analysis file");
		return False;
	}
	if( !LinkPosCount)
	{
		PressKeyToContMsg( "Empty analysis file: nothing to graph");
		return False;
	}
	return True;
}

void ProcessMenuSelection( void)
{
	Flag HoldDisplayOpeningMsgs;

	ConsecutiveSlews = 0;
	AlignMs_f_ptr();
	SetCurrentAltazToAccumMs();
	switch( Response)
	{
		case MenuQuit:
			ProcessMenuQuit();
			break;
		case MenuSite:
			ProcessMenuSite();
			break;
		case MenuSaveConfigFile:
			WriteConfig();
			break;
		case MenuDataFile:
			MenuCoordFile();
			break;
		case MenuDataFileClosest:
			if( ProcessMenuDataFileClosest( FilenameNotSet, ExactInputFieldsMatchOK))
				;
			else
				PressKeyToContMsg( "Could not find match");
			break;
		case MenuDataFileClosestNotSame:
			if( ProcessMenuDataFileClosest( FilenameNotSet, AvoidMatchInputFields))
				;
			else
				PressKeyToContMsg( "Could not find match");
			break;
		case MenuDataFileClosestNotInputFile:
			if( ProcessMenuDataFileClosest( FilenameNotSet, DoNotMatchInputFile))
				;
			else
				PressKeyToContMsg( "Could not find match");
			break;
		case MenuSearchDataFiles:
			ProcessMenuSearch();
			break;
		case MenuGrandTourClosest:
			if( Two.Init)
				GetGrandTourRecClosestCurrentEquat();
			else
				PressKeyToContMsg( "Must enter at least 2 inits");
			break;
		case MenuScrollTour:
			chdir( DataDir);
			if( SelectDataFilename( ScrollFile))
			{
				strcpy( ScrollFilename, Filename);
				LoadScrollFileFromFile();
			}
			GetCurDir( DataDir);
			chdir( DefaultDir);
			break;
		case MenuMinorPlanets:
			SelectMinorPlanet();
			break;
		case MenuWriteInputComment:
			ProcessMenuWriteInputComment();
			break;
		case MenuDosShell:
			ProcessMenuDosShell();
			DisplayMouse();
			break;
		case MenuGuide:
			if( UseMouseFlag)
				CloseMouseControl();
			ProcessMenuGuide();
			if( UseMouseFlag)
				InitMouseControl();
			break;
		case MenuUserDefinedHotkeys:
			ProcessUserHotkeys();
			break;
		case MenuHotkeys:
			ProcessHotkeys();
			break;
		case MenuStepsizes:
			ProcessMenuStepsizes();
			break;
		case MenuHandpadDesign:
			ProcessMenuHandpadDesign();
			break;
		case MenuHandpad:
			ProcessMenuHandpad();
			break;
		case MenuSelectHandpadMode:
			ProcessMenuSelectHandpadMode();
			break;
		case MenuHandpadLeft:
			ProcessMenuHandpadLeft();
			break;
		case MenuHandpadRight:
			ProcessMenuHandpadRight();
			break;
		case MenuHandpadSpeed:
			ProcessMenuHandpadSpeed();
			break;
		case MenuHPUpdateDrift:
			ProcessMenuHPUpdateDrift();
			break;
		case MenuReverseUpDownButtons:
			ProcessMenuReverseUpDownButtons();
			break;
		case MenuTrack:
			if( TrackByRateFlag)
			{
				TrackByRateFlag = No;
				MsZeroSoundOn = No;
				nosound();
				WriteTrackByRate();
			}
			else
				ProcessMenuTrack();
			break;
		case MenuFRMotorTrack:
			ProcessMenuFRMotorTrack();
			break;
		case MenuFRStepsize:
			ProcessMenuFRStepsize();
			break;
		case MenuMsSpeed:
			ProcessMenuMsSpeed();
			break;
		case MenuGuideSpeed:
			ProcessMenuGuideSpeed();
			break;
		case MenuChangeGuideFRAngle:
			ProcessMenuGuideFRAngle();
			break;
		case MenuReadSlew:
			InputEquatSlewDat();
			ProcessHPEvents();
			break;
		case MenuWriteSlew:
			WriteEquatSlewOutFile();
			ProcessHPEvents();
			break;
		case MenuDecMotor:
			ProcessMenuDecMotor();
			break;
		case MenuReverseAZMotors:
			ProcessMenuReverseAZMotors();
			break;
		case MenuReverseFRMotor:
			ProcessMenuReverseFRMotor();
			break;
		case MenuReverseFocusMotor:
			ProcessMenuReverseFocusMotor();
			break;
		case MenuChangeHsParms:
			ProcessMenuChangeHsParms();
			break;
		case MenuChangeMsParms:
			ProcessMenuChangeMsParms();
			break;
		case MenuAutoMsParms:
			ProcessMenuAutoMsParms();
			break;
		case MenuChangeBacklashParms:
			ProcessMenuChangeBacklashParms();
			break;
		case MenuZeroBacklash:
			ProcessMenuZeroBacklash();
			break;
		case MenuAutoGEMFlipOnOff:
			if( GEMFlipPossible)
				ProcessMenuAutoGEMFlipOnOff();
			else
				PressKeyToContMsg( "GEMFlip not possible according to configuration");
			break;
		case MenuLX200Control:
			ProcessMenuLX200Control();
			break;
		case MenuTestSerial:
			ProcessMenuTestSerial();
			break;
		case MenuTestPPort:
			ProcessMenuTestPPort();
			break;
		case Menu2MotorTrack:
			ProcessMenu2MotorTrack();
			break;
		case MenuTrackEncoder:
			TrackEncoder();
			break;
		case MenuScreen:
			ProcessMenuScreen();
			break;
		case MenuPECOnOff:
			ProcessMenuPECOnOff();
			break;
		case MenuPECSynchAlt:
			ProcessMenuPECSynchAlt();
			break;
		case MenuPECSynchAz:
			ProcessMenuPECSynchAz();
			break;
		case MenuPECReload:
			HoldDisplayOpeningMsgs = DisplayOpeningMsgs;
			DisplayOpeningMsgs = False;
			LoadPEC();
			DisplayOpeningMsgs = HoldDisplayOpeningMsgs;
			break;
		case MenuPECDisplay:
			DisplayPECgraphically();
			break;
		case MenuPECAverageFiles:
			if( !AveragePECAnalysisFiles( altaxis))
				PressKeyToContMsg( "no pecalt**.txt files found");
			if( !AveragePECAnalysisFiles( azaxis))
				PressKeyToContMsg( "no pecaz**.txt files found");
			break;
		case MenuPECMedianSmooth:
			ProcessMenuPECMedianSmooth();
			break;
		case MenuPECGuidingAltDisplay:
			AnalyzeGuideAltArray();
			break;
		case MenuPECGuidingAzDisplay:
			AnalyzeGuideAzArray();
			break;
		case MenuPECZeroOut:
			ZeroAltPEC();
			ZeroAzPEC();
			PressKeyToContMsg( "Zeroed out Altitude and Azimuth PEC");
			break;
		case MenuDisplayInit:
			DisplayInit();
			break;
		case MenuKillInits:
			KillInits();
			break;
		case MenuEnterInit1:
			EnterInit( 1);
			break;
		case MenuEnterInit2:
			if( One.Init)
				EnterInit( 2);
			else
				PressKeyToContMsg( "Must enter init 1 first");
			break;
		case MenuEnterInit3:
			if( Two.Init)
				EnterInit( 3);
			else
				PressKeyToContMsg( "Must enter init 2 first");
			break;
		case MenuClosestInit:
			if( Two.Init)
				ClosestInit();
			else
				PressKeyToContMsg( "Must enter at least 2 inits");
			break;
		case MenuInit1:
			Current.Ra = In.Ra;
			Current.Dec = In.Dec;
			strcpy( WhyInit, WHY_INIT_KEYBOARD);
			KBEventInitMatrix( 1);
			break;
		case MenuInit2:
			if( One.Init)
			{
				Current.Ra = In.Ra;
				Current.Dec = In.Dec;
				strcpy( WhyInit, WHY_INIT_KEYBOARD);
				KBEventInitMatrix( 2);
			}
			else
				PressKeyToContMsg( "Must already have completed init 1");
			break;
		case MenuInit3:
			if( Two.Init)
			{
				Current.Ra = In.Ra;
				Current.Dec = In.Dec;
				strcpy( WhyInit, WHY_INIT_KEYBOARD);
				KBEventInitMatrix( 3);
			}
			else
				PressKeyToContMsg( "Must already have completed init 2");
			break;
		case MenuSetZ1Z2Z3:
			ProcessMenuSetZ1Z2Z3();
			break;
		case MenuAltOffset:
			ProcessAltOffset();
			break;
		case MenuAzOffsetAxisAlign:
			ProcessAzOffsetAxisAlign();
			break;
		case MenuGraphAnalysis:
			if( ProcessMenuCheckAnalysis())
				ProcessMenuGraphAnalysis();
			break;
		case MenuPurgeAnalysis:
			ProcessMenuPurgeAnalysis();
			break;
		case MenuAnalysisBestZ1Z2Z3:
			ProcessMenuAnalysisBestZ1Z2Z3();
			break;
		case MenuGraphZ1Z2Z3:
			if( ProcessMenuCheckAnalysis())
				ProcessMenuGraphZ1Z2Z3();
			break;
		case MenuPolarAlign:
			ProcessPolarAlign0();
			break;
		case MenuAltAzEC:
			if( ProcessMenuCheckAnalysis())
				ProcessMenuAltAzEC();
			break;
		case MenuAltAzECOnOff:
			ProcessMenuAltAzECOnOff();
			break;
		case MenuAltAltEC:
			if( ProcessMenuCheckAnalysis())
				ProcessMenuAltAltEC();
			break;
		case MenuAltAltECOnOff:
			ProcessMenuAltAltECOnOff();
			break;
		case MenuAzAzEC:
			if( ProcessMenuCheckAnalysis())
				ProcessMenuAzAzEC();
			break;
		case MenuAzAzECOnOff:
			ProcessMenuAzAzECOnOff();
			break;
		case MenuAnalysisToPMC:
			if( ProcessMenuCheckAnalysis())
				ProcessMenuAnalysisToPMC();
			break;
		case MenuGraphPMC:
			ProcessMenuGraphPMC();
			break;
		case MenuPMCOnOff:
			ProcessMenuPMCOnOff();
			break;
		case MenuWriteAnalysisPoint:
			if( Two.Init)
				WriteAnalysisFile();
			else
				PressKeyToContMsg( "Must init 2 first");
			break;
		case MenuInputEquat:
			ProcessMenuInputEquat();
			break;
		case MenuInputEquatToAltaz:
			ProcessMenuInputEquatToAltaz();
			break;
		case MenuOffsetEquat:
			ProcessMenuOffsetEquat();
			break;
		case MenuDriftEquat:
			ProcessMenuDriftEquat();
			break;
		case MenuInputAltaz:
			ProcessMenuInputAltaz();
			break;
		case MenuOffsetAltaz:
			ProcessMenuOffsetAltaz();
			break;
		case MenuDriftAltaz:
			ProcessMenuDriftAltaz();
			break;
		case MenuZeroDrift:
			ProcessMenuZeroDrift();
			break;
      case MenuResetDrift:
         ProcessMenuResetDrift();
         break;
		case MenuResetAccumDrift:
			ProcessMenuResetAccumDrift();
			break;
		case MenuSav1:
			ProcessSav1();
			break;
		case MenuRes1:
			ProcessRes1();
			break;
		case MenuSav2:
			ProcessSav2();
			break;
		case MenuRes2:
			ProcessRes2();
			break;
		case MenuRestoreLastObject:
			ProcessMenuRestoreLastObject();
			break;
		case MenuHomeCoord:
			ProcessMenuHomeCoord();
			break;
		case MenuMoveEquat:
			ProcessMenuMoveEquat();
			break;
		case MenuMoveAltaz:
			ProcessMenuMoveAltaz();
			break;
		case MenuMoveHome:
			ProcessMenuMoveHome();
			break;
		case MenuMoveHs:
			ProcessMenuMoveHs();
			break;
		case MenuMoveMs:
			ProcessMenuMoveMs();
			break;
		case MenuMoveGEMFlip:
			if( GEMFlipPossible)
				ProcessMenuMoveGEMFlip();
			else
				PressKeyToContMsg( "GEMFlip not possible according to configuration");
			break;
		case MenuMoveZeroPEC:
			ProcessMenuMoveZeroPEC();
			break;
		case MenuFocusFastSpeed:
			ProcessMenuFocusFastSpeed();
			break;
		case MenuFocusSlowSpeed:
			ProcessMenuFocusSlowSpeed();
			break;
		case MenuMoveFocus:
			if( FocusMethod == FocusMethod_Pulse_1_14 || FocusMethod == FocusMethod_Pulse_16_17)
				ProcessMenuMoveFocus();
			else
				PressKeyToContMsg( "Not appropriate FocusMethod");
			break;
		case MenuMoveFocusEP:
			if( FocusMethod == FocusMethod_Pulse_1_14 || FocusMethod == FocusMethod_Pulse_16_17)
				ProcessMenuMoveFocusEP();
			else
				PressKeyToContMsg( "Not appropriate FocusMethod");
			break;
		case MenuSaveFocusEP:
			if( FocusMethod == FocusMethod_Pulse_1_14 || FocusMethod == FocusMethod_Pulse_16_17)
				ProcessMenuSaveFocusEP();
			else
				PressKeyToContMsg( "Not appropriate FocusMethod");
			break;
		case MenuResetFocus:
			if( FocusMethod == FocusMethod_Pulse_1_14 || FocusMethod == FocusMethod_Pulse_16_17)
				ProcessMenuResetFocus();
			else
				PressKeyToContMsg( "Not appropriate FocusMethod");
			break;
		case MenuResetEquat:
			ProcessMenuResetEquat();
			break;
		case MenuResetAltaz:
			ProcessMenuResetAltaz();
			break;
		case MenuResetHome:
			ProcessMenuResetHome();
			break;
		case MenuInitEncoders:
			ProcessMenuInitEncoders();
			break;
		case MenuResetToEncoders:
			ProcessMenuResetToEncoders();
			break;
		case MenuGEMMeridianFlip:
			if( GEMFlipPossible)
				ProcessMenuGEMMeridianFlip();
			else
				PressKeyToContMsg( "GEMFlip not possible according to configuration");
			break;
		case MenuGEMFlipPossible:
			ProcessMenuGEMFlipPossible();
			break;
		case MenuFRReset:
			ProcessMenuFRReset();
			break;
		default:
			ProcessHPEvents();
	}
}

void ProcessMenuMoveHs( void)
{
	struct AZLong HoldSteps, DeltaSteps;
	Flag EncoderReadFlag;
	struct AZDouble StartEncoderAZ, DeltaEncoderAZ;

	/* get steps to move */
	HoldSteps.A = HoldSteps.Z = 0;
	WriteWindow( MsgFrame);
	VidMemXY.X = MsgFrame.Left + 3;
	VidMemXY.Y = MsgFrame.Top + 2;
	gotoxy( VidMemXY.X, VidMemXY.Y);
	printf( "Enter altitude halfsteps to move (+ or -) ");
	gotoxy( VidMemXY.X += 43, VidMemXY.Y);
	if( GetLong( &HoldSteps.A))
	{
		VidMemXY.X = MsgFrame.Left + 3;
		VidMemXY.Y = MsgFrame.Top + 4;
		gotoxy( VidMemXY.X, VidMemXY.Y);
		printf( "Enter azimuth halfsteps to move (+ or -) ");
		gotoxy( VidMemXY.X += 43, VidMemXY.Y);
		if( GetLong( &HoldSteps.Z))
		{
			VidMemXY.X = MsgFrame.Left + 3;
			VidMemXY.Y = MsgFrame.Top + 6;
			gotoxy( VidMemXY.X, VidMemXY.Y);
			ContMsgRoutine();
			RemoveWindow( MsgFrame);
			/* save current encoder values */
			EncoderReadFlag = No;
			if( EncoderState > NotInitialized)
			{
				QueryAndReadEncoders();
				if( EncoderState == Read)
				{
					EncoderReadFlag = Yes;
					StartEncoderAZ.A = EncoderAZ.A;
					StartEncoderAZ.Z = EncoderAZ.Z;
				}
			}
			/* move desired steps */
			Dir.A = CW;
			if( HoldSteps.A < 0)
			{
				HoldSteps.A = -HoldSteps.A;
				Dir.A = CCW;
			}
			Dir.Z = CW;
			if( HoldSteps.Z < 0)
			{
				HoldSteps.Z = -HoldSteps.Z;
				Dir.Z = CCW;
			}
			Steps.A = HoldSteps.A;
			Steps.Z = HoldSteps.Z;
			KBEventMoveHs();
			PauseUntilNewSidTime();
			HPEventGetEquat();
			/* analyze vis-a-vis encoders */
			if( EncoderReadFlag && EncoderState > NotInitialized)
			{
				QueryAndReadEncoders();
				if( EncoderState == Read)
				{
					DeltaEncoderAZ.A = EncoderAZ.A - StartEncoderAZ.A;
					BoundsHalfRev( &DeltaEncoderAZ.A);
					DeltaEncoderAZ.Z = EncoderAZ.Z - StartEncoderAZ.Z;
					BoundsOneRev( &DeltaEncoderAZ.Z);
					DeltaSteps.A = HoldSteps.A - Steps.A;
					if( Dir.A == CCW)
						DeltaSteps.A = -DeltaSteps.A;
					DeltaSteps.Z = HoldSteps.Z - Steps.Z;
					if( Dir.Z == CCW)
						DeltaSteps.Z = -DeltaSteps.Z;

					WriteWindow( MsgFrame);
					VidMemXY.X = MsgFrame.Left + 3;
					VidMemXY.Y = MsgFrame.Top + 2;
					gotoxy( VidMemXY.X, VidMemXY.Y);
					printf( "alt %ld hsteps, openloop %5.5fd, encoder %5.5fd", DeltaSteps.A,
					(((double)DeltaSteps.A*AltFullStepSizeArcsec)/2)*DegPerArcsec,
					DeltaEncoderAZ.A*RadToDeg);
					VidMemXY.X = MsgFrame.Left + 3;
					VidMemXY.Y = MsgFrame.Top + 3;
					gotoxy( VidMemXY.X, VidMemXY.Y);
					printf( "fstep openloop %5.5f\", calculated %5.5f\"", AltFullStepSizeArcsec,
					DeltaSteps.A==0? 0: ((DeltaEncoderAZ.A/(double)DeltaSteps.A)*2)*RadToArcsec);

					VidMemXY.X = MsgFrame.Left + 3;
					VidMemXY.Y = MsgFrame.Top + 5;
					gotoxy( VidMemXY.X, VidMemXY.Y);
					printf( "az %ld hsteps, openloop %5.5fd, encoder %5.5fd",
					DeltaSteps.Z, (((double)DeltaSteps.Z*AzFullStepSizeArcsec)/2)*DegPerArcsec,
					DeltaEncoderAZ.Z*RadToDeg);
					VidMemXY.X = MsgFrame.Left + 3;
					VidMemXY.Y = MsgFrame.Top + 6;
					gotoxy( VidMemXY.X, VidMemXY.Y);
					printf( "fstep openloop %5.5f\", calculated %5.5f\"", AzFullStepSizeArcsec,
					DeltaSteps.Z==0? 0: ((DeltaEncoderAZ.Z/(double)DeltaSteps.Z)*2)*RadToArcsec);

					VidMemXY.X = MsgFrame.Left + 3;
					VidMemXY.Y = MsgFrame.Top + 7;
					gotoxy( VidMemXY.X, VidMemXY.Y);
					ContMsgRoutine();
					RemoveWindow( MsgFrame);
				}
			}
		}
		else
			RemoveWindow( MsgFrame);
	}
	else
		RemoveWindow( MsgFrame);
}

void ProcessMenuMoveMs( void)
{
	struct AZLong HoldSteps, TotalSteps;
	struct XY AltXY, AzXY;

	/* get steps to move */
	WriteWindow( MsgFrame);
	VidMemXY.X = MsgFrame.Left + 2;
	VidMemXY.Y = MsgFrame.Top + 2;
	gotoxy( VidMemXY.X, VidMemXY.Y);
	printf( "Enter altitude microsteps to move (+ or -) ");
	gotoxy( VidMemXY.X += 44, VidMemXY.Y);
	if( GetLong( &HoldSteps.A))
	{
		VidMemXY.X = MsgFrame.Left + 2;
		VidMemXY.Y = MsgFrame.Top + 3;
		gotoxy( VidMemXY.X, VidMemXY.Y);
		printf( "Enter azimuth microsteps to move (+ or -) ");
		gotoxy( VidMemXY.X += 44, VidMemXY.Y);
		if( GetLong( &HoldSteps.Z))
		{
			VidMemXY.X = MsgFrame.Left + 2;
			VidMemXY.Y = MsgFrame.Top + 4;
			gotoxy( VidMemXY.X, VidMemXY.Y);
			ContMsgRoutine();
			VidMemXY.X = MsgFrame.Left + 2;
			VidMemXY.Y = MsgFrame.Top + 5;
			gotoxy( VidMemXY.X, VidMemXY.Y);
			printf( "press any key to abort...");
			VidMemXY.X = MsgFrame.Left + 2;
			VidMemXY.Y = MsgFrame.Top + 6;
			gotoxy( VidMemXY.X, VidMemXY.Y);
			printf( "steps moved alt:       az:");
			AltXY.Y = AzXY.Y = MsgFrame.Top + 5;
			AltXY.X = MsgFrame.Left + 18;
			AzXY.X = MsgFrame.Left + 28;

			/* round up to nearest fullstep */
			if( HoldSteps.A % Ms)
				HoldSteps.A += (Ms - (HoldSteps.A % Ms));
			if( HoldSteps.Z % Ms)
				HoldSteps.Z += (Ms - (HoldSteps.Z % Ms));

			Dir.A = CW;
			if( HoldSteps.A < 0)
			{
				HoldSteps.A = -HoldSteps.A;
				Dir.A = CCW;
			}
			Dir.Z = CW;
			if( HoldSteps.Z < 0)
			{
				HoldSteps.Z = -HoldSteps.Z;
				Dir.Z = CCW;
			}

			TotalSteps.A = HoldSteps.A;
			TotalSteps.Z = HoldSteps.Z;

			while( !KeyStroke && (HoldSteps.A > 0 || HoldSteps.Z > 0))
			{
				if( HoldSteps.A > MsTick.A)
					Steps.A = MsTick.A;
				else
					Steps.A = HoldSteps.A;
				if( HoldSteps.Z > MsTick.Z)
					Steps.Z = MsTick.Z;
				else
					Steps.Z = HoldSteps.Z;

				/* subtract steps expected to move */
				HoldSteps.A -= Steps.A;
				HoldSteps.Z -= Steps.Z;

				MoveMs_f_ptr();

				/* add back in steps remaining, ie, not moved */
				HoldSteps.A += Steps.A;
				HoldSteps.Z += Steps.Z;

				TextAttr = DisplayText;
				VidMemXY.X = AltXY.X;
				VidMemXY.Y = AltXY.Y;
				sprintf( StrBuf, "%5ld", TotalSteps.A-HoldSteps.A);
				WriteStrBufToScreen_f_ptr();
				VidMemXY.X = AzXY.X;
				VidMemXY.Y = AzXY.Y;
				sprintf( StrBuf, "%5ld", TotalSteps.Z-HoldSteps.Z);
				WriteStrBufToScreen_f_ptr();
			}
			if( KeyStroke)
				getch();

			SetCurrentAltazToAccumMs();

			VidMemXY.X = MsgFrame.Left + 2;
			VidMemXY.Y = MsgFrame.Top + 7;
			gotoxy( VidMemXY.X, VidMemXY.Y);
			ContMsgRoutine();
			RemoveWindow( MsgFrame);

			PauseUntilNewSidTime();
			HPEventGetEquat();
		}
		else
			RemoveWindow( MsgFrame);
	}
	else
		RemoveWindow( MsgFrame);
}

/*
if not flipped, then scope is on east side of pier facing west, if true, then scope on west side facing east:
consequently, travel CW to flipped position in northern hemisphere, travel CCW in southern hemisphere;
if alt<pole (DecDistance>0), then not flipped;
*/
void ProcessMenuMoveGEMFlip( void)
{
	double DecDistance, RaDistance;
	struct AZLongV HoldSteps;
	struct AZFlag HoldDir;

	Steps.Z = 0;
	Dir.A = Dir.Z = CW;
	DecDistance = QtrRev - Current.Alt;
	if( DecDistance < 0)
	{
		DecDistance = -DecDistance;
		Dir.A = CCW;
	}
	if( LatitudeDeg >= 0)
	{
		if( Dir.A == CCW)
			Dir.Z = CCW;
	}
	else
		if( Dir.A == CW)
			Dir.Z = CCW;

	Steps.A = DecDistance / HsRad.A;
	HoldSteps = Steps;
	HoldDir = Dir;

	for( Ix = 0; Ix < 4; Ix++)
	{
		sound( 400);
		delay( 200);
		nosound();
		delay( 200);
	}
	TextAttr = CurrentText;
	VidMemXY = DisplayXY[DisplayGEMFlipMove];
	sprintf( StrBuf, "moving to pole in DEC: %ld hs; %3.3f deg", Steps.A, DecDistance*RadToDeg);
	WriteStrBufToScreen_f_ptr();

	KBEventMoveHs();
	PauseUntilNewSidTime();
	HPEventGetEquat();

	if( !KeyStroke)
	{
		DrawScreenLine( DisplayLine1Y);
		TextAttr = CurrentText;
		VidMemXY = DisplayXY[DisplayGEMFlipMove];
		if( DisplayOpeningMsgs && !LX200ComPort)
		{
			sprintf( StrBuf, "move CW or CCW across the meridian (c/w)? ");
			WriteStrBufToScreen_f_ptr();
			GetResponseWithLX200Check();
			if( Response == 'c' || Response == 'C')
				Dir.Z = CW;
			else
				Dir.Z = CCW;
		}
		RaDistance = HalfRev;
		Steps.Z = RaDistance / HsRad.Z;
		Steps.A = 0;

		for( Ix = 0; Ix < 3; Ix++)
		{
			sound( 400);
			delay( 200);
			nosound();
			delay( 200);
		}
		DrawScreenLine( DisplayLine1Y);
		TextAttr = CurrentText;
		VidMemXY = DisplayXY[DisplayGEMFlipMove];
		sprintf( StrBuf, "moving %s across meridian in RA: %ld hs; %3.3f deg", Dir.Z==CW? "CW": "CCW",
		Steps.Z, RaDistance*RadToDeg);
		WriteStrBufToScreen_f_ptr();

		KBEventMoveHs();
		PauseUntilNewSidTime();
		HPEventGetEquat();

		if( !KeyStroke)
		{
			Steps = HoldSteps;
			Dir.A = HoldDir.A;

			for( Ix = 0; Ix < 2; Ix++)
			{
				sound( 400);
				delay( 200);
				nosound();
				delay( 200);
			}

			DrawScreenLine( DisplayLine1Y);
			TextAttr = CurrentText;
			VidMemXY = DisplayXY[DisplayGEMFlipMove];
			sprintf( StrBuf, "moving from pole in DEC: %ld hs; %3.3f deg", Steps.A, DecDistance*RadToDeg);
			WriteStrBufToScreen_f_ptr();

			KBEventMoveHs();
			PauseUntilNewSidTime();
			HPEventGetEquat();

			for( Ix = 0; Ix < 1; Ix++)
			{
				sound( 400);
				delay( 200);
				nosound();
				delay( 200);
			}
		}
		else
			ProcessMenuMoveGEMFlipCancel();
	}
	else
		ProcessMenuMoveGEMFlipCancel();

	DrawScreenLine( DisplayLine1Y);
}

void ProcessMenuMoveGEMFlipCancel( void)
{
	while( KeyStroke)
		getch();
	PressKeyToContMsg( "GEM Flip move cancelled");
	/* function inverts TrackFlag, leading to tracking being turned off */
	TrackFlag = On;
	ProcessMenuTrack();
}

void ProcessMenuFocusFastSpeed( void)
{
	double focusspeed;

	WriteWindow( MsgFrame);
	VidMemXY.X = MsgFrame.Left + 2;
	VidMemXY.Y = MsgFrame.Top + 3;
	gotoxy( VidMemXY.X, VidMemXY.Y);
	printf( "Please enter fast focus steps per second ");
	if( GetDouble( &focusspeed))
		FocusFastStepsSec = focusspeed;
	RemoveWindow( MsgFrame);
	if( FocusFastStepsSec < 1)
	{
		PressKeyToContMsg( "Fast focus speed must be > 0, setting it to 1");
		FocusFastStepsSec = 1;
	}
	if( FocusFastStepsSec > 500)
	{
		PressKeyToContMsg( "Fast focus speed cannot exceed 500, setting it to 500");
		FocusFastStepsSec = 500;
	}
	WriteFocusFastSpeed();
}

void ProcessMenuFocusSlowSpeed( void)
{
	double focusspeed;

	WriteWindow( MsgFrame);
	VidMemXY.X = MsgFrame.Left + 2;
	VidMemXY.Y = MsgFrame.Top + 3;
	gotoxy( VidMemXY.X, VidMemXY.Y);
	printf( "Please enter slow focus steps per second ");
	if( GetDouble( &focusspeed))
		FocusSlowStepsSec = focusspeed;
	RemoveWindow( MsgFrame);
	if( FocusSlowStepsSec < 1)
	{
		PressKeyToContMsg( "Slow focus speed must be > 0, setting it to 1");
		FocusSlowStepsSec = 1;
	}
	if( FocusSlowStepsSec > 500)
	{
		PressKeyToContMsg( "Slow focus speed cannot exceed 500, setting it to 500");
		FocusSlowStepsSec = 500;
	}
	WriteFocusSlowSpeed();
}

void ProcessMenuMoveFocus( void)
{
	int NewFocusPosition;
	int StepsToMove, StepsMoved;
	int msdelay;

	WriteWindow( MsgFrame);
	VidMemXY.X = MsgFrame.Left + 2;
	VidMemXY.Y = MsgFrame.Top + 3;
	gotoxy( VidMemXY.X, VidMemXY.Y);
	printf( "Please enter focus position to move to ");
	if( GetInt( &NewFocusPosition))
	{
		_setcursortype( _NOCURSOR);
		TextAttr = DisplayText;
		SetFocusControlLines();
		/* set focus motor direction */
		StepsToMove = NewFocusPosition - FocusPosition;
		if( StepsToMove < 0)
		{
			StepsToMove = -StepsToMove;
			ReverseFocusDir();
		}
		if( ReverseFocusMotor)
			ReverseFocusDir();
		StepsMoved = 0;
		FocusFastDisplayFlag = True;
		msdelay = 1000 / (FocusFastStepsSec * 2);
		/* get state of port so as to not disturb unused parallel port pins */
		GetUnusedPPortLines();
		/* setup handpad vars */
		InitHandpad = Handpad;
		HandpadOKFlag = Yes;
		while( StepsMoved < StepsToMove && HandpadOKFlag && !KeyStroke)
		{
			delay( msdelay);
			/* raise focus motor pulse */
			BiDirOutNibbleValue = UnusedFocusPPortLines + FocusPulseBit + FocusDir;
			BiDirOutNibble();
			if( FocusDir)
			{
				FocusDiagPulseHighCW++;
				FocusPosition++;
			}
			else
			{
				FocusDiagPulseHighCCW++;
				FocusPosition--;
			}
			delay( msdelay);
			/* lower focus motor pulse */
			BiDirOutNibbleValue = UnusedFocusPPortLines + FocusDir;
			BiDirOutNibble();
			FocusDiagPulseLow++;
			SetHandpadOKFlag();
			StepsMoved++;
			/* display focus position */
			VidMemXY.X = MsgFrame.Left + 41;
			VidMemXY.Y = MsgFrame.Top + 3;
			gotoxy( VidMemXY.X, VidMemXY.Y);
			sprintf( StrBuf, "%04d", FocusPosition);
			WriteStrBufToScreen_f_ptr();
		}
		_setcursortype( _NORMALCURSOR);
	}
	if( KeyStroke)
		getch();
	VidMemXY.X = MsgFrame.Left + 2;
	VidMemXY.Y = MsgFrame.Top + 5;
	gotoxy( VidMemXY.X, VidMemXY.Y);
	printf( "Move finished. ");
	ContMsgRoutine();
	RemoveWindow( MsgFrame);
}

void ProcessKBMoveFocus( void)
{
	int MsDelay;

	SetFocusControlLines();
	/* set focus motor direction */
	if( Response == F6)
		ReverseFocusDir();
	if( ReverseFocusMotor)
		ReverseFocusDir();

	FocusFastDisplayFlag = False;
	MsDelay = 1000 / (FocusSlowStepsSec * 2);
	/* get state of port so as to not disturb unused parallel port pins */
	GetUnusedPPortLines();
	while( !NewSidT())
	{
		delay( MsDelay);
		/* raise focus motor pulse */
		BiDirOutNibbleValue = UnusedFocusPPortLines + FocusPulseBit + FocusDir;
		BiDirOutNibble();
		if( FocusDir)
		{
			FocusDiagPulseHighCW++;
			FocusPosition++;
		}
		else
		{
			FocusDiagPulseHighCCW++;
			FocusPosition--;
		}
		delay( MsDelay);
		/* lower focus motor pulse */
		BiDirOutNibbleValue = UnusedFocusPPortLines + FocusDir;
		BiDirOutNibble();
		FocusDiagPulseLow++;
		SetHandpadOKFlag();
	}
}

/* eyepiece and eyepiece focusing section contributed by R.Bonomini */

void ProcessMenuSaveFocusEP( void)
{
	int Ix = 0;
	int EPchoice = 0;
	int PositionChoice = 0;
	int Ok = 0;

	WriteWindow( EPFrame);
	VidMemXY.X = EPFrame.Left + 4;
	VidMemXY.Y = EPFrame.Top + 1;
	while (Ix < Eyepieces)
	{
		gotoxy( VidMemXY.X + 35*(Ix%2), VidMemXY.Y + (Ix/2+1));
		printf( "<%d>[%4d] %s ", Ix+1, EPFocusPosition[Ix].Position, EPFocusPosition[Ix].Name);
		Ix++;
	}
	VidMemXY.Y = VidMemXY.Y + (Eyepieces/2+2);
	gotoxy( VidMemXY.X, VidMemXY.Y);
	printf( "Choose Eyepiece to change: ");
	VidMemXY.Y=VidMemXY.Y+1;
	gotoxy( VidMemXY.X, VidMemXY.Y);
	if( GetInt( &EPchoice) && EPchoice)
	{
		VidMemXY.X = EPFrame.Left + 4;
		VidMemXY.Y=VidMemXY.Y+1;
		gotoxy( VidMemXY.X, VidMemXY.Y);
		if( EPchoice)
		{
			printf( "Please enter new focus position for eyepiece %d: ", EPchoice);
			VidMemXY.Y = VidMemXY.Y + 1;
			gotoxy( VidMemXY.X, VidMemXY.Y);
			if( GetInt( &PositionChoice))
			{
				if (EPchoice && PositionChoice)
				{
					EPFocusPosition[EPchoice-1].Position = PositionChoice;
					Ok = 1;
				}
			}
		}
	}
	VidMemXY.Y = VidMemXY.Y + 1;
	VidMemXY.X = EPFrame.Left + 4;
	gotoxy( VidMemXY.X, VidMemXY.Y);
	printf( "%s", Ok?"Updated ":"Not updated ");
	ContMsgRoutine();
	RemoveWindow( EPFrame);
}

void ProcessMenuMoveFocusEP( void)
{
	int NewFocusPosition = 0;
	int StepsToMove;
	int StepsMoved;
	int msdelay;
	int Ix = 0;
	int Choice = 0;

	WriteWindow( EPFrame);
	VidMemXY.X = EPFrame.Left + 4;
	VidMemXY.Y = EPFrame.Top + 1;
	while (Ix<Eyepieces)
	{
		gotoxy( VidMemXY.X + 35*(Ix%2), VidMemXY.Y + (Ix/2+1));
		printf( "<%d>[%4d] %s ", Ix+1, EPFocusPosition[Ix].Position, EPFocusPosition[Ix].Name);
		Ix++;
	}
	VidMemXY.Y = VidMemXY.Y + (2 + Eyepieces/2 + Eyepieces%2);
	gotoxy( VidMemXY.X, VidMemXY.Y);
	printf( "Choose Eyepiece: ");
	if( GetInt( &Choice))
		NewFocusPosition = Choice? EPFocusPosition[Choice-1].Position : 0;

	/* R.Bonomini: after getting an absolute focus position, move to it with the same code as above */
	VidMemXY.Y = VidMemXY.Y - 1;
	if( NewFocusPosition)
	{
		_setcursortype( _NOCURSOR);
		TextAttr = DisplayText;
		SetFocusControlLines();
		/* set focus motor direction */
		StepsToMove = NewFocusPosition - FocusPosition;
		if( StepsToMove < 0)
		{
			StepsToMove = -StepsToMove;
			ReverseFocusDir();
		}
		if( ReverseFocusMotor)
			ReverseFocusDir();
		StepsMoved = 0;
		FocusFastDisplayFlag = True;
		msdelay = 1000 / (FocusFastStepsSec * 2);
		/* get state of port so as to not disturb unused parallel port pins */
		GetUnusedPPortLines();
		/* setup handpad vars */
		InitHandpad = Handpad;
		HandpadOKFlag = Yes;
		while( StepsMoved < StepsToMove && HandpadOKFlag && !KeyStroke)
		{
			delay( msdelay);
			/* raise focus motor pulse */
			BiDirOutNibbleValue = UnusedFocusPPortLines + FocusPulseBit + FocusDir;
			BiDirOutNibble();
			if( FocusDir)
			{
				FocusDiagPulseHighCW++;
				FocusPosition++;
			}
			else
			{
				FocusDiagPulseHighCCW++;
				FocusPosition--;
			}
			delay( msdelay);
			/* lower focus motor pulse */
			BiDirOutNibbleValue = UnusedFocusPPortLines + FocusDir;
			BiDirOutNibble();
			FocusDiagPulseLow++;
			SetHandpadOKFlag();
			StepsMoved++;
			/* display focus position */
			VidMemXY.X = EPFrame.Left + 21;
			gotoxy( VidMemXY.X, VidMemXY.Y);
			sprintf( StrBuf, "%04d", FocusPosition);
			WriteStrBufToScreen_f_ptr();
		}
		_setcursortype( _NORMALCURSOR);
	}
	if( KeyStroke)
		getch();
	VidMemXY.X = EPFrame.Left + 4;
	VidMemXY.Y = EPFrame.Top + 4 + Eyepieces/2 + Eyepieces%2;
	gotoxy( VidMemXY.X, VidMemXY.Y);
	if (Choice)
		printf( "Move to <%d>[%4d] finished. ", Choice, NewFocusPosition);
	ContMsgRoutine();
	RemoveWindow( EPFrame);
}

void ProcessMenuResetFocus( void)
{
	double focuspos;

	WriteWindow( MsgFrame);
	VidMemXY.X = MsgFrame.Left + 2;
	VidMemXY.Y = MsgFrame.Top + 3;
	gotoxy( VidMemXY.X, VidMemXY.Y);
	printf( "Please enter new focuser position ");
	if( GetDouble( &focuspos))
		FocusPosition = focuspos;
	RemoveWindow( MsgFrame);
	WriteFocusPosition();
}

void ProcessMenuMoveZeroPEC( void)
{
	struct AZInt IxToMove;

	if( PECFlag)
	{
		if( PECIx.A > PECSize/2)
		{
			IxToMove.A = PECSize - PECIx.A;
			Dir.A = CW;
		}
		else
		{
			IxToMove.A = PECIx.A;
			Dir.A = CCW;
		}
		Steps.A = IxToMove.A * MsPerPECIx / MsPerHs;

		if( PECIx.Z > PECSize/2)
		{
			IxToMove.Z = PECSize - PECIx.Z;
			Dir.Z = CW;
		}
		else
		{
			IxToMove.Z = PECIx.Z;
			Dir.Z = CCW;
		}
		Steps.Z = IxToMove.Z * MsPerPECIx / MsPerHs;

		KBEventMoveHs();
		PauseUntilNewSidTime();
		HPEventGetEquat();
	}
	else
		PressKeyToContMsg( "PEC off - cannot move");
}

void AddAltOffset( void)
{
	struct Position Temp;

	Current.Alt += AltOffset;
	SetAccumMsToCurrentAltaz();
	SetEncoderAZandEncoderOffset();
	Three.Alt += AltOffset;
	Two.Alt += AltOffset;
	One.Alt += AltOffset;
	AltOffset = 0;
	Temp = Current;
	Current = One;
	strcpy( WhyInit, WHY_INIT_ALTOFF);
	/* not KBEventInitMatrix() so as to avoid displaying init info */
	InitMatrix( 1);
	Current = Temp;
}

void ProcessAltOffset( void)
{
	double HoldAltOffset = AltOffset;

	if( Two.Init)
	{
		WriteWindow( MsgFrame);
		VidMemXY.X = MsgFrame.Left + 3;
		VidMemXY.Y = MsgFrame.Top + 2;
		gotoxy( VidMemXY.X, VidMemXY.Y);
		printf( "calculating...");
		CalcAltOffsetIteratively( One, Two);
		RemoveWindow( MsgFrame);
	}
	if( AltOffset != MAXDOUBLE || Two.Init != Yes)
	{
		WriteWindow( MsgFrame);
		VidMemXY.X = MsgFrame.Left + 3;
		VidMemXY.Y = MsgFrame.Top + 2;
		gotoxy( VidMemXY.X, VidMemXY.Y);
		printf( "Add in altitude offset %3.3f and re-init (y/n)? ",
		AltOffset*RadToDeg);
		GetResponseWithLX200Check();
		printf( "%c", Response);

		if( Response == 'Y' || Response == 'y' || Response == Return)
		{
			AddAltOffset();
			VidMemXY.X = MsgFrame.Left + 3;
			VidMemXY.Y = MsgFrame.Top + 4;
			gotoxy( VidMemXY.X, VidMemXY.Y);
			printf( "Old altitude offset %3.3f, new %3.3f", HoldAltOffset*RadToDeg,
			AltOffset*RadToDeg);
			VidMemXY.X = MsgFrame.Left + 3;
			VidMemXY.Y = MsgFrame.Top + 6;
			gotoxy( VidMemXY.X, VidMemXY.Y);
			ContMsgRoutine();
			HPEventGetEquat();
		}
		else
		{
			VidMemXY.X = MsgFrame.Left + 3;
			VidMemXY.Y = MsgFrame.Top + 4;
			gotoxy( VidMemXY.X, VidMemXY.Y);
			printf( "altitude not offset");
			VidMemXY.X = MsgFrame.Left + 3;
			VidMemXY.Y = MsgFrame.Top + 6;
			gotoxy( VidMemXY.X, VidMemXY.Y);
			ContMsgRoutine();
		}
		RemoveWindow( MsgFrame);
		DisplayInitStatusOnScreen();
	}
	else
		PressKeyToContMsg( "must have 2 inits before alt offset can be calculated");
}

void ProcessAzOffsetAxisAlign( void)
{
	struct Position Temp;
	double HoldZ1Deg, HoldZ2Deg, HoldZ3Deg;

	if( Three.Init)
	{
		Temp = Current;
		HoldZ1Deg = Z1Deg;
		HoldZ2Deg = Z2Deg;
		HoldZ3Deg = Z3Deg;
		WriteWindow( MsgFrame);
		Three.Init = No;
		Current = One;
		InitMatrix( 1);
		VidMemXY.X = MsgFrame.Left + 3;
		VidMemXY.Y = MsgFrame.Top + 2;
		gotoxy( VidMemXY.X, VidMemXY.Y++);
		printf( "calculating...");
		ComputeBestZ123FromPosition( &Three);
		gotoxy( VidMemXY.X, VidMemXY.Y++);
		printf( "axis misalignment (Z1) = %3.2f deg", Z1*RadToDeg);
		gotoxy( VidMemXY.X, VidMemXY.Y++);
		printf( "azimuth offset (Z2) = %3.2f deg", Z2*RadToDeg);
		gotoxy( VidMemXY.X, VidMemXY.Y++);
		printf( "altitude offset (Z3) = %3.2f deg", Z3*RadToDeg);
		gotoxy( VidMemXY.X, VidMemXY.Y+=1);
		ContMsgRoutine();
		/* go back to original initialization since ComputeBestZ123FromPosition() changed it */
		Z1Deg = HoldZ1Deg;
		Z2Deg = HoldZ2Deg;
		Z3Deg = HoldZ3Deg;
		SetMountErrorsDeg( HoldZ1Deg, HoldZ2Deg, HoldZ3Deg);
		Three.Init = Yes;
		Current = One;
		InitMatrix( 1);
		Current = Temp;
		RemoveWindow( MsgFrame);
	}
	else
		PressKeyToContMsg( "must have 3 inits before Z1, Z2, Z3 can be calculated");
}

void ProcessSav1( void)
{
	/* saves current coordinates to SavedIn */
	SavedIn = Current;
	ProcessHPEvents();
}

void ProcessRes1( void)
{
	/* retrieves SavedIn coordinates and puts them in Input fields */
	In = SavedIn;
	DisplayIn( "saved input #1", NameBlanks);
	ProcessHPEvents();
}

void ProcessSav2( void)
{
	/* saves current coordinates to SavedIn2 */
	SavedIn2 = Current;
	ProcessHPEvents();
}

void ProcessRes2( void)
{
	/* retrieves SavedIn2 coordinates and puts them in Input fields */
	In = SavedIn2;
	DisplayIn( "saved input #2", NameBlanks);
	ProcessHPEvents();
}

void ProcessMenuDecMotor( void)
{
	WriteWindow( MsgFrame);
	VidMemXY.X = MsgFrame.Left + 3;
	VidMemXY.Y = MsgFrame.Top + 2;
	gotoxy( VidMemXY.X, VidMemXY.Y++);
	if( EnableMotor.A)
		printf( "Disable ");
	else
		printf( "Enable ");
	printf( "Declination motor microstepping (y/n)? ");
	GetResponseWithLX200Check();
	if( Response == 'Y' || Response == 'y' || Response == Return)
		EnableMotor.A = ! EnableMotor.A;
	RemoveWindow( MsgFrame);
}

void ProcessMenuReverseAZMotors( void)
{
	WriteWindow( MsgFrame);
	gotoxy( MsgFrame.Left + 3, MsgFrame.Top + 2);
	printf( "Reverse alt/dec motor (y/n)? ");
	GetResponseWithLX200Check();
	printf( "%c", Response);
	if( Response == 'Y' || Response == 'y' || Response == Return)
	{
		ReverseAMotor = !ReverseAMotor;
		InitHsArrays();
		InitMsArrays();
	}
	gotoxy( MsgFrame.Left + 3, MsgFrame.Top + 4);
	printf( "Reverse az/RA motor (y/n)? ");
	GetResponseWithLX200Check();
	printf( "%c", Response);
	if( Response == 'Y' || Response == 'y' || Response == Return)
	{
		ReverseZMotor = !ReverseZMotor;
		InitHsArrays();
		InitMsArrays();
	}
	delay( 500);
	RemoveWindow( MsgFrame);
}

void ProcessMenuReverseFRMotor( void)
{
	WriteWindow( MsgFrame);
	gotoxy( MsgFrame.Left + 3, MsgFrame.Top + 2);
	printf( "Reverse field rotation motor (y/n)? ");
	GetResponseWithLX200Check();
	printf( "%c", Response);
	if( Response == 'Y' || Response == 'y' || Response == Return)
		ReverseFRMotor = !ReverseFRMotor;
	delay( 500);
	RemoveWindow( MsgFrame);
}

void ProcessMenuReverseFocusMotor( void)
{
	WriteWindow( MsgFrame);
	gotoxy( MsgFrame.Left + 3, MsgFrame.Top + 2);
	printf( "Reverse focuser motor (y/n)? ");
	GetResponseWithLX200Check();
	printf( "%c", Response);
	if( Response == 'Y' || Response == 'y' || Response == Return)
		ReverseFocusMotor = !ReverseFocusMotor;
	delay( 500);
	RemoveWindow( MsgFrame);
}

