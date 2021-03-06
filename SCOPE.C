/*
copyright 1990 through 2009 by Mel Bartels

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

void InputEquatSlewDat( void)
{
	/* read (rb) slew.dat where:
	R <Ra degrees>
	D <Dec degrees>
	p <Alt degrees>
	q <Az degrees>
	...
	t <time()> <JD>
	L <lat> <long> */
	Input = fopen( FullyQualifiedSlewFile, "r");
	if( Input == NULL)
		PressKeyToContMsg( "Could not read slew file");
	else
	{
		do
			fscanf( Input, "%s", Name);
		while( strcmpi( Name, "R") != 0);
		FReadDouble( Input, &In.Ra);
		In.Ra /= RadToDeg;
		do
			fscanf( Input, "%s", Name);
		while( strcmpi( Name, "D") != 0);
		FReadDouble( Input, &In.Dec);
		In.Dec /= RadToDeg;
		fclose( Input);
		DisplayIn( "from guide (Ra, Dec)", NameBlanks);
	}
}

void WriteEquatSlewOutFile( void)
{
	Output = fopen( FullyQualifiedSlewOutFile, "w");
	if( Output == NULL)
		PressKeyToContMsg( "Could not write slew out file");
	else
	{
		/* az, then alt */
		fprintf( Output, "R %f %f", Current.Ra*RadToDeg, Current.Dec*RadToDeg);
		fclose( Output);
	}
}

void WriteAltazSlewOutFile( void)
{
	Output = fopen( FullyQualifiedSlewOutFile, "w");
	if( Output == NULL)
		PressKeyToContMsg( "Could not write slew out file");
	/* az, then alt */
	fprintf( Output, "%f %f", Current.Az*RadToDeg, Current.Alt*RadToDeg);
	fclose( Output);
}

void WriteLogFile( void)
{
	char LogFileName[NameSize];
	char c;

	printf( "\ncreate log file from input.dat file ('y' for yes, any other key to ignore)? ");
	Response = getch();
	printf( "%c", Response);
	if( Response == 'Y' || Response == 'y' || Response == Return)
	{
		getdate( &d);

		d.da_year -= 100*(d.da_year/100);
		LogFileName[0] = '0' + d.da_year/10;
		LogFileName[1] = '0' + d.da_year - 10*(d.da_year/10);
		LogFileName[2] = '0' + d.da_mon/10;
		LogFileName[3] = '0' + d.da_mon - 10*(d.da_mon/10);
		LogFileName[4] = '0' + d.da_day/10;
		LogFileName[5] = '0' + d.da_day - 10*(d.da_day/10);
		LogFileName[6] = 'a';
		LogFileName[7] = '.';
		LogFileName[8] = 'l';
		LogFileName[9] = 'o';
		LogFileName[10] = 'g';
		LogFileName[11] = EndOfStr;

		Input = fopen( LogFileName, "r");
		while( Input !=NULL && LogFileName[6] <= 'z')
		{
			fclose( Input);
			LogFileName[6]++;
			Input = fopen( LogFileName, "r");
		}
		if( LogFileName[6] > 'z')
		{
			fclose( Input);
			printf( "\n26 log files for the date have been created: cannot create anymore");
		}
		else
		{
			Input = fopen( InputFile, "r");
			if( Input == NULL)
				printf( "\nunable to open %s for read", InputFile);
			else
			{
				Output = fopen( LogFileName, "w");
				if( Output == NULL)
					printf( "\nunable to open %s for write", LogFileName);
				else
				{
					while( !feof( Input))
					{
						FReadChar( Input, &c);
						fprintf( Output, "%c", c);
					}
					fclose( Output);
					printf( "\nsaved log file %s", LogFileName);
				}
				fclose( Input);
			}
		}
	}
}

void BadExit( const char* Msg)
{
	CloseSteppers();
	CloseEncoderResetLogFile();
	CloseSerial( EncoderComPort);
	CloseSerial( LX200ComPort);
	BadMsgExit( Msg);
}

