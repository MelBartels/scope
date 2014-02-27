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

/* HANDPAD LAYOUT

handpad uses 4 data lines, each line tied to a bit from the parallel port,
attach bit 16 to pin 13 of the parallel port,
attach bit 32 to pin 12 of the parallel port,
attach bit 64 to pin 10 of the parallel port,
attach bit 128 to pin 11 of the parallel port,

the handpad also needs + 5V DC

use a 9 pin serial connector cable to attach the handpad to the pc

when a button( s) is pressed, + 5V DC is applied to the appropriate bit( s), otherwise the bit( s)
is left pulled to ground

all bits are pulled to ground with 220 ohm resistors

use diodes positioned just after the switches and buttons to ensure that only the bits desired are
raised to + 5V DC

upper part of handpad is a 3 way switch, or alternatively, two 2 way switches, and is wired in the
following manner:
switch position #1, for initializing position #1 and other functions, activates bits 16, 64
switch position #2, neutral: does nothing, activates nothing
switch position #3, for initializing position #2 and other functions, activates bits 32, 64
switch also used for other functions via keyboard direction;

lower part of handpad consists of 4 momentary on buttons in the following pattern with a 2 way
switch in the middle:
  *
* S *
  *

upper button = Up, activates bit 16
lower button = Down, activates bit 32
left button = CCW, activates bit 64
right button = CW, activates bits 16, 32 (alternate handpad arrangement for external guiding inputs
	that precludes speed switch and mode switch since all 4 lines are devoted to directional input is
	for right button = CW to activate bit 128)

switch position #1, slow microstepping, activates bit 128
switch position #2, fast halfstepping, activates nothing */

/* parallel port 25 pin connector pin-out:
2 altitude stepper motor - red
3 altitude stepper motor - green
4 altitude stepper motor - red/white
5 altitude stepper motor - green/white
6 azimuth stepper motor - red
7 azimuth stepper motor - green
8 azimuth stepper motor - red/white
9 azimuth stepper motor - green/white
10 handpad input (pulled to ground via 220 ohm resistor)
11 handpad input (pulled to ground via 220 ohm resistor)
12 handpad input (pulled to ground via 220 ohm resistor)
13 handpad input (pulled to ground via 220 ohm resistor)
18-25 ground
1 optional field rotation motor pulse
14 optional field rotation motor direction
1, 14, 15, 17 optional auxiliary control
16 optional 5th phase altitude stepper
17 optional 5th phase azimuth stepper
15, 16, 17, 10+12+13 optional PEC auto synch signals
	10+12+13 can be simulated by:
		UpKey + DownKey + CCWKey
		CCWKey + CWKey
		UpKey + RightKey
		DownKey + LeftKey
*/

void InitializeHandpad( void)
{
	if( AutoAltPECPin == 101213l || AutoAzPECPin == 101213l)
		UseAutoPECSynch101213Flag = Yes;
	else
		UseAutoPECSynch101213Flag = No;

	if( HandpadPresentFlag)
		ReadHandpad_f_ptr = ReadHandpad;
	else
		ReadHandpad_f_ptr = NoHandpad;

	/* set this flag after checking handpad status */
	AutoPECSynch101213Detected = No;

	Handpad = MsKey;
	ReadHandpad_f_ptr();

	if( Buttons & CCWKey)
	{
		if( DisplayOpeningMsgs)
			printf( "\nhandpad not connected or CCW button depressed - ignoring handpad");
		ReadHandpad_f_ptr = NoHandpad;
		Buttons = Off;
	}
	if( Buttons & CWKey)
	{
		if( DisplayOpeningMsgs)
			printf( "\nhandpad not connected or CW button depressed - ignoring handpad");
		ReadHandpad_f_ptr = NoHandpad;
		Buttons = Off;
	}
	if( Buttons & UpKey)
	{
		if( DisplayOpeningMsgs)
			printf( "\nhandpad not connected or UP button depressed - ignoring handpad");
		ReadHandpad_f_ptr = NoHandpad;
		Buttons = Off;
	}
	if( Buttons & DownKey)
	{
		if( DisplayOpeningMsgs)
			printf( "\nhandpad not connected or Down button depressed - ignoring handpad");
		ReadHandpad_f_ptr = NoHandpad;
		Buttons = Off;
	}
	if( Buttons & LeftKey)
	{
		if( DisplayOpeningMsgs)
			printf( "\nhandpad not connected or Left button depressed - ignoring handpad");
		ReadHandpad_f_ptr = NoHandpad;
		Buttons = Off;
	}
	if( Buttons & RightKey)
	{
		if( DisplayOpeningMsgs)
			printf( "\nhandpad not connected or Right button depressed - ignoring handpad");
		ReadHandpad_f_ptr = NoHandpad;
		Buttons = Off;
	}
}