/* pin assignments:
A motor CW  = handpad up   = mouse up (1 click)    = aux 1  = mouse focus slow plus  = LX200 focus out
A motor CCW = handpad down = mouse down (2 clicks) = aux 14 = mouse focus slow minus = LX200 focus in
Z motor CW  = handpad CW   = mouse right (3 clicks)= aux 16 = mouse focus fast plus  = LX200 focus out
Z motor CCW = handpad CCW  = mouse left (4 clicks) = aux 17 = mouse focus fast minus = LX200 focus in

handpad CW  + MsSpeed = handpad focus slow plus
handpad CCW + MsSpeed = handpad focus slow minus

pport pin 1 = field rotation pulse or focus pulse
pport pin 14 = field rotation direction or focus direction, on = CW = out = plus
pport pin 16 = focus pulse
pport pin 17 = focus direction, on = CW = out = plus
*/

/* control paths:
	main() has a while !Quit loop:

		if GRAND TOUR activated and handpaddle signals time to load new object coordinates from file,
		then ProcessGrandTour()
			in ProcessGrandTour(), GetGrandTourRec() loads coordinates into Current coordinates and
			turns off handpaddle signal so that next time through while loop, if nothing intercedes,
			control passes to ProcessHPEvents() which will move scope to the grand tour coordinates;

		else if SCROLL TOUR activated then control passes to, and stays inside of ProcessScroll()
		until scroll finished
			in ProcessScroll(), if handpaddle indicates time for next scroll command, or auto scroll
			on, then command is acted upon,
				if altazimuth move, move is broken into steps per tick, then
					SetDirDistanceStepsThenMove() is called inside while loop to make the move until
					user entered time in the scroll command expires
				else if equatorial move, move is broken into steps per tick, then
					Move_Update_Handpad_Subr() is called inside while loop with current coordinates
					constantly incremented to make the move until user entered time in the scroll command
					expires;
						inside Move_Update_Handpad_Subr(),
						if handpaddle button pressed, ProcessHPEventsMoveMotors() is called otherwise
							MoveToCurrentRaDec() called if tracking on
							NOTE: ProcessHPEventsMoveMotors() calls either AltazMoveMs() which keeps
							control until microstepping ends, or KBEventMoveHs(), depending on microstep/
							halfstep switch setting
							NOTE: MoveToCurrentRaDec(), if two.init and tracking on, calls
							SetDirDistanceStepsThenMove() which calls either HPEventMoveMs() or
							HPEventMoveHs() depending on distance requirements:
								HPEventMoveMs() displays button status, checks backlash, and calls MoveMs():
									MoveMs() does field rotation calculations if appropriate, outputs the
									PWMs and FR, and updates the average PWMRepsTick

				else Move_Update_Handpad_Subr() is repeatedly called from within while loop until time
				for next scroll command

		else if mouse event, then ProcessMouseEvent()
		in ProcessMouseEvent(), if MouseModeMs(), control is kept until mouse button released

		else if KEYSTROKE then ProcessKBEvents()
			in ProcessKBEvents(), ProcessMenuResponse() is called:
				if F1-F4 virtual hand paddle direction button pressed, ProcessVirtualHandpad() is
				called, keeping control using HPEventMoveMs() until hand paddle button released

		else process HANDPADDLE events ProcessHPEvents()
				in ProcessHPEvents(),
					if auxiliary mode on, then ProcessHPEventsHandpadAux() called which turns on
					appropriate control bit then calls CheckMiscEvents()

					else if focus mode on, then either ProcessHPEventsFocus_OnOff_1617() or
					ProcessHPEventsFocus_OnOff_1617_Slow1_14() followed by CheckMiscEvents()

					else if direction buttons pressed
						if guide on, then ProcessHPEventsGuide() called which calls MoveToCurrentRaDec()
						else if FR turned on and field rotation motor move enabled by mode key movement
						and a CW/CCW direction button is pressed
							if tracking, then let MoveMs() calculate the FR move so call CheckMiscEvents()
							else ProcessHPEventsFR(), which keeps control until handpaddle button released
							or keyboard keypress
						else if focus turned on and focus motor move enabled by mode key movement
						and a CW/CCW direction button is pressed
							if tracking, then let MoveMs() calculate the Focus move so call
							CheckMiscEvents()
							else ProcessHPEventsFocusMethod_Pulse_Dir(), which keeps control until
							handpaddle button released or keyboard keypress
						else ProcessHPEventsMoveMotors() called (see NOTE above)

					else no direction buttons pressed, so look at mode switch,
						if possible after mode switch is processed, or if no mode switch action,
							CheckMiscEvents() is called:
								first, IACA events are checked for, if none, then
								secondly LX200 events are checked for ***, if none, then
								lastly MoveToCurrentRaDec() called (see NOTE above)
								*** if an LX200 event, then
									ReadLX200Input()
									if an LX200 motor command to process, then
										ProcessLX200_Motor_Cmd():
										in ProcessLX200_Motor_Cmd(),
											if start slew, then
												menu selection is updated and called, setting Current to In;
												next pass through ProcessHPEvents() will call CheckMiscEvents()
												which will call MoveToCurrentRaDec()
											else if LX200 speed setting is 'center' or 'find',
												control kept inside while loop with repeated calls of
												HPEventMoveMs() followed by ReadLX200Input()
											else
												MoveToCurrentRaDec() is called

control of repeated microstepping motion kept inside functions:
Track(), repeatedly calls MoveMs_f_ptr()
Track2Motors2Rates(), repeatedly calls MoveMs_f_ptr()
ProcessLX200_Motor_Cmd(), repeatedly calls HPEventMoveMs()
AltazMoveMs(), repeatedly calls HPEventMoveMs()
MoveBacklash(), repeatedly calls MoveMs_f_ptr()
ProcessMenuAutoMsParms(), repeatedly calls MoveMs_f_ptr()
ProcessMenuMoveMs(), repeatedly calls MoveMs_f_ptr()
ProcessVirtualHandpad(), repeatedly calls HPEventMoveMs()
ProcessMouseEvent(), repeatedly calls HPEventMoveMs()

control of repeated field rotation motion kept inside function:
ProcessHPEventsFR(), repeatedly calls NewSidT()

control of repeated focus motion kept inside functions:
ProcessHPEventsFocusMethod_Pulse_Dir(), repeatedly calls NewSidT()
and ProcessMenuMoveFocus()

during tracking, field rotation motion and focus motion setup at start of MoveMs() by:
	SetFRStepsDirMoveMs() (normal field rotation tracking)
	SetFRStepsDirHandpad() (if field rotation motor movement commanded by handpad)
	SetFocusStepsDirMoveMs() (focus motor commanded by handpad)
then processed by PulseFRFocusPerPWM() which is called at end of each PWM in MoveMs()

if FocusMethod == FocusMethod_Pulse_1_14 || FocusMethod == FocusMethod_Pulse_1617, then LX200 focus
control calls ProcessHPEventsFocusMethod_Pulse_Dir() if not tracking, if tracking is on, focus
control functions inside MoveMs() will step the focus motor */

void main( int argc, char** argv)
{
	//struct Position Pos;
	int Ix;

	strcpy( ConfigFile, "config.dat");

	Parity = NoneParity;
	DataBits = 8;
	StopBits = 1;

	CalledByGuideFlag = No;
	KeepGoingFlag = No;
	ReadSlewFlag = No;
	StartScrollFlag = No;

	/* if '-k' and '-s' (after full init): slew to Ra, Dec in slew.dat and keep going, exiting when
	desired, writing slew_out.dat file;
	if '-k' (before full init): (no slew.dat), keep going until centered on init position, write
	slew_out.dat file and exit;
	if '-s': slew to Ra, Dec in slew.dat and exit, writing slew_out.dat file;
	if no '-k' and no '-s': write slew_out.dat file and exit;

	if -c, then use following string as configuration file name, ie scope.exe -c config.dat will
	result in config.dat being used

	if -x, then use following string as scroll file name, and execute scroll file upon program
	startup, ie scope.exe -x nan.scr	will cause nan.scr to be loaded and started */

	/* argv[0] is name of executing program */
	for( Ix = 1; Ix < argc; Ix++)
		if( argv[Ix][0] == '-')
			if( strcmpi( &argv[Ix][1], "GUIDE") == 0)
				CalledByGuideFlag = Yes;
			else if( argv[Ix][1] == 'k')
				KeepGoingFlag = Yes;
			else if( argv[Ix][1] == 's')
				ReadSlewFlag = Yes;
			else if( (argv[Ix][1] == 'c' || argv[Ix][1] == 'C') && Ix < argc-1)
				strcpy( ConfigFile, argv[Ix+1]);
			else if( (argv[Ix][1] == 'x' || argv[Ix][1] == 'X') && Ix < argc-1)
			{
				strcpy( ScrollFilename, argv[Ix+1]);
				StartScrollFlag = Yes;
			}

	InitCommonVars();
	ReadConfig();

	/*
		Pos.Ra = Pos.Dec = 0;
		applyCorrectionsFromDataFileCoordYearToEpochNow(&Pos);
		printf("\n%f %f %f %f %f %f %f %f", Pos.Precession.A*RadToArcsec, Pos.Precession.Z*RadToArcsec,
		Pos.Nutation.A*RadToArcsec, Pos.Nutation.Z*RadToArcsec, Pos.AnnualAberration.A*RadToArcsec,
		Pos.AnnualAberration.Z*RadToArcsec, Pos.Ra*RadToArcsec, Pos.Dec*RadToArcsec);
		ContMsgRoutine();
	*/
	/*
		HsRecFile = fopen( HsRecFilename, "w");
		if( HsRecFile == NULL)
			BadExit( strcat( "Could not create ", HsRecFilename));
		HsRecIx = 0;
	*/

	/*
		InitTimes( DisplayOpeningMsgs, Tz, DST, LongitudeDeg);
		InitConvert();
		TestConvert();
		getch();
		TestAltAltAzTrack();
	*/


	if( DisplayOpeningMsgs)
	{
		printf( "\nCopyright BBAstroDesigns Inc. 2009\n");
		printf( "\nLIMITED WARRANTY This software is provided ``as is'' and any express or");
		printf( "\nimplied warranties, including, but not limited to, the implied warranties");
		printf( "\nof merchantability and fitness for a particular purpose are disclaimed.");
		printf( "\nIn no event shall BBAstroDesigns be liable for any direct, indirect,");
		printf( "\nincidental, special, exemplary, nor consequential damages (including, but");
		printf( "\nnot limited to, procurement of substitute goods or services, loss of use,");
		printf( "\ndate, or profits, or business interruption) however caused and on any");
		printf( "\ntheory of liability, whether in contract, strict liability, or tort");
		printf( "\n(including negligence or otherwise) arising in any way out of the use of");
		printf( "\nthis software, even if advised of the possibility of such damage.\n");
		printf( "\nThis software licensed under the GNU GENERAL PUBLIC LICENSE. You may");
		printf( "\ndistribute this software per the GNU GPL. See the enclosed gpl.txt.\n\n");
		ContMsgRoutine();
	}

	if( DisplayOpeningMsgs)
		printf( "\ncalled by guide: %d, keep_going: %d, read slew.dat file %d",
		CalledByGuideFlag, KeepGoingFlag, ReadSlewFlag);

	/* if( strcmpi( TestString, "TestSerial") == 0)
	{
		InitSerial( EncoderComPort, EncoderBaudRate, Parity, DataBits, StopBits);
		TestSerial( EncoderComPort);
		CloseSerial( EncoderComPort);
	} */
	/* else if( strcmpi( TestString, "TestVideo") == 0)
	{
		InitVideo( DisplayOpeningMsgs);
		TestVideo();
	} */
	/* else if( strcmpi( TestString, "TestATimes") == 0)
	{
		InitTimes( DisplayOpeningMsgs, Tz, DST, LongitudeDeg);
		TestTimes();
	} */
	/* else if( strcmpi( TestString, "TestParallelPort") == 0)
	{
		InitPPort();
		TestPPort();
		ClosePPort();
	} */
	/* else if( strcmpi( TestString, "TestRefract") == 0)
	{
		InitRefract();
		TestRefract();
	} */
	/* else if( strcmpi( TestString, "TestMouse") == 0)
	{
		TestMouse();
	} */
	/* else if( strcmpi( TestString, "TestEncoders") == 0)
	{
		InitSerial( EncoderComPort, EncoderBaudRate, Parity, DataBits, StopBits);
		InitEncoders();
		TestEncoders();
		CloseSerial( EncoderComPort);
	} */
	/* else if( strcmpi( TestString, "TestHandpad") == 0)
	{
		InitPPort();
		InitializeHandpad();
		TestHandpad();
		ClosePPort();
	} */
	/* else if( strcmpi( TestString, "TestConversion") == 0)
	{
		InitTimes( DisplayOpeningMsgs, Tz, DST, LongitudeDeg);
		InitConvert();
		TestConvert();
	} */
	/* else if( strcmpi( TestString, "TestAltOffset") == 0)
	{
		InitTimes( DisplayOpeningMsgs, Tz, DST, LongitudeDeg);
		InitConvert();
		TestAltOffset();
	} */
	/* else if( strcmpi( TestString, "TestIACA") == 0)
	{
		InitTimes( DisplayOpeningMsgs, Tz, DST, LongitudeDeg);
		TestIACA();
		InitIACA();
	} */
	/* else if( strcmpi( TestString, "WritePWMValues") == 0)
	{
		InitTimes( DisplayOpeningMsgs, Tz, DST, LongitudeDeg);
		InitVideo( DisplayOpeningMsgs);
		InitPPort();
		InitMotors();
		WritePWMValues();
		CloseSteppers();
		ClosePPort();
	}
	else */
	{
		if( strcmpi( TestString, "NoTest") != 0
		&& strcmpi( TestString, "PreloadGuidexx.dat") != 0
		&& strcmpi( TestString, "Track") != 0)
		{
			if( DisplayOpeningMsgs)
				printf( "\nsetting unrecognized TestString to 'NoTest'");
			strcpy( TestString, "NoTest");
		}
		InitSerial( EncoderComPort, EncoderBaudRate, Parity, DataBits, StopBits);
		InitEncoders();
		InitMouseControl();
		InitTimes( DisplayOpeningMsgs, Tz, DST, LongitudeDeg);
		InitVideo( DisplayOpeningMsgs);
		InitPPort();
		InitializeHandpad();
		InitMotors();
		InitConvert();
		InitRefract();
		InitPEC();
		InitGuide();
		if( strcmpi( TestString, "PreloadGuidexx.dat") == 0)
		{
			LoadGuideAlts();
			LoadGuideAzs();
		}
		InitIACA();
		InitLX200Input();
		InitHPEvent();
		if( !CalledByGuideFlag ||
		(CalledByGuideFlag && (KeepGoingFlag || ReadSlewFlag)))
		{
			InitKBEvent();
			if( ReadSlewFlag)
				InputEquatSlewDat();
			if( StartScrollFlag)
				LoadScrollFileFromFile();
			if( strcmpi( TestString, "Track") == 0)
				Start2MotorTrackWithDefaultValues();
			while( !QuitFlag)
			{
				SequentialTaskController();
				/* GrandTourFlag used to flag next object: set in ProcessHPEventsModeSwitch() */
				if( GrandTourLoaded && GrandTourFlag)
					ProcessGrandTour();
				else
					if( ScrollLoaded && ScrollFlag)
						ProcessScroll();
					else
						if( HPPolarAlignLoaded && HPPolarAlignFlag)
							ProcessHPPolarAlign();
						else
						{
							if( UseMouseFlag && ProcessMouseEvent())
								;
							else
								if( KeyStroke)
									ProcessKBEvents();
								else
									ProcessHPEvents();
						}
			}
			CloseKBEvent();
			if( DisplayOpeningMsgs)
			{
				AskAndWriteConfig();
				WriteLogFile();
			}
		}
		CloseSteppers();
		ClosePPort();
		CloseEncoderResetLogFile();
		CloseSerial( EncoderComPort);
		CloseSerial( LX200ComPort);
		if( CalledByGuideFlag)
			WriteAltazSlewOutFile();
		CloseMouseControl();
	}

	/*
		for( Ix = 0; Ix < HsRecSize; Ix++)
			fprintf( HsRecFile, "%8ld   %8ld\n", HsRec[Ix].A, HsRec[Ix].Z);
		// first position is index 0
		fprintf( HsRecFile, " last entry in circular queue at position %d", HsRecIx);
		fclose( HsRecFile);
	*/
}