void SetHandpad( void)
{
	/* from InNibble(): only select upper 4 bits and then invert highest bit (128) */
	Handpad = (inportb( PPortAddrInNibble)& 240)^128;
}

void ReadHandpadSubr( void)
{
	int HoldHandpad;
	int rMsSpeed;
	int rButtons;

	if( UseAutoPECSynch101213Flag)
	{
		HoldHandpad = Handpad;
		SetHandpad();
		if( (Handpad & AutoPECSynch101213) == AutoPECSynch101213)
		{
			AutoPECSynch101213Detected = Yes;
			/* don't update Handpad if AutoPECSynch101213 */
			Handpad = HoldHandpad;
		}
		else
			AutoPECSynch101213Detected = No;
	}
	else
		SetHandpad();

	/* in standard design, data lines are multiplexed to indicate button/key: hence only 1 button/key
	can be indicated at a time */
	if( HandpadDesign == StandardHandpad)
	{
		rMsSpeed = Handpad&RawMsKey;
		rButtons = Handpad - rMsSpeed;
		if( UpDownButtonsReversedFlag)
			if( rButtons == RawUpKey)
				rButtons = RawDownKey;
			else
				if( rButtons == RawDownKey)
					rButtons = RawUpKey;
		if( rMsSpeed == RawMsKey)
			MsSpeed = MsKey;
		else
			MsSpeed = Off;
		switch( rButtons)
		{
			case RawUpKey:
				Buttons = UpKey;
				break;
			case RawDownKey:
				Buttons = DownKey;
				break;
			case RawCCWKey:
				Buttons = CCWKey;
				break;
			case RawCWKey:
				Buttons = CWKey;
				break;
			case RawLeftKey:
				Buttons = LeftKey;
				break;
			case RawRightKey:
				Buttons = RightKey;
				break;
			case None:
				Buttons = Off;
		}
	}
	/* simultaneous directions are possible since each data line is a direction line */
	else
		if( HandpadDesign == DirectionOnlyHandpad)
		{
			Buttons = Off;
			if( Handpad & RawUpKey)
				Buttons += UpKey;
			if( Handpad & RawDownKey)
				Buttons += DownKey;
			if( Handpad & RawCCWKey)
				Buttons += CCWKey;
			if( Handpad & RawAltCWKey)
				Buttons += CWKey;
		}
}

void ReadHandpad( void)
{
	static Flag ReadHandpadUnderway;

	if( !ReadHandpadUnderway)
	{
		ReadHandpadUnderway = True;
		ReadHandpadSubr();
		ReadHandpadUnderway = False;
	}
}

void NoHandpad( void)
{
	Handpad = MsKey;
}

void SetHandpadOKFlag( void)
{
	/* once Handpad no longer equals InitHandpad, don't let HandpadOKFlag be reset to True by
	subsequent reads of the hand paddle */
	if( HandpadOKFlag)
	{
		ReadHandpad_f_ptr();
		if( Handpad == InitHandpad)
			HandpadOKFlag = Yes;
		else
			HandpadOKFlag = No;
	}
}

/*
void TestHandpad( void)
{
	int Input;
	int Pad;

	printf( "\n\n\nTest of handpad code...'r' to read Pad, 'q' to quit.\n");
	Input = ' ';
	while( Input != 'q')
	{
		Input = getch();
		if( Input == 'r' || Input == 'R')
		{
			Pad = BiDirInNibble() + InNibble4Bit();
			printf( "bit1:%d", Pad&1?1:0);
			printf( " bit2:%d", Pad&2?1:0);
			printf( " bit4:%d", Pad&4?1:0);
			printf( " bit8:%d", Pad&8?1:0);
			printf( " bit16:%d", Pad&16?1:0);
			printf( " bit32:%d", Pad&32?1:0);
			printf( " bit64:%d", Pad&64?1:0);
			printf( " bit128:%d", Pad&128?1:0);
			NewLine;
			printf( "---> buttons pressed: ");
			Pad &= 240;
			if( Pad&RawMsKey)
			{
				printf( "Microstep ");
				Pad -= RawMsKey;
			}
			else
				printf( "Halfstep ");
			switch( Pad)
			{
				case RawLeftKey:
					printf( " leftkey");
					break;
				case RawRightKey:
					printf( " rightkey");
					break;
				case RawUpKey:
					printf( " up ");
					break;
				case RawDownKey:
					printf( " down ");
					break;
				case RawCCWKey:
					printf( " CCW ");
					break;
				case RawCWKey:
					printf( " CW ");
					break;
				case None:
					printf( " no other key pressed ");
					break;
				default:
					printf( " unknown ");
			}
			NewLine;
		}
	}
	NewLine;
	ContMsgRoutine();
}
*/
