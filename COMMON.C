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

void GetResponseWithLX200Check( void)
{
	while( !KeyStroke)
	{
		WaitForNewSidT();
		CheckLX200Events();
	}
	Response = getch();
}

/* writes ContMsg and waits for keyboard tap */
void ContMsgRoutine( void)
{
	printf( "%s", ContMsg);
	while( True)
	{
		CheckLX200Events();
		if( KeyStroke)
		{
			Response = getch();
			if( Response == ExtendedKeyboardStroke)
				Response = getch();
			break;
		}
		if( UseMouseFlag && MouseLeftButtonPressCount())
			break;
	}
}

void BadMsgExit( const char* Msg)
{
	clrscr();
	gotoxy( 1, 1);
	printf( "Error: %s.\n", Msg);
	ContMsgRoutine();
	_exit( 1);
}

void InitCommonVars( void)
{
	headkeys = (int far*) HeadKeysMemoryLocation;
	tailkeys = (int far*) TailKeysMemoryLocation;

	/* all internal values kept in radians:
	1 revolution = 2 Pi Radians = 360 Degrees = 24 Hours = 1440 Minutes */
	OneRev = 2*M_PI;
	HalfRev = OneRev/2;
	QtrRev = OneRev/4;
	RadToDeg = 360./OneRev;
	DegToRad = OneRev/360.;
	RadToArcmin = 60.*RadToDeg;
	ArcminToRad = DegToRad/60.;
	RadToArcsec = 60.*RadToArcmin;
	ArcsecToRad = ArcminToRad/60.;
	RadToTenthsArcsec = 10.*RadToArcsec;
	TenthsArcsecToRad = ArcsecToRad/10.;
	RadToHr = 24./OneRev;
	HrToRad = OneRev/24.;
	RadToMin = 60.*RadToHr;
	MinToRad = HrToRad/60.;
	RadToSec = 60.*RadToMin;
	SecToRad = MinToRad/60.;
	RadToHundSec = 100.*RadToSec;
	HundSecToRad = SecToRad/100.;
	RadToMilliSec = 1000.*RadToSec;
	MilliSecToRad = SecToRad/1000.;
	DaysToHr = 24.;
	DaysToMin = 60.*DaysToHr;
	DaysToSec = 60.*DaysToMin;
	ArcsecPerRev = 1296000.;
	RevPerArcsec = 1/1296000.;
	ArcsecPerDeg = 3600.;
	DegPerArcsec = 1/3600.;

	SidRate = 1.002737909;

	/* ~18.2065 clock ticks per second */
	ClockTicksDay = 1573040.;
   HalfClockTicksDay = ClockTicksDay / 2;
	ClockTicksSec = ClockTicksDay/DaysToSec;
	ClockTicksMin = ClockTicksDay/DaysToMin;
	ClockTicksHr = ClockTicksDay/DaysToHr;
	RadToClockTick = ClockTicksDay/OneRev;
	ClockTickToRad = OneRev/ClockTicksDay;

	LargeNum[0] = 1+2+4+8+16+32;
	LargeNum[1] = 2+4;
	LargeNum[2] = 1+2+8+16+64;
	LargeNum[3] = 1+2+4+8+64;
	LargeNum[4] = 2+4+32+64;
	LargeNum[5] = 1+4+8+32+64;
	LargeNum[6] = 4+8+16+32+64;
	LargeNum[7] = 1+2+4;
	LargeNum[8] = 1+2+4+8+16+32+64;
	LargeNum[9] = 1+2+4+32+64;

	IRQTable[0].IRQMask = 0x01;
	IRQTable[1].IRQMask = 0x02;
	IRQTable[2].IRQMask = 0x04;
	IRQTable[3].IRQMask = 0x08;
	IRQTable[4].IRQMask = 0x10;
	IRQTable[5].IRQMask = 0x20;
	IRQTable[6].IRQMask = 0x40;
	IRQTable[7].IRQMask = 0x80;
	IRQTable[8].IRQMask = 0x01;
	IRQTable[9].IRQMask = 0x02;
	IRQTable[10].IRQMask = 0x04;
	IRQTable[11].IRQMask = 0x08;
	IRQTable[12].IRQMask = 0x10;
	IRQTable[13].IRQMask = 0x20;
	IRQTable[14].IRQMask = 0x40;
	IRQTable[15].IRQMask = 0x80;

	IRQTable[0].Vector = 0x08;
	IRQTable[1].Vector = 0x09;
	IRQTable[2].Vector = 0x0A;
	IRQTable[3].Vector = 0x0B;
	IRQTable[4].Vector = 0x0C;
	IRQTable[5].Vector = 0x0D;
	IRQTable[6].Vector = 0x0E;
	IRQTable[7].Vector = 0x0F;
	IRQTable[8].Vector = 0x70;
	IRQTable[9].Vector = 0x71;
	IRQTable[10].Vector = 0x72;
	IRQTable[11].Vector = 0x73;
	IRQTable[12].Vector = 0x74;
	IRQTable[13].Vector = 0x75;
	IRQTable[14].Vector = 0x76;
	IRQTable[15].Vector = 0x77;

	CommPort[Com1].Base = 0x3F8;
	CommPort[Com2].Base = 0x2F8;

	CommPort[Com1].IRQ = 4;
	CommPort[Com2].IRQ = 3;

	/*com3 and com4 settings are defaulted in config.dat where users can change them via config file */
}

void BoundsOneRev( double* V)
{
	*V = ValidRad( *V);
}

void BoundsHalfRev( double* V)
{
	*V = ValidRadPi( *V);
}

void BoundsSinCos( double* V)
{
	if( *V > 1.)
		*V = 1.;
	if( *V < -1.)
		*V = -1.;
}

// bring a number in radians within the bounds of 0 to 2*Pi;
double ValidRad( double Rad)
{
	double RemoveNum;
	double Diff;

	RemoveNum = (double) ((long) (Rad/OneRev));
	Diff = Rad - RemoveNum*OneRev;

	if (Diff < 0.)
		Diff += OneRev;
	return Diff;
}

// bring a number in radians within the bounds of 0 to 2*Pi but then adjust
// the return value to be between -Pi to +Pi;
double ValidRadPi( double Rad)
{
	Rad = ValidRad( Rad);
	if (Rad > HalfRev)
		Rad -= OneRev;
	return Rad;
}

void DisplayNegSign( const int startx, const int starty, const int xsize, const int ysize,
const int attr)
{
	int endx;

	endx = startx + 2*xsize/3;
	VidMemXY.Y = starty+ysize/2;
	for( VidMemXY.X = startx+xsize/3; VidMemXY.X <= endx; VidMemXY.X++)
		Screen[VidMemXY.Y][VidMemXY.X].Attr = attr;
}

void DisplayColon( const int startx, const int starty, const int xsize, const int ysize,
const int attr)
{
	Screen[starty+ysize/3][startx+xsize/2].Attr = attr;
	Screen[starty+ysize/3][1+startx+xsize/2].Attr = attr;

	Screen[starty+2*ysize/3][startx+xsize/2].Attr = attr;
	Screen[starty+2*ysize/3][1+startx+xsize/2].Attr = attr;
}

void DisplayLargeNum( const int num, const int startx, const int starty, const int xsize,
const int ysize, const int attr)
{
	int endx, endy;

	if( num > 9)
		BadExit( "bad display digit in DisplayLargeNum(...)");

	if( LargeNum[num] & 1)
	{
		endx = startx + xsize+1;
		VidMemXY.Y = starty;
		for( VidMemXY.X = startx; VidMemXY.X <= endx; VidMemXY.X++)
			Screen[VidMemXY.Y][VidMemXY.X].Attr = attr;
	}
	if( LargeNum[num] & 64)
	{
		endx = startx + xsize+1;
		VidMemXY.Y = starty + ysize/2;
		for( VidMemXY.X = startx; VidMemXY.X <= endx; VidMemXY.X++)
			Screen[VidMemXY.Y][VidMemXY.X].Attr = attr;
	}
	if( LargeNum[num] & 8)
	{
		endx = startx + xsize+1;
		VidMemXY.Y = starty + ysize;
		for( VidMemXY.X = startx; VidMemXY.X <= endx; VidMemXY.X++)
			Screen[VidMemXY.Y][VidMemXY.X].Attr = attr;
	}
	if( LargeNum[num] & 32)
	{
		endy = starty + ysize/2;
		VidMemXY.X = startx;
		for( VidMemXY.Y = starty; VidMemXY.Y <= endy; VidMemXY.Y++)
		{
			Screen[VidMemXY.Y][VidMemXY.X].Attr = attr;
			Screen[VidMemXY.Y][VidMemXY.X+1].Attr = attr;
		}
	}
	if( LargeNum[num] & 2)
	{
		endy = starty + ysize/2;
		VidMemXY.X = startx + xsize;
		for( VidMemXY.Y = starty; VidMemXY.Y <= endy; VidMemXY.Y++)
		{
			Screen[VidMemXY.Y][VidMemXY.X].Attr = attr;
			Screen[VidMemXY.Y][VidMemXY.X+1].Attr = attr;
		}
	}
	if( LargeNum[num] & 16)
	{
		endy = starty + ysize;
		VidMemXY.X = startx;
		for( VidMemXY.Y = starty + ysize/2; VidMemXY.Y <= endy; VidMemXY.Y++)
		{
			Screen[VidMemXY.Y][VidMemXY.X].Attr = attr;
			Screen[VidMemXY.Y][VidMemXY.X+1].Attr = attr;
		}
	}
	if( LargeNum[num] & 4)
	{
		endy = starty + ysize;
		VidMemXY.X = startx + xsize;
		for( VidMemXY.Y = starty + ysize/2; VidMemXY.Y <= endy; VidMemXY.Y++)
		{
			Screen[VidMemXY.Y][VidMemXY.X].Attr = attr;
			Screen[VidMemXY.Y][VidMemXY.X+1].Attr = attr;
		}
	}
}

/*
void TestDisplayLargeNum( void)
{
	int startx = 1;
	int starty = 1;
	int xsize = 7;
	int ysize = 8;
	int attr = GREEN << 4;

	clrscr();
	for( Ix = 0; Ix <= 9; Ix++)
	{
		DisplayLargeNum( Ix, startx, starty, xsize, ysize, attr);
		startx += xsize + 3;
		if( startx >= 70)
		{
			startx = 1;
			starty = ysize + 3;
		}
	}
	ContMsgRoutine();
}
*/

void GetCurDir( char* Dir)
{
	/* fill string with "X:\" */
	strcpy( Dir, "X:\\");
	/* change the 'X' to the current drive */
	Dir[0] = 'A' + getdisk();
	/* fill rest of string with path */
	getcurdir( 0, Dir + 3);
}

Flag Gets( char Str[], const int NumChars)
{
	int IxA, IxB;

	for( IxA = 0; IxA < NumChars; IxA++)
		Str[IxA] = ' ';
	Str[NumChars] = EndOfStr;
	printf( "[");
	VidMemXY.X = wherex();
	VidMemXY.Y = wherey();
	gotoxy( VidMemXY.X + NumChars, VidMemXY.Y);
	printf( "]");
	gotoxy( VidMemXY.X, VidMemXY.Y);
	IxA = 0;
	GetResponseWithLX200Check();
	while( Response != Return && Response != Esc)
	{
		if( IxA && (Response == Backspace || Response == LeftCursor))
			Str[--IxA] = ' ';
		else if( IxA < NumChars)
			Str[IxA++] = Response;
		for( IxB = 0; IxB < IxA; IxB++)
		{
			gotoxy( VidMemXY.X + IxB, VidMemXY.Y);
			printf( "%c", Str[IxB]);
		}
		for( IxB = IxA; IxB < NumChars; IxB++)
		{
			gotoxy( VidMemXY.X + IxB, VidMemXY.Y);
			printf( " ");
		}
		gotoxy( VidMemXY.X + IxA, VidMemXY.Y);
		GetResponseWithLX200Check();
	}
	if( Response == Return)
		if( IxA)
			return UserEnteredNumber;
		else
			return UserDidNotEnterNumber;
	else
		return UserEscaped;
}

Flag GetNumStr( char NumStr[], int NumChars)
{
	int IxA, IxB;

	for( IxA = 0; IxA < NumChars; IxA++)
		NumStr[IxA] = ' ';
	NumStr[NumChars] = EndOfStr;
	printf( "[");
	VidMemXY.X = wherex();
	VidMemXY.Y = wherey();
	gotoxy( VidMemXY.X + NumChars, VidMemXY.Y);
	printf( "]");
	gotoxy( VidMemXY.X, VidMemXY.Y);
	IxA = 0;
	GetResponseWithLX200Check();
	while( Response != Return && Response != Esc)
	{
		if( IxA && (Response == Backspace || Response == LeftCursor))
			NumStr[--IxA] = ' ';
		else if( IxA < NumChars && ((Response >= '0' && Response <= '9') || Response == '.' ||
		Response == '+' || Response == '-'))
				NumStr[IxA++] = Response;
		for( IxB = 0; IxB < IxA; IxB++)
		{
			gotoxy( VidMemXY.X + IxB, VidMemXY.Y);
			printf( "%c", NumStr[IxB]);
		}
		for( IxB = IxA; IxB < NumChars; IxB++)
		{
			gotoxy( VidMemXY.X + IxB, VidMemXY.Y);
			printf( " ");
		}
		gotoxy( VidMemXY.X + IxA, VidMemXY.Y);
		GetResponseWithLX200Check();
	}
	if( Response == Return)
		if( IxA)
			return UserEnteredNumber;
		else
			return UserDidNotEnterNumber;
	else
		return UserEscaped;
}

Flag GetFlag( Flag* Num)
{
	char NumStr[MaxCharsFlag+1];
	Flag R;

	R = GetNumStr( NumStr, MaxCharsFlag);
	if( R == UserEnteredNumber)
		*Num = (Byte) atoi( NumStr);
	return R;
}

Flag GetInt( int* Num)
{
	char NumStr[MaxCharsInt+1];
	Flag R;

	R = GetNumStr( NumStr, MaxCharsInt);
	if( R == UserEnteredNumber)
		*Num = atoi( NumStr);
	return R;
}

Flag GetLong( long* Num)
{
	char NumStr[MaxCharsLong+1];
	Flag R;

	R = GetNumStr( NumStr, MaxCharsLong);
	if( R == UserEnteredNumber)
		*Num = atol( NumStr);
	return R;
}

Flag GetDouble( double* Num)
{
	char NumStr[MaxCharsDbl+1];
	Flag R;

	R = GetNumStr( NumStr, MaxCharsDbl);
	if( R == UserEnteredNumber)
		*Num = atof( NumStr);
	return R;
}

void FReadDouble( FILE* File, double* V)
{
	if( !feof( File))
	{
		fscanf( File, "%s", Value);
		*V = atof( Value);
	}
}

void FReadFlag( FILE* File, Flag* V)
{
	if( !feof( File))
	{
		fscanf( File, "%s", Value);
		*V = (Flag) atoi( Value);
	}
}

void FReadByte( FILE* File, Byte* V)
{
	if( !feof( File))
	{
		fscanf( File, "%s", Value);
		*V = (Byte) atoi( Value);
	}
}

void FReadInt( FILE* File, int* V)
{
	if( !feof( File))
	{
		fscanf( File, "%s", Value);
		*V = atoi( Value);
	}
}

void FReadUnsigned( FILE* File, unsigned* V)
{
	if( !feof( File))
	{
		fscanf( File, "%s", Value);
		*V = (unsigned) atoi( Value);
	}
}

void FReadLong( FILE* File, long* V)
{
	if( !feof( File))
	{
		fscanf( File, "%s", Value);
		*V = atol( Value);
	}
}

void FReadChar( FILE* File, char* V)
{
	if( !feof( File))
		fread( V, 1, 1, File);
	else
		*V = ' ';
}

void FReadToNewLine( FILE* File)
{
	char Dummy = ' ';

	while( !feof( File) && Dummy != '\n')
			fread( &Dummy, 1, 1, File);
}

void FReadToChar( FILE* File, const char V)
{
	char Dummy = ' ';

	while( !feof( File) && Dummy != V)
			fread( &Dummy, 1, 1, File);
}

/* read until char count exhausted or newline: return yes if newline encountered; reads past initial blanks */
Flag FReadStringToCharCountOrNewLine( FILE* File, char* Name, const int CharCount)
{
	int Ix;
	Flag NewLineFlag;

	Name[0] = EndOfStr;
	/* read past blanks */
	do
	{
		FReadChar( File, &Name[0]);
	}while( !feof( File) && Name[0] == ' ');
	Ix = 0;
	while( !feof( Input) && Ix < CharCount-1 && Name[Ix++] != '\n')
		FReadChar( File, &Name[Ix]);
	Ix--;
	if( Name[Ix] == '\n')
		NewLineFlag = True;
	else
		NewLineFlag = False;
	/* fill out name */
	while( Ix < CharCount-1)
		Name[Ix++] = EndOfStr;
	return NewLineFlag;
}

void FReadStringToCharCountAndNewLine( FILE* File, char* Name, const int CharCount)
{
	int Ix;
   char Dummy = ' ';

	Name[0] = EndOfStr;
	/* read past blanks */
	do
	{
		FReadChar( File, &Name[0]);
	}while( !feof( File) && Name[0] == ' ');
	Ix = 0;
	while( !feof( Input) && Ix < CharCount-1 && Name[Ix++] != '\n')
		FReadChar( File, &Name[Ix]);
	Ix--;
	if( Name[Ix] != '\n' && Name[Ix+1] != '\n')
     while( !feof( File) && Dummy != '\n')
       fread( &Dummy, 1, 1, File);

	/* fill out name */
	while( Ix < CharCount-1)
		Name[Ix++] = EndOfStr;

/* Now need to examine case where name string is at start of record and there's junk at end of file */
  if( (FileType == ReadCometElements || FileType == ReadAsteroidElements) && Name[0] == EndOfStr)
    while( !feof( File))
      fread( &Dummy, 1, 1, File);
}

Flag IsWhiteSpace( const char C)
{
	if( C == Blank ||
		 C == Tab ||
		 C == VTab ||
		 C == LF ||
		 C == Return ||
		 C == Backspace ||
		 C == NewLineChar ||
		 C == EndOfStr)
		return True;
	return False;
}

void ReadInputParseString( FILE* Input)
{
	FReadStringToCharCountOrNewLine( Input, ParsedString.Str, MaxStrIx);
	ParseString();
}

void ParseString( void)
{
	int Ix;

	ParsedString.CntBegIx = -1;
	ParsedString.BegIx[ParsedString.CntBegIx] = 0;
	ParsedString.PrevWhiteSpaceFlag = True;
	ParsedString.WhiteSpaceFlag = False;
	for( Ix = 0; Ix < MaxStrIx && ParsedString.Str[Ix] != '\n'; Ix++)
	{
		ParsedString.WhiteSpaceFlag = IsWhiteSpace( ParsedString.Str[Ix]);
		if( ParsedString.WhiteSpaceFlag)
			ParsedString.Str[Ix] = EndOfStr;
		if( ParsedString.PrevWhiteSpaceFlag && !ParsedString.WhiteSpaceFlag)
			ParsedString.BegIx[++ParsedString.CntBegIx] = Ix;
		ParsedString.PrevWhiteSpaceFlag = ParsedString.WhiteSpaceFlag;
	}
}

void TestReadInputParseString( FILE* Input)
{
	while( !feof( Input))
	{
		printf( "\nstart of line:");
		ReadInputParseString( Input);
		for( Ix = 0; Ix <= ParsedString.CntBegIx; Ix++)
			printf( "\n%d %s", ParsedString.BegIx[Ix], &ParsedString.Str[ParsedString.BegIx[Ix]]);
		getch();
	}
}

/* returns next increment count above highest numbered file name, ie, if name00.txt and name02.txt
exist, then 3 is returned, if no file exists, then returns 0 */
int FindNextIncrFilename( char* Name, char* Extension)
{
	char Fname[13];
	int value;
	int highestvalue = -1;

	strcpy( Fname, Name);
	strcat( Fname, "??.");
	strcat( Fname, Extension);

	FindFirst = True;
	FoundFile = True;

	while( FoundFile)
	{
		if( FindFirst)
		{
			FindFirst = False;
			/* returns 0 if successful */
			FoundFile = !findfirst( Fname, &Ffblk, 0);
		}
		else
			FoundFile = !findnext( &Ffblk);

		if( FoundFile)
			if( Ffblk.ff_name[strlen( Name)] >= '0' && Ffblk.ff_name[strlen( Name)] <= '9' &&
			Ffblk.ff_name[strlen( Name)+1] >= '0' && Ffblk.ff_name[strlen( Name)+1] <= '9')
			{
				value = (Ffblk.ff_name[strlen( Name)]-'0') * 10 + Ffblk.ff_name[strlen( Name)+1]-'0';

				if( value > highestvalue)
					highestvalue = value;
			}
	}
	return highestvalue + 1;
}

/* serial port... */

/* interrupt driven receive and transmit */

void interrupt Ser1ISR( void)
{
	unsigned Base = CommPort[Com1].Base;

	/* receive */
	while( inportb( Base + LSR) & DATA_READY)
	{
		SerBufRead[Com1][SerBufReadEndIx[Com1]++] = inportb( Base);
		/* SerBufEnd must be 2^n - 1 for this to work: much faster than comparing and setting to zero */
		SerBufReadEndIx[Com1] &= SerBufEnd;
		SerReadCount[Com1]++;
	}
	/* transmit */
	if( (inportb( Base + IIR) & 7) == XMT_INT)
		/* if no data to transmit then turn off transmit interrupt */
		if( SerBufWriteBegIx[Com1] == SerBufWriteEndIx[Com1])
		{
			/* clear DLAB */
			outportb( Base + LCR, inportb( Base+LCR) & ~DLAB);
			/* disable transmit interrupt */
			outportb( Base + IER, INT_ON_RCV);
		}
		/* else transmit Byte */
		else
		{
			outportb( Base, SerBufWrite[Com1][SerBufWriteBegIx[Com1]]);
			SerBufWriteBegIx[Com1]++;
			SerBufWriteBegIx[Com1] &= SerBufEnd;
			SerWriteCount[Com1]++;
		}
	/* enable PIC to service this interrupt again */
	if( CommPort[Com1].IRQ >= 8)
		outportb( PIC_EOI_ADDR_SLAVE, EOI);
	outportb( PIC_EOI_ADDR, EOI);
}

void interrupt Ser2ISR( void)
{
	unsigned Base = CommPort[Com2].Base;

	while( inportb( Base + LSR) & DATA_READY)
	{
		SerBufRead[Com2][SerBufReadEndIx[Com2]++] = inportb( Base);
		SerBufReadEndIx[Com2] &= SerBufEnd;
		SerReadCount[Com2]++;
	}
	if( (inportb( Base + IIR) & 7) == XMT_INT)
		if( SerBufWriteBegIx[Com2] == SerBufWriteEndIx[Com2])
		{
			outportb( Base + LCR, inportb( Base+LCR) & ~DLAB);
			outportb( Base + IER, INT_ON_RCV);
		}
		else
		{
			outportb( Base, SerBufWrite[Com2][SerBufWriteBegIx[Com2]]);
			SerBufWriteBegIx[Com2]++;
			SerBufWriteBegIx[Com2] &= SerBufEnd;
			SerWriteCount[Com2]++;
		}
	if( CommPort[Com2].IRQ >= 8)
		outportb( PIC_EOI_ADDR_SLAVE, EOI);
	outportb( PIC_EOI_ADDR, EOI);
}

void interrupt Ser3ISR( void)
{
	unsigned Base = CommPort[Com3].Base;

	while( inportb( Base + LSR) & DATA_READY)
	{
		SerBufRead[Com3][SerBufReadEndIx[Com3]++] = inportb( Base);
		SerBufReadEndIx[Com3] &= SerBufEnd;
		SerReadCount[Com3]++;
	}
	if( (inportb( Base + IIR) & 7) == XMT_INT)
		if( SerBufWriteBegIx[Com3] == SerBufWriteEndIx[Com3])
		{
			outportb( Base + LCR, inportb( Base+LCR) & ~DLAB);
			outportb( Base + IER, INT_ON_RCV);
		}
		else
		{
			outportb( Base, SerBufWrite[Com3][SerBufWriteBegIx[Com3]]);
			SerBufWriteBegIx[Com3]++;
			SerBufWriteBegIx[Com3] &= SerBufEnd;
			SerWriteCount[Com3]++;
		}
	if( CommPort[Com3].IRQ >= 8)
		outportb( PIC_EOI_ADDR_SLAVE, EOI);
	outportb( PIC_EOI_ADDR, EOI);
}

void interrupt Ser4ISR( void)
{
	unsigned Base = CommPort[Com4].Base;

	while( inportb( Base + LSR) & DATA_READY)
	{
		SerBufRead[Com4][SerBufReadEndIx[Com4]++] = inportb( Base);
		SerBufReadEndIx[Com4] &= SerBufEnd;
		SerReadCount[Com4]++;
	}
	if( (inportb( Base + IIR) & 7) == XMT_INT)
		if( SerBufWriteBegIx[Com4] == SerBufWriteEndIx[Com4])
		{
			outportb( Base + LCR, inportb( Base+LCR) & ~DLAB);
			outportb( Base + IER, INT_ON_RCV);
		}
		else
		{
			outportb( Base, SerBufWrite[Com4][SerBufWriteBegIx[Com4]]);
			SerBufWriteBegIx[Com4]++;
			SerBufWriteBegIx[Com4] &= SerBufEnd;
			SerWriteCount[Com4]++;
		}
	if( CommPort[Com4].IRQ >= 8)
		outportb( PIC_EOI_ADDR_SLAVE, EOI);
	outportb( PIC_EOI_ADDR, EOI);
}

Flag InitSerial( const int ComPort, const long BaudRate, const int Parity, const int DataBits,
const int StopBits)
{
	unsigned R;
	int Ix, IxA;
	unsigned Vector;
	int IRQ;
	int Mask;

	if( ComPort < 1 || ComPort > MaxCommPorts)
		return False;

	for( Ix = 0; Ix < MaxCommPorts; Ix++)
	{
		for( IxA = 0; IxA <= SerBufEnd; IxA++)
			SerBufWrite[Ix][IxA] = 0;
		SerBufWriteBegIx[Ix] = SerBufWriteEndIx[Ix] = 0;
		SerWriteCount[Ix] = SerBufEnd+1;
	}

	IRQ = CommPort[ComPort-1].IRQ;
	Vector = IRQTable[IRQ].Vector;
	Mask = IRQTable[IRQ].IRQMask;
	SerialBase = CommPort[ComPort-1].Base;
	/* save current serial interrupt routine and set new serial interrupt routine */
	switch( ComPort-1)
	{
		case Com1:
			OldSer1IntVec = getvect( Vector);
			setvect( Vector, Ser1ISR);
			break;
		case Com2:
			OldSer2IntVec = getvect( Vector);
			setvect( Vector, Ser2ISR);
			break;
		case Com3:
			OldSer3IntVec = getvect( Vector);
			setvect( Vector, Ser3ISR);
			break;
		case Com4:
			OldSer4IntVec = getvect( Vector);
			setvect( Vector, Ser4ISR);
			break;
	}
	/* set PIC by zeroing relevant bit */
	if( IRQ >= 8)
	{
		outportb( IntMaskReg, inportb( IntMaskReg) & (~IRQTable[2].IRQMask));
		outportb( IntMaskRegSlave, inportb( IntMaskReg) & (~Mask));
	}
	else
		outportb( IntMaskReg, inportb( IntMaskReg) & (~Mask));

	InitRingBuffers( ComPort);

	/* set UART */
	R = (unsigned) (115200L / BaudRate);
	/* loopback required by early 8250s */
	outportb( SerialBase + MCR, 0x10);
	/* set DLAB = 1, if divisor latch access bit == 1, then ports 0, 1 used
	as baud rate divisor, otherwise ports 0, 1 used to receive/transmit */
	outportb( SerialBase + LCR, 0x80);
	/* load rate divisor LSB */
	outportb( SerialBase, R & 0xFF);
	/* load divisor MSB */
	outportb( SerialBase + IER, (R>>8) & 0xFF);
	/* data bits: 1st two bits used to set DataBits; DataBits can range from 5 (bits value of 0)
	to 8 (bits value of 3) */
	/* stop bits: uses 3rd bit: if 2 StopBits, then set to 1, otherwise 0 */
	/* parity bits: uses 4th to 6th bits */
	/* 7th bit is break bit, 8th bit is DLAB (divisor latch access bit) */
	/* set DLAB back to zero, set DataBits, StopBits */
	outportb( SerialBase + LCR, ((DataBits-5) & 3) | Parity | (StopBits==2? 4:0));
	/* turn on DTR, RTS (set to 0 to turn off MCR) */
	outportb( SerialBase + MCR, MCR_DTR | MCR_RTS);
	/* enable UART */
	/* clear out any stray char */
	inportb( SerialBase);
	/* read LSR to clear it */
	inportb( SerialBase + LSR);
	/* ditto MSR */
	inportb( SerialBase + MSR);
	/* enable 8250 Rx interrupt only */
	outportb( SerialBase + IER, INT_ON_RCV);
	/* turn on RTS, DTR, OUT2 */
	outportb( SerialBase + MCR, 0x0B);
	/* code to turn on then off the 16550 fifo
	outportb( SerialBase + IIR, 0x07);
	printf( "\nserial fifo %s", (inportb( SerialBase + IIR) & 0xC0)? "on": "off");
	outportb( SerialBase + IIR, 0);
	printf( "\nserial fifo %s", (inportb( SerialBase + IIR) & 0xC0)? "on": "off");*/
	return True;
}

Flag CloseSerial( const int ComPort)
{
	Byte B;
	unsigned Vector;
	int IRQ;
	int Mask;

	if( ComPort < 1 || ComPort > MaxCommPorts)
		return False;

	/* clear out any remaining chars to read */
	while( ReadSerial( ComPort, &B))
		;

	IRQ = CommPort[ComPort-1].IRQ;
	Vector = IRQTable[IRQ].Vector;
	Mask = IRQTable[IRQ].IRQMask;

	SerialBase = CommPort[ComPort-1].Base;
	/* OUT2 off to disable int */
	outportb( SerialBase + MCR, inportb( SerialBase + MCR) ^ 0x08);
	/* reset PIC, set the relevant bit */
	if( IRQ >= 8)
	{
		outportb( IntMaskReg, inportb( IntMaskReg) | IRQTable[2].IRQMask);
		outportb( IntMaskRegSlave, inportb( IntMaskReg) | Mask);
	}
	else
		outportb( IntMaskReg, inportb( IntMaskReg) | Mask);

	/* restore original interrupt vector */
	switch( ComPort-1)
	{
		case Com1:
			setvect( Vector, OldSer1IntVec);
			break;
		case Com2:
			setvect( Vector, OldSer2IntVec);
			break;
		case Com3:
			setvect( Vector, OldSer3IntVec);
			break;
		case Com4:
			setvect( Vector, OldSer4IntVec);
			break;
	}
	return True;
}

void InitRingBuffers( const int ComPort)
{
	if( ComPort < 1 || ComPort > MaxCommPorts)
		BadExit( "Bad ComPort in InitRingBuffers()");
	disable();
	SerBufReadBegIx[ComPort-1] = SerBufReadEndIx[ComPort-1] = SerReadCount[ComPort-1] = 0;
	SerBufWriteBegIx[ComPort-1] = SerBufWriteEndIx[ComPort-1] = SerWriteCount[ComPort-1] = 0;
	enable();
}

Flag ReadSerial( const int ComPort, Byte* B)
{
	if( ComPort < 1 || ComPort > MaxCommPorts)
		return False;

	disable();
	if( SerReadCount[ComPort-1])
	{
		*B = SerBufRead[ComPort-1][SerBufReadBegIx[ComPort-1]++];
		/* SerBufEnd must be 2^n - 1 for this to work: very much faster than comparing and setting to zero */
		SerBufReadBegIx[ComPort-1] &= SerBufEnd;
		SerReadCount[ComPort-1]--;
		enable();
		return True;
	}
	else
	{
		enable();
		return False;
	}
}

Flag WaitForReadBytes( const int ComPort, const int numBytesToWaitFor, const int BailIntervalMilliSec)
{
	double BailTRad;

	if( ComPort < 1 || ComPort > MaxCommPorts)
		BadExit( "Bad ComPort in WaitForReadBytes()");

	NewSidT();
	BailTRad = SidT + BailIntervalMilliSec * MilliSecToRad;
	if( BailTRad > OneRev)
		BailTRad -= OneRev;

	while( SerReadCount[ComPort-1]<numBytesToWaitFor && BailTRad>SidT && (BailTRad-SidT)<HalfRev)
		WaitForNewSidT();

	if( SerReadCount[ComPort-1] >= numBytesToWaitFor)
		return True;
	else
		return False;
}

void WriteSerial( const int ComPort, const Byte B)
{
	if( ComPort < 1 || ComPort > MaxCommPorts)
		BadExit( "Bad ComPort in WriteSerial()");

	SerBufWrite[ComPort-1][SerBufWriteEndIx[ComPort-1]] = B;
	SerBufWriteEndIx[ComPort-1]++;
	SerBufWriteEndIx[ComPort-1] &= SerBufEnd;

	SerialBase = CommPort[ComPort-1].Base;
	/* clear DLAB */
	outportb( SerialBase + LCR, inportb( SerialBase+LCR) & ~DLAB);
	/* enable transmit interrupt */
	outportb( SerialBase + IER, INT_ON_RCV + INT_ON_XMT);
}

/* 9600 bps ~= 1000 cps (char/sec) or ~55 char/clocktick */
void WriteSerialString( const int ComPort, char C[], const int CharCount)
{
	int Ix;

	if( ComPort < 1 || ComPort > MaxCommPorts)
		BadExit( "Bad ComPort in WriteSerial()");

	for( Ix = 0; Ix < CharCount; Ix++)
	{
		SerBufWrite[ComPort-1][SerBufWriteEndIx[ComPort-1]] = C[Ix];
		SerBufWriteEndIx[ComPort-1]++;
		SerBufWriteEndIx[ComPort-1] &= SerBufEnd;
	}

	SerialBase = CommPort[ComPort-1].Base;
	/* clear DLAB */
	outportb( SerialBase + LCR, inportb( SerialBase+LCR) & ~DLAB);
	/* enable transmit interrupt */
	outportb( SerialBase + IER, INT_ON_RCV + INT_ON_XMT);
}

void WriteSerialPauseUntilXmtFinished( const int ComPort, const Byte B)
{
	WriteSerial( ComPort, B);
	while( !WriteSerialFinished(ComPort))
		;
}

void WriteSerialStringPauseUntilXmtFinished( const int ComPort, char C[], const int CharCount)
{
	WriteSerialString( ComPort, C, CharCount);
	while( !WriteSerialFinished(ComPort))
		;
}

Flag WriteSerialFinished( const int ComPort)
{
	if( ComPort < 1 || ComPort > MaxCommPorts)
		BadExit( "Bad ComPort in WriteSerialFinished()");

	SerialBase = CommPort[ComPort-1].Base;
	if( inportb( SerialBase + LSR) & XMT_COMPLETE)
		return True;
	else
		return False;
}

void DisplaySerialString( char C[], const int CharCount)
{
	printf( "\nserial output: ");
	for( Ix = 0; Ix < CharCount; Ix++)
		printf( "%c", C[Ix]);
	printf( " (hex: ");
	for( Ix = 0; Ix < CharCount; Ix++)
		printf( " %#x ", (Byte) C[Ix]);
	printf( ")");
}

void TestSerial( const int ComPort)
{
	Flag QuitFlag = No;
	Byte B;
	Flag Hex;

	CloseSerial( ComPort);
	InitSerial( ComPort, EncoderBaudRate, Parity, DataBits, StopBits);

	printf( "\n\n\nTest of serial port on com %1d (%ld,8,n,1):\n", ComPort, EncoderBaudRate);
	printf( "Display in hex (y/n)? ");
	Response = getch();
	printf( "%c", Response);
	printf( "\nPress ? to enter a string, end with the return key");
	printf( "\nPress ! to quit\n\n");
	if( Response == 'Y' || Response == 'y' || Response == Return)
		Hex = Yes;
	else
		Hex = No;

	while( !QuitFlag)
	{
		if( ReadSerial( ComPort, &B))
			if( Hex)
				printf( " %#x ", (int)B);
			else
				printf( "%c", B);

		if( KeyStroke)
		{
			B = getch();
			if( B == '!')
				QuitFlag = Yes;
			else
				if( B == '?')
				{
					gets( StrBuf);
					WriteSerialString( ComPort, StrBuf, strlen( StrBuf));
				}
				else
				{
					WriteSerial( ComPort, B);
					printf( "%c", B);
				}
		}
	}
	NewLine;
	ContMsgRoutine();
}

/* video... courtesy Dale Eason */

/* Speed gain by using direct video memory is considerable */

/* The PC's screen in text mode consists of 25 rows and 80 columns.
The first row and column is addressed as location (0, 0), and the last row and column as location
(24, 79).  This is swapped from the goto( x, y) function.  Each screen location actually consists of
two components: a character and an attribute.  To support this arragement, a structure called
'Texel' is used to represent the character and the attribute written to a screen location.

To write a character and its attribute to a screen location, a screen pointer is used:
ScreenArea far *ScreenPtr;

This screen pointer can be set to one of two values: B000:000h for a monochrome display adapter and
B800:000h for a graphics adapter.  To determine which adapter is installed, use a call to the
ROM-BIOS (interrupt 0x10).

Once ScreenPtr has been assigned a screen address, a character and an attribute can be written to
the screen.  But first, to simplify the notation, we've defined the macro constant: #define Screen
(*ScreenPtr)

note: (*ScreenPtr)[][] is a pointer to an array, vs *ScreenPtr[][] which is an array of pointers,
'[]' taking precedence over '*': (*ScreenPtr)[5][10].Ch = 'A' must be written with parenthesis;

Putting all this together, we can write the character A with normal video to row 6, column 11 using:

	  Screen[5][10].Ch = 'A';
	  Screen[5][10].Attr = 7;

We can also use this notation to read a character and its attributes at a specific screen location:

	 Ch = Screen[5][10].Ch;
	 Attr = Screen[5][10].Attr; */

void InitVideo( const Flag DisplayOpeningMsgs)
{
	Regs.h.ah = 15;
	int86( 0x10, &Regs, &Regs);
	if( Regs.h.al == 7)
		MemSeg = 0xB000;
	else
		MemSeg = 0xB800;
	MemOff = Regs.h.bh * (unsigned) 0x1000;
	ScreenPtr = (ScreenArea far*) (( (long) MemSeg << 16) | (long) MemOff);

	if( DisplayOpeningMsgs)
		if( MemSeg == 0xB000)
			printf( "\nvideo mode: monochrome");
		else
			printf( "\nvideo mode: graphics adaptor");

	WriteCharToScreen_f_ptr = WriteCharToScreen;
	WriteStrBufToScreen_f_ptr = WriteStrBufToScreen;
}

void NULL_FUNCTION( void) {}

/* write a character to the screen at Y and X */
void WriteCharToScreen( const char Char)
{
	Screen[VidMemXY.Y][VidMemXY.X].Ch = Char;
	Screen[VidMemXY.Y][VidMemXY.X].Attr = TextAttr;
	/*
	textattr( TextAttr);
	gotoxy( VidMemXY.X+1, VidMemXY.Y+1);
	cprintf( "%c", Char);
	*/
}

/* write a character string using StrBuf to the screen at Y and X; VidMemXY.X is changed, VidMemXY.Y
is not */
void WriteStrBufToScreen( void)
{
	char* StrPtr = StrBuf;

	while( *StrPtr)
	{
		Screen[VidMemXY.Y][VidMemXY.X].Attr = TextAttr;
		Screen[VidMemXY.Y][VidMemXY.X++].Ch = *StrPtr++;
	}
	/* textattr() and cprintf() will also print text to the console with the correct attributes, ie,
	textattr( TextAttr);
	gotoxy( VidMemXY.X+1, VidMemXY.Y+1);
	while( *StrPtr)
		cprintf( "%c", *StrPtr++);
	*/
}

void Clrscr( void)
{
	clrscr();

	TextAttr = DefaultText;
	gettextinfo( &ti);
	for( VidMemXY.X = ti.winleft-1; VidMemXY.X < ti.winright; VidMemXY.X++)
		for( VidMemXY.Y = ti.wintop-1; VidMemXY.Y < ti.winbottom; VidMemXY.Y++)
			WriteCharToScreen( ' ');

}

/*
void TestVideo( void)
{
	struct XY HoldXY;
	int i;
	const maxi = MAXINT - 1;
	time_t t;
	time_t start;

	clrscr();
	printf( "\n\n\nTest of video code...\n");
	printf( "segment %u   offset %d\n\n", MemSeg, MemOff);

	directvideo = 1;
	HoldXY.X = wherex();
	HoldXY.Y = wherey();
	printf( "To display ");

	// my turn
	start = time( NULL);
	for( i = 1; i <= maxi; i++)
	{
		// my direct video
		sprintf( StrBuf, "%6d", i);
		VidMemXY.X = HoldXY.X + 11;
		VidMemXY.Y = HoldXY.Y - 1;
		WriteStrBufToScreen_f_ptr();
	}
	t = time( NULL);
	gotoxy( HoldXY.X + 20, HoldXY.Y);
	printf( " times");
	gotoxy( HoldXY.X, HoldXY.Y + 1);
	printf( "Took my code %d seconds", t - start);

	// Borland's turn
	start = time( NULL);
	for( i = 1; i <= maxi; i++)
	{
		gotoxy( HoldXY.X + 12, HoldXY.Y);
		printf( "%6d", i);
	}
	t = time( NULL);
	gotoxy( HoldXY.X, HoldXY.Y + 2);
	printf( "Took Borland with direct video %d seconds", t - start);

	// direct video off
	directvideo = 0;
	start = time( NULL);
	for( i = 1; i <= maxi; i++)
	{
		gotoxy( HoldXY.X + 12, HoldXY.Y);
		printf( "%6d", i);
	}
	t = time( NULL);
	gotoxy( HoldXY.X, HoldXY.Y + 3);
	printf( "Borland w/o direct video %d seconds", t - start);

	NewLine;
	ContMsgRoutine();
}
*/

/* CMOS clock used to reset DOS clock when DOS clock has been disturbed */

int UpdateInProgress( void)
{
	int Ix;

	/* docs at http://www.cubic.org/source/archive/hardware/cmos/cmos.txt say that CMOS_Reg_Status_B
	can also be used */
	outportb( CCP, CMOS_Reg_Status_A);
	/* add delay to give time for return to settle */
	for( Ix=0; Ix < 5; Ix++)
		;
	return CMOS_RTN & Update_In_Progress;
}

void DisplayCMOSTimeDate( void)
{
	printf( "%2d:%02d:%02d   (%2d)%2d/%02d/%02d", CMOS_Hr, CMOS_Min, CMOS_Sec,
	Century, CMOS_Yr, CMOS_Mon, CMOS_Day);
}

void SetDOSToCMOS_RTC_ViaPort( void)
{
	disable();
	/* wait until CMOS clock is ready to increment to next second */
	while( !UpdateInProgress())
		;
	/* wait until CMOS clock is finished updating */
	while( UpdateInProgress())
		;

	/* get CMOS values */
	outportb( CCP, CMOS_Reg_BCD_Sec);
	BCD = CMOS_RTN;
	CMOS_Sec = DECODE_BCD;
	outportb( CCP, CMOS_Reg_BCD_Min);
	BCD = CMOS_RTN;
	CMOS_Min = DECODE_BCD;
	outportb( CCP, CMOS_Reg_BCD_Hr);
	BCD = CMOS_RTN;
	CMOS_Hr = DECODE_BCD;
	outportb( CCP, CMOS_Reg_BCD_Day);
	BCD = CMOS_RTN;
	CMOS_Day = DECODE_BCD;
	outportb( CCP, CMOS_Reg_BCD_Mon);
	BCD = CMOS_RTN;
	CMOS_Mon = DECODE_BCD;
	outportb( CCP, CMOS_Reg_BCD_Yr);
	BCD = CMOS_RTN;
	CMOS_Yr = DECODE_BCD;

	SetDOSToCMOS_Vars();
	enable();
}

void SetDOSToCMOS_RTC_ViaBios( void)
{
	ReadRealTimeClock( &CMOS_Hr, &CMOS_Min, &CMOS_Sec, &_daylight);
	ReadDateofRealTimeClock( &Century, &CMOS_Yr, &CMOS_Mon, &CMOS_Day);
	SetDOSToCMOS_Vars();
}

void SetDOSToCMOS_Vars( void)
{
	static int PrevYr, PrevMon, PrevDay;

	if( CMOS_Yr == 0 && PrevYr == 99 && CMOS_Mon == 1 && PrevMon == 12 && CMOS_Day == 1 &&
	PrevDay == 31 && Century == 19)
		Century = 20;
	d.da_year = Century * 100 + CMOS_Yr;
	d.da_mon = CMOS_Mon;
	d.da_day = CMOS_Day;
	setdate( &d);
	t.ti_hour = CMOS_Hr;
	t.ti_min = CMOS_Min;
	t.ti_sec = CMOS_Sec;
	t.ti_hund = 0;
	settime( &t);
	PrevYr = CMOS_Yr;
	PrevMon = CMOS_Mon;
	PrevDay = CMOS_Day;
}

void SetRealTimeClock( const int hr, const int min, const int sec)
{
	int daylight = 0;

	Regs.h.ah = 3;
	/* hr, min, sec in BCD format */
	BCD = hr;
	Regs.h.ch = ENCODE_BCD;
	BCD = min;
	Regs.h.cl = ENCODE_BCD;
	BCD = sec;
	Regs.h.dh = ENCODE_BCD;
	Regs.h.dl = daylight;
	int86( TimeOfDayInterrupt, &Regs, &Regs);
}

void SetDateofRealTimeClock( const int century, const int year, const int mon, const int day)
{
	Regs.h.ah = 5;
	/* century (19 or 20), year, mon, day in BCD format */
	BCD = century;
	Regs.h.ch = ENCODE_BCD;
	BCD = year;
	Regs.h.cl = ENCODE_BCD;
	BCD = mon;
	Regs.h.dh = ENCODE_BCD;
	BCD = day;
	Regs.h.dl = ENCODE_BCD;
	int86( TimeOfDayInterrupt, &Regs, &Regs);
}

Flag ReadRealTimeClock( int* hr, int* min, int* sec, int* daylight)
{
	Regs.h.ah = 2;
	int86( TimeOfDayInterrupt, &Regs, &Regs);
	BCD = Regs.h.ch;
	*hr = DECODE_BCD;
	BCD = Regs.h.cl;
	*min = DECODE_BCD;
	BCD = Regs.h.dh;
	*sec = DECODE_BCD;
	*daylight = Regs.h.dl;
	/* carry flag set if RTC not running (inaccessible) */
	return !Regs.x.cflag;
}

Flag ReadDateofRealTimeClock( int* century, int* year, int* mon, int* day)
{
	Regs.h.ah = 4;
	int86( TimeOfDayInterrupt, &Regs, &Regs);
	BCD = Regs.h.ch;
	*century = DECODE_BCD;
	BCD = Regs.h.cl;
	*year = DECODE_BCD;
	BCD = Regs.h.dh;
	*mon = DECODE_BCD;
	BCD = Regs.h.dl;
	*day = DECODE_BCD;
	/* carry flag set if RTC not running (inaccessible) */
	return !Regs.x.cflag;
}

/* astronomical times... */

/* Sidereal time algorithm: at program start, determine sidereal time based on date, time and
longitude, and read bios clock tick count; then repeatedly read the new bios clock tick count,
calculate # of ticks since program start, calculate the sidereal time increment based on tick count,
and add to program start sidereal time to find the new sidereal time */

void InitTimes( const Flag DisplayOpeningMsgs, const double Tz, const int DST, double LongitudeDeg)
{
	long Yr;
	int M;
	int D;
	int h;
	int m;
	double s;

	Ticks = (long far*) TICK_ADDR;
	/* set century for Y2K problem with CMOS clock */
	getdate( &d);
	Century = d.da_year / 100;

	if( CMOS_RTC_Access)
		SetDOSToCMOS_RTC_f_ptr = SetDOSToCMOS_RTC_ViaBios;
	else
		SetDOSToCMOS_RTC_f_ptr = SetDOSToCMOS_RTC_ViaPort;

	SetDOSToCMOS_RTC_f_ptr();

	if( DisplayOpeningMsgs)
	{
		if( CMOS_RTC_Access)
			printf( "\nCMOS RTC access via bios interrupt:");
		else
			printf( "\nCMOS RTC access via direct port calls:");
		printf( "\nUpdating DOS Time & Date to CMOS Time & Date of ");
		DisplayCMOSTimeDate();
	}

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
}

void SetStartBiosClockAndSidTime( const long Yr, const int M, const int D, const int h, const int m,
const double s, const double Tz, const int DST, double LongitudeDeg)
{
	/* set starting bios clock tick count */
	StartTicks = *Ticks;
	Days = 0;
	CalcJD( Yr, M, D, Tz, DST, h, m, s);
	CalcSidHr( Yr, M, D, Tz, DST, h, m, s, LongitudeDeg);
	StartSidT = SidT = SidHr*HrToRad;
}

int NewSidT( void)
{
	static long LastTicks;

	/* bios clock ticks... */
	if( *Ticks == LastTicks)
		return No;
	else
	{
		/* if ticks rolls over to zero */
		if( *Ticks < LastTicks - HalfClockTicksDay)
			Days++;
		LastTicks = *Ticks;
		/* calc sidereal time based on bios clock ticks since program start */
		SidT = ValidRad( StartSidT + SidRate * (LastTicks + Days*ClockTicksDay - StartTicks) * ClockTickToRad);
		Current.SidT = SidT;
		return Yes;
	}
}

void WaitForNewSidT( void)
{
	while( !NewSidT())
		;
}

double calcJDFromYear(const double year) {
	long longYear;
	double holdJD;
	double longYearJD;
	double elapsedDays;
	double JDFromYear;

	// start with calculating JD for Jan 1 of year
	longYear = (long) year;
	holdJD = JD;
	CalcJD(longYear, 1, 1, 0, 0, 0, 0, 0);
	longYearJD = JD;
	JD = holdJD;
	// get elapsed days based on fractional part of year
	elapsedDays = (year - (double) longYear) * 365.25;
	// add elapsed days
	JDFromYear = (double) longYearJD + elapsedDays;
	return JDFromYear;
}

void CalcJD( const long Y, const int M, const int D, const double Tz, const int DST, const int h,
const int m, const double s)
{
	long A;
	double B;

	/* return 1st formula if Gregorian calendar, otherwise 2nd formula for Julian calendar: Sky and
	Telescope, August, 1991, pg 183 */
	if( (Y > 1582) || (Y == 1582 && M > 10) || (Y == 1582 && M == 10 && D > 15))
		A = 367*Y - 7*(Y + (M + 9)/12)/4 - 3*((Y + (M - 9)/7)/100 + 1)/4 + 275*M/9 + D + 1721029L;
	else
		A = 367*Y - 7*(Y + 5001 + (M - 9)/7)/4 + 275*M/9 + D + 1729777L;
	/* subtract 12hrs since JD starts at 12noon UT */
	B = (Tz - DST - 12 + h + m/60. + s/3600)/24;
	JD = A + B;
}

void CalcSidHr( const long Y, const int M, const int D, const double Tz, const int DST, const int h,
const int m, const double s, const double LongitudeDeg)
{
	/* JD at 0hrs UT (will be in form of a whole number + 0.5) */
	double JD0HrUT;
	/* fractional part of JD beyond JD0HrUT */
	double FracDay;
	/* intermediate calculated result */
	double T;
	/* sidereal time at 0hrs UT */
	double Sid0HrUT;

	CalcJD( Y, M, D, Tz, DST, h, m, s);

	FracDay = fmod( JD, 1);
	/* FracDay meas. from 0hr UT or < JD > .5 */
	if( FracDay > 0.5)
		FracDay -= 0.5;
	else
		FracDay += 0.5;
	JD0HrUT = JD - FracDay;

	/* Astronomical Formulae for Calculators, by Jean Meeus, pg 39 */
	T = (JD0HrUT - 2415020.)/36525.;
	Sid0HrUT = 6.6460656 + 2400.051262*T + 0.00002581*T*T;
	SidHr = fmod( FracDay*SidRate*24. + Sid0HrUT - LongitudeDeg/15., 24);
}

void GetHMSH( long TotalHundSec, struct HMSH* V)
{
	long HrHundSec;
	long MinHundSec;
	long SecHundSec;

	if( TotalHundSec < 0)
	{
		TotalHundSec = -TotalHundSec;
		V->Sign = Minus;
	}
	else
		V->Sign = Plus;

	V->Hr = (int) ( TotalHundSec/360000L);
	HrHundSec = (long) V->Hr*360000L;
	V->Min = (int) ((TotalHundSec - HrHundSec)/6000);
	MinHundSec = (long) V->Min*6000;
	V->Sec = (int) ((TotalHundSec - HrHundSec - MinHundSec)/100);
	SecHundSec = (long) V->Sec*100;
	V->HundSec = (int) (TotalHundSec - HrHundSec - MinHundSec - SecHundSec);
}

void DisplayHMSH( const struct HMSH V)
{
	printf( "%2d:%02d:%02d.%02d", V.Hr, V.Min, V.Sec, V.HundSec);
}

void StrBufHMSH( const struct HMSH V)
{
	sprintf( StrBuf, "%2d:%02d:%02d.%02d", V.Hr, V.Min, V.Sec, V.HundSec);
}

void DisplayHMST( const struct HMSH V)
{
	printf( "%2d:%02d:%02d.%1d", V.Hr, V.Min, V.Sec, (V.HundSec + 5)/10);
}

void DisplayHMS( const struct HMSH V)
{
	int Sec, Min, Hr;

	Sec = (int)((double)V.Sec+(double)V.HundSec/100+.5);
	Min = V.Min;
	Hr = V.Hr;
	if( Sec >= 60)
	{
		Sec = 0;
		Min++;
		if( Min >= 60)
		{
			Min = 0;
			Hr++;
			if( Hr >= 24)
				Hr = 0;
		}
	}
	printf( "%2d:%02d:%02d", Hr, Min, Sec);
}

void StrBufHMS( const struct HMSH V)
{
	int Sec, Min, Hr;

	Sec = (int)((double)V.Sec+(double)V.HundSec/100+.5);
	Min = V.Min;
	Hr = V.Hr;
	if( Sec >= 60)
	{
		Sec = 0;
		Min++;
		if( Min >= 60)
		{
			Min = 0;
			Hr++;
			if( Hr >= 24)
				Hr = 0;
		}
	}
	sprintf( StrBuf, "%2d:%02d:%02d", Hr, Min, Sec);
}

void DisplaySHMS( const struct HMSH V)
{
	char Sign;

	if( V.Sign == Plus)
		Sign = '+';
	else
		Sign = '-';
	printf( "%c%2d:%02d:%02d", Sign, V.Hr, V.Min, (int)((double)V.Sec+(double)V.HundSec/100+.5));
}

void StrBufSHMS( const struct HMSH V)
{
	char Sign;

	if( V.Sign == Plus)
		Sign = '+';
	else
		Sign = '-';
	sprintf( StrBuf, "%c%2d:%02d:%02d", Sign, V.Hr, V.Min, V.Sec);
}

void DisplaySHMSH( const struct HMSH V)
{
	char Sign;

	if( V.Sign == Plus)
		Sign = '+';
	else
		Sign = '-';
	printf( "%c%2d:%02d:%02d.%02d", Sign, V.Hr, V.Min, V.Sec, V.HundSec);
}

void StrBufSHMSH( const struct HMSH V)
{
	char Sign;

	if( V.Sign == Plus)
		Sign = '+';
	else
		Sign = '-';
	sprintf( StrBuf, "%c%2d:%02d:%02d.%02d", Sign, V.Hr, V.Min, V.Sec, V.HundSec);
}

void CalcRadFromHMSH( double* R, struct HMSH V)
{
	*R = (double) V.Hr*HrToRad + (double) V.Min*MinToRad +
	(double) V.Sec*SecToRad + (double) V.HundSec*HundSecToRad;

	if( V.Sign == Minus)
		*R = -*R;
}

void CalcRadFromDMS( double* R, struct DMS V)
{
	*R = (double) V.Deg*DegToRad + (double) V.Min*ArcminToRad +
	 (double) V.Sec*ArcsecToRad;
	if( V.Sign == Minus)
		*R = -*R; 
}

/*
void TestTimes( void)
{
	printf( "\n\n\nTest of astronomical times code, press any key to stop...\n");
	printf( "\nJulian Date %f   Sidereal Time ", JD);
	VidMemXY.X = wherex();
	VidMemXY.Y = wherey();
	while( !KeyStroke)
		if( NewSidT())
		{
			GetHMSH( RadToHundSec*SidT + 0.5, &SidTimeHMSH);
			gotoxy( VidMemXY.X, VidMemXY.Y);
			DisplayHMSH( SidTimeHMSH);
		}
	getch();

	if( CMOS_RTC_Access)
		printf( "\nCMOS RTC access via bios interrupt:");
	else
		printf( "\nCMOS RTC access via direct port calls:");

	printf( "\nTest of CMOS RTC access, press any key to stop...\n");
	VidMemXY.X = wherex();
	VidMemXY.Y = wherey();
	while( !KeyStroke)
	{
		SetDOSToCMOS_RTC_f_ptr();
		gotoxy( VidMemXY.X, VidMemXY.Y);
		DisplayCMOSTimeDate();
	}
	getch();
	NewLine;
	ContMsgRoutine();
}
*/

/* coordinates... */

void ValidRa( struct Position* Pos)
{
	BoundsOneRev( &Pos->Ra);
}

void ValidHA( struct Position* Pos)
{
	BoundsHalfRev( &Pos->HA);
}

void ValidAz( struct Position* Pos)
{
	BoundsOneRev( &Pos->Az);
}

void SetCoordDeg( struct Position* Pos, const double RaDeg, const double DecDeg,
const double AltDeg, const double AzDeg, const double SidTDeg)
{
	Pos->Ra = RaDeg*DegToRad;
	Pos->Dec = DecDeg*DegToRad;
	Pos->Alt = AltDeg*DegToRad;
	Pos->Az = AzDeg*DegToRad;
	Pos->SidT = SidTDeg*DegToRad;
}

void ShowCoord( struct Position* Pos)
{
	printf( "\ncoordinates in degrees are:\n");
	printf( " Ra: %7.3f", Pos->Ra * RadToDeg);
	printf( "  Dec: %7.3f", Pos->Dec * RadToDeg);
	printf( "  Alt: %7.3f", Pos->Alt * RadToDeg);
	printf( "  Az: %7.3f", Pos->Az * RadToDeg);
	printf( "  SidT: %7.3f", Pos->SidT * RadToDeg);
}

void FShowCoord( FILE* Output, char* Name, struct Position* Pos)
{
	fprintf( Output, "\nfor position %s: init=%s; (in deg)", Name, Pos->Init?"yes":"no");
	if( Pos->Init)
	{
		fprintf( Output, " Ra: %7.3f", Pos->Ra * RadToDeg);
		fprintf( Output, "  Dec: %7.3f", Pos->Dec * RadToDeg);
		fprintf( Output, "  Alt: %7.3f", Pos->Alt * RadToDeg);
		fprintf( Output, "  Az: %7.3f", Pos->Az * RadToDeg);
		fprintf( Output, "  SidT: %7.3f", Pos->SidT * RadToDeg);
	}
}

void GetDMS( long TotalSec, struct DMS* V)
{
	if( TotalSec < 0)
	{
		TotalSec = -TotalSec;
		V->Sign = Minus;
	}
	else
		V->Sign = Plus;
	V->Deg = (int) (TotalSec/3600);
	V->Min = (int) ((TotalSec - (long) (V->Deg) * 3600)/60);
	V->Sec = (int) (TotalSec - V->Deg * 3600 - V->Min * 60);
}

void DisplayDMS( const struct DMS D)
{
	char Sign;

	if( D.Sign)
		Sign = '+';
	else
		Sign = '-';
	printf( "%c%2d:%02d:%02d", Sign, D.Deg, D.Min, D.Sec);
}

void StrBufDMS( const struct DMS D)
{
	char Sign;

	if( D.Sign)
		Sign = '+';
	else
		Sign = '-';
	sprintf( StrBuf, "%c%2d:%02d:%02d", Sign, D.Deg, D.Min, D.Sec);
}

void VidMemDeg( const double Deg)
{
	sprintf( StrBuf, "%7.3f", Deg * RadToDeg);
	WriteStrBufToScreen_f_ptr();
}

void VidMemRaHMS( struct Position* Pos)
{
	double R;

	if( Pos->Ra < 0)
		R = -.5;
	else
		R = .5;

	GetHMSH( RadToHundSec*Pos->Ra + R, &Pos->RaHMSH);
	StrBufHMS( Pos->RaHMSH);
	WriteStrBufToScreen_f_ptr();
}

void VidMemRaHMSH( struct Position* Pos)
{
	double R;

	if( Pos->Ra < 0)
		R = -.5;
	else
		R = .5;

	GetHMSH( RadToHundSec*Pos->Ra + R, &Pos->RaHMSH);
	StrBufHMSH( Pos->RaHMSH);
	WriteStrBufToScreen_f_ptr();
}

void VidMemRaSHMS( struct Position* Pos)
{
	double R;

	if( Pos->Ra < 0)
		R = -.5;
	else
		R = .5;

	GetHMSH( RadToHundSec*Pos->Ra + R, &Pos->RaHMSH);
	StrBufSHMS( Pos->RaHMSH);
	WriteStrBufToScreen_f_ptr();
}

void VidMemHAHMS( struct Position* Pos)
{
	double R;

	if( Pos->HA < 0)
		R = -.5;
	else
		R = .5;

	GetHMSH( RadToHundSec*Pos->HA + R, &Pos->HAHMSH);
	StrBufSHMS( Pos->HAHMSH);
	WriteStrBufToScreen_f_ptr();
}

void VidMemHAHMSH( struct Position* Pos)
{
	double R;

	if( Pos->HA < 0)
		R = -.5;
	else
		R = .5;

	GetHMSH( RadToHundSec*Pos->HA + R, &Pos->HAHMSH);
	StrBufSHMSH( Pos->HAHMSH);
	WriteStrBufToScreen_f_ptr();
}

void VidMemDecDMS( struct Position* Pos)
{
	double R;

	if( Pos->Dec < 0)
		R = -.5;
	else
		R = .5;

	GetDMS( RadToArcsec*Pos->Dec + R, &Pos->DecDMS);
	StrBufDMS( Pos->DecDMS);
	WriteStrBufToScreen_f_ptr();
}

void VidMemSidT( struct Position* Pos)
{
	double R;

	if( Pos->SidT < 0)
		R = -.5;
	else
		R = .5;

	GetHMSH( RadToHundSec*Pos->SidT + R, &Pos->SidTimeHMSH);
	StrBufHMS( Pos->SidTimeHMSH);
	WriteStrBufToScreen_f_ptr();
}

/* parallel port */

/* Normally, a PC can have up to 3 parallel printer ports - LPT1, LPT2 & LPT3.
Their (16-bit) base addresses in the processor's I/O space are loaded into memory when the machine
is booted, starting at address 408 Hex.

A well-written program requiring direct access to any of these ports should endeavour to find out
which are present and where at run-time (rather than use constant declarations)

Due to 80x86 processors being " little-endian", the addresses are stored as follows:

408 409  40a 40b  40c 40d
LPT1     LPT2     LPT3
low/high low/high low/high
byte     byte     byte

For example, the following values -

408 409 40a 40b 40c 40d
78  03  78  02  00  00

mean that LPT1 is at 378 Hex, LPT2 is at 278 Hex, and LPT3 is not present.

8 bits out (data port): base address portid lpt1 = 0x378 (decimal 888), lpt2 = 0x278 (decimal 632),
monochrome video card = 0x3BC (decimal 956);

25 - pin connector pins 2 (least significant) through 9 (most significant),
logical low in produces logical low out;

5 bits in (status port): base address + 1, portid 0x379 (or try 0x279 or 0x3BD):

	8		16		32		64		128	bit
	H		H		H		H		H		in
	15		13		12		10		11		25 pin connector pin #
	L		H		H		H		L		result, eg, inverted for bit 128;

	module logic rectifies the inverted bit;

4 bits in or out (control port): base address + 2, portid 0x37A (or try 0x27A or 0x3BE):

	1       2       4       8    		bit
	H       H       H       H        in or out
	1       14      16      17       25 pin connector pin #
	L       L       H       L        result, eg, inverted for all bits except for bit 4;

	module logic rectifies the inverted bits;

more info on the control port (4 bits I/O):
bit 16 = irq enable, 32 = bi-dir enable for data port;
these pins have two states, 0V and open collector or high impedence; to read, it may be necessary
to put the pins in open collector mode by outputting xxxx0100 (turn on all pins, respecting the
inversions), now the input on the pins if pulled low can be read;

putting I/O port into high impedance, all pins respond to grounding, otherwise,
all input pins respond to +5VDC except for pin 15, only pin 15 responds to grounding;

remaining pin connections: 25 pin connector pins 18 through 25 are grounds; */

void InitPPort( void)
{
	int HoldPPortAddr = PPortAddr;

	/* if PPortAddr <=3, treat it as a lpt num */
	if( PPortAddr > 0 && PPortAddr <= 3)
		if( !(PPortAddr = GetPPortAddr( PPortAddr)))
		{
			sprintf( StrBuf, "Bad lpt:%01d", HoldPPortAddr);
			BadExit( StrBuf);
		}
	PPortAddrOutByte = PPortAddr;
	PPortAddrInNibble = PPortAddr + 1;
	PPortAddrInOutNibble = PPortAddr + 2;
	/* make the 8 output parallel port pins logical low */
	OutValue = Off;
	if( InvertOutput)
		OutValue = ~OutValue;
	OutByte();
	/* always set the value, then call the function, that way, the value can always
	be relied upon to hold the last outputted state */
	BiDirOutNibbleValue = 0;
	BiDirOutNibble();
}

void ClosePPort( void)
{
	OutValue = Off;
	if( InvertOutput)
		OutValue = ~OutValue;
	OutByte();
}

unsigned GetPPortAddr( int lptnum)
{
	/* make a far pointer with segment 0x40 and offset 0x008+, then return its value */
	return *(unsigned far *)(MK_FP( 0x40, 0x008 + (lptnum-1)*2));
}

void DisplayLpts (void)
{
	int x;

	printf( "\n\n\ndisplaying lpt ports:");
	for( x=1; x<=3; x++)
		printf( "\nlpt%1d: 0x%04x hex or %4d decimal", x, GetPPortAddr( x), GetPPortAddr( x));
	printf( "\n\n");
}

void OutByte( void)
{
	outportb( PPortAddrOutByte, OutValue);
}

void BiDirOutNibble( void)
{
	/* 4 bits out: parallel port lines 1, 14, 16, 17 */
	outportb( PPortAddrInOutNibble, (BiDirOutNibbleValue &15) ^11);
}

Byte InNibble4Bit( void)
{
	/* 4 bits in: parallel port lines 13, 12, 10, 11 using bits 16, 32, 64, 128 */
	return (inportb( PPortAddrInNibble) &240) ^128;
}

Byte InNibble5Bit( void)
{
	/* 5 bits in: parallel port lines 15, 13, 12, 10, 11 using bits 8, 16, 32, 64, 128 */
	return (inportb( PPortAddrInNibble) &248) ^136;
}

Byte BiDirInNibble( void)
{
	/* 4 bits in: parallel port lines 1, 14, 16, 17 */
	return (inportb( PPortAddrInOutNibble) &15) ^11;
}

void TestPPort( void)
{
	int Input;
	int x, y;

	printf( "\n\n\Test of parallel port:");
	DisplayLpts();
	printf( "\nusing parallel port address of %d\n", PPortAddr);
	printf( "\ntest of 8 bit output port (parallel port pins 2-9):\n");
	printf( "'h' for logical high, 'l' for logical low, 'q' to quit.\n");
	Input = ' ';
	while( Input != 'q')
	{
		Input = getch();
		if( Input == 'h' || Input == 'H')
		{
			printf( "outputting logical high...\n");
			OutValue = PortOn;
			OutByte();
		}
		else if( Input == 'l' || Input == 'L')
		{
			printf( "outputting logical low...\n");
			OutValue = PortOff;
			OutByte();
		}
	}
	OutValue = PortOff;
	OutByte();

	printf( "\ntest of 4 bit I-O port (parallel port pins 1, 14, 16, 17):\n");
	printf( "'h' for logical high, 'l' for logical low, 'q' to quit.\n");
	Input = ' ';
	while( Input != 'q')
	{
		Input = getch();
		if( Input == 'h' || Input == 'H')
		{
			printf( "outputting logical high...\n");
			BiDirOutNibbleValue = PortOn;
			BiDirOutNibble();
		}
		else if( Input == 'l' || Input == 'L')
		{
			printf( "outputting logical low...\n");
			BiDirOutNibbleValue = PortOff;
			BiDirOutNibble();
		}
	}
	BiDirOutNibbleValue = PortOff;
	BiDirOutNibble();

	printf( "\ntest of parallel input pins (press any key to quit):\n\n");
	x = wherex();
	y = wherey();
	while( !KeyStroke)
	{
		gotoxy( x, y);
		InNibbleValue = InNibble5Bit();
		BiDirInNibbleValue = BiDirInNibble();
		printf( "1:%c ", BiDirInNibbleValue&1? 'l':'H');
		printf( "10:%c ", InNibbleValue&64? 'H':'l');
		printf( "11:%c ", InNibbleValue&128? 'l':'H');
		printf( "12:%c ", InNibbleValue&32? 'H':'l');
		printf( "13:%c ", InNibbleValue&16? 'H':'l');
		printf( "14:%c ", BiDirInNibbleValue&2? 'l':'H');
		printf( "15:%c ", InNibbleValue&8? 'l':'H');
		printf( "16:%c ", BiDirInNibbleValue&4? 'H':'l');
		printf( "17:%c", BiDirInNibbleValue&8? 'l':'H');
	}
	getch();
	NewLine;
	ContMsgRoutine();
}

/* refraction makes an object appear higher in the sky than it really is when close to the horizon;
if scope aimed at horizon, then it will actually be seeing an object -34.5' below horizon;
translate scope->sky as 0 -> -34.5 and sky->scope as -34.5 -> 0
causes tracking rate to slow down the closer the scope is to the horizon;
refraction handled by routines that translate between scope's position (Cfg.ServoParms[].ActualPosition) 
   and sky altitude (Cfg.Current altaz coord);
 
interpolate: 
1. find points that the angle fits between
   ex: angle of 10 has end point of R[10][0] and beginning point of R[9][0]
2. get position between end points
   position = (a-bp)/(ep-bp) 
   ex: a=1, bp=2, ep=0
		 position = (1-2)/(0-2) = .5
3. scope->sky refraction = amount of refraction at beginning point + 
                           position * (amount of refract at end point - amount of refract at beg point)
   r = br + p*(er-br), r = br + (a-bp)/(ep-bp)*(er-br), r = br + (a-bp)*(er-br)/(ep-bp)
   ex: br=18, er=34.5
       r = 18 + .5*(34.5-18) = 26.25 arcmin
4. corrected angle = angle - refraction
   ex: c = a-r = c = 60 arcmin - 26.25 arcmin = 33.75 arcmin

to reverse (sky->scope): have corrected angle of c, find altitude of a;
   ex: c = 60 arcmin - 26.25 arcmin = 33.75 arcmin, solve for a:
1. c = a - r, a = c + r, a = (c+br)(ep-bp) + (a-bp)*(er-br)/(ep-bp),
   a(ep-bp) = c*ep - c*bp + br*ep - br*bp + a*er - a*br - bp*er + bp*br,
   a*ep - a*bp - a*er + a*br = c*ep - c*bp + br*ep - br*bp - bp*er + bp*br,
   a*(ep-bp-er+br) = bp(-c-br-er+br) + ep(c+br),
   a = (bp(-c-er) + ep(c+br)) / (ep-bp-er+br),   
   ex: convert all units to armin...
       c=33.75
       br=18
       er=34.5
       bp=120
       ep=0
   a = (120(-33.75-34.5) + 0) / (0-120-34.5+18),
   a = 120*-68.25 / -136.5,
	a = 60

(if refract added to angle, eg, corrected angle = angle + refraction, ie, c=a+r,
 then use the following to back out the correction:
 to reverse: have corrected angle of ca, find altitude of a
	 ex: ca = 1deg + 26.25arcmin = 86.25arcmin, solve for a
 1. ca = a + r, a = ca - r, a = ca - br - (a-bp)*(er-br)/(ep-bp),
	 a*(ep-bp)+(a-bp)*(er-br) = (ca-br)*(ep-bp),
	 a*ep-a*bp+a*er-a*br-bp*er+bp*br = ca*ep-ca*bp-br*ep+br*bp,
	 a*(ep-bp+er-br) = ca*ep-ca*bp-br*ep+br*bp+bp*er-bp*br,
	 a*(ep-bp+er-br) = ca*(ep-bp)-br*ep+bp*er,
	 a = (ca*(ep-bp)-br*ep+bp*er) / (ep-bp+er-br),
	 ex: convert all units to armin...
		  a = 86.25-18-(34.5-18)(60-120)/(0-120) = 86.25-18-8.25 = 60 (from 1st line)
		  a = (86.25*(0-120)-18*0+120*34.5)/(0-120+34.5-18) = (-10350+4140)/(-103.5) = 60
)
*/

void InitRefract( void)
{
	/* table of refraction per altitude angle:
	R[][0] is altitude angle in degrees
	R[][1] is refraction in arcminutes of corresponding altitude angles */
	// to compute scope->sky refraction, subtract interpolation of R table from altitude
	// to compute sky->scope refraction, add interpolation of R table to altitude
	R[0][0] = 90.;   R[0][1] = 0.;
	R[1][0] = 60.;   R[1][1] = .55;
	R[2][0] = 30.;   R[2][1] = 1.7;
	R[3][0] = 20.;   R[3][1] = 2.6;
	R[4][0] = 15.;   R[4][1] = 3.5;
	R[5][0] = 10.;   R[5][1] = 5.2;
	R[6][0] =  8.;   R[6][1] = 6.4;
	R[7][0] =  6.;   R[7][1] = 8.3;
	R[8][0] =  4.;   R[8][1] = 11.5;
	R[9][0] =  2.;   R[9][1] = 18;
	R[10][0] = 0.;   R[10][1] = 34.5;
	// to allow for sky->scope interpolation when scope->sky results in negative elevation
	R[11][0] = -1.;  R[11][1] = 42.75;

	Refract = 0;
};

// this function calcs refraction at a particular aimed altitude, or translate scope->sky coordinates
// ie, at the horizon, refraction will be 34.5 arcmin, resulting in a lower aimed at sky coordinate
void CalcRefractScopeToSky( const double Alt)
{
	int Ix;
	double a;
	double bp;
	double ep;
	double br;
	double er;

	/* Alt is in radians; convert to degrees for use with refraction table */
	a = Alt*RadToDeg;
	if( a > 90)
		a = 90;
	for( Ix = 0; a <= R[Ix][0] && Ix < MaxRefractIx; Ix++)
		;
	bp = R[Ix-1][0];
	ep = R[Ix][0];
	br = R[Ix-1][1];
	er = R[Ix][1];

	Refract = br + (a-bp)*(er-br)/(ep-bp);
	// table gives values in arcmin, so convert to radians
	Refract *= ArcminToRad;
}

// this function calcs refraction to remove from an already compensated altitude, eg translate sky->scope coordinates
// ie, at 34.5 arcmin below horizon, value to remove is 34.5 arcmin, resulting in a higher scope coordinate
void CalcRefractSkyToScope( const double Alt)
{
	int Ix;
	double a;
	double bp;
	double ep;
	double br;
	double er;
	double a1;

	/* Alt is in radians; convert to degrees for use with refraction table */
	a = Alt*RadToDeg;
	if( a > 90)
		a = 90;
	for( Ix = 0; a < R[Ix][0] && Ix < MaxRefractIx; Ix++)
		;
	bp = R[Ix-1][0];
	ep = R[Ix][0];
	br = R[Ix-1][1];
	er = R[Ix][1];

	/* convert deg to arcmin */
	a *= 60.;
	bp *= 60;
	ep *= 60;

	// 'a' = corrected altitude
	a1 = (bp*(-a-er) + ep*(a+br)) / (ep-bp-er+br);
	// table gives values in arcmin, so convert to radians
	Refract = a1*ArcminToRad - Alt;
}

/*
void TestRefract()
{
	double InputAlt;
	double SkyElevation;

	printf( "\n\nRefract test\n");
	printf( "input telescope elevation above horizon in degrees: ");
	GetDouble( &InputAlt);
	CalcRefractScopeToSky( InputAlt*DegToRad);
	SkyElevation = InputAlt*DegToRad - Refract;
	printf( "scope->sky refraction is %f armin (%f arcsec)\n", Refract*RadToArcmin, Refract*RadToArcsec);
	printf( "scope actually sees object of elevation %f degrees\n", SkyElevation*RadToDeg);
	CalcRefractSkyToScope( SkyElevation);
	printf( "sky->scope refraction is %f armin (%f arcsec)\n", Refract*RadToArcmin, Refract*RadToArcsec);
	printf( "giving scope elevation of %f degrees\n", (SkyElevation + Refract)*RadToDeg);
	printf( "end of Refract class test\n\n");
}
*/

/* egavga routines */

void InitGraphics( void)
{
	int gdriver;
	int gmode;
	int errorcode;


	gettext( 1, 1, MaxX, MaxY, WinBuffer);
	clrscr();

	/* request autodetection */
	gdriver = DETECT;

	/* create egavga.obj file by changing to \bc45\bgi then execute bgiobj.exe egavga which creates
	the egavga.obj file
	copy the egavga.obj file to the working subdirectory
	add graphics.h to source file
	add \bc45\lib\graphics.lib to project
	add egavga.obj to project */

	if( registerbgidriver( EGAVGA_driver) < 0)
		BadExit( "could not register egavga driver");

	/* init graphics and local vars */
	initgraph( &gdriver, &gmode, "");

	/* read result of initialization */
	errorcode = graphresult();
	if( errorcode != grOk)
	{
		BadExit( strcat( "initgraph error: ", grapherrormsg( errorcode)));
		printf( "Press any key to halt:");
		getch();
		/* terminate with err code */
		exit( 1);
	}
	maxx = getmaxx();
	maxy = getmaxy();

	/* putpixel( x, y, color); */
}

void CloseGraphics( void)
{
	closegraph();
	clrscr();
	window( 1, 1, MaxX, MaxY);
	puttext( 1, 1, MaxX, MaxY, WinBuffer);
	_setcursortype( _NORMALCURSOR);
}

/* mouse functions */

void InitMouse( void)
{
	ResetMouse();
}

void CallMouse( void)
{
	union REGS inregs, outregs;

	inregs.x.ax = inax;
	inregs.x.bx = inbx;
	inregs.x.cx = incx;
	inregs.x.dx = indx;

	int86( MOUSE_INTERRUPT, &inregs, &outregs);

	outax = outregs.x.ax;
	outbx = outregs.x.bx;
	outcx = outregs.x.cx;
	outdx = outregs.x.dx;
}

void GetDefaultMouseSensitivity( void)
{
	inax = GET_MOUSE_SENSITIVITY;
	CallMouse();

	HorizMickeySensitivity = outbx;
	VertMickeySensitivity = outcx;
	ThresholdDblSpeed = outdx;
}

void DecodeMousePosition( void)
{
	MouseXGraph = outcx;
	MouseYGraph = outdx;
	MouseXText = MouseXGraph/8 + 1;
	MouseYText = MouseYGraph/8 + 1;
}

/* resets mouse to default values:
mouse is positioned to screen center
mouse cursor is reset and hidden
no interrupts are enabled (mask = 0)
double speed threshold set to 64 mickeys per second
horizontal mickey to pixel ratio (8 to 8)
vertical mickey to pixel ratio (16 to 8)
max width and height are set to maximum for video mode */
void ResetMouse( void)
{
	inax = RESET_MOUSE;
	CallMouse();
	if(outax != SUCCESSFULL_MOUSE_RESET)
		BadExit( "could not find mouse driver");
}

void LowMouseSensitivity( void)
{
	GetDefaultMouseSensitivity();

	inax = SET_MOUSE_SENSITIVITY;
	inbx = 4;
	incx = 4;
	indx = 64;
	CallMouse();
}

void ResetMouseSensitivity( void)
{
	inax = SET_MOUSE_SENSITIVITY;
	inbx = HorizMickeySensitivity;
	incx = VertMickeySensitivity;
	indx = ThresholdDblSpeed;
	CallMouse();
}

void DisplayMouse( void)
{
	inax = DISPLAY_MOUSE;
	CallMouse();
}

void HideMouse( void)
{
	inax = HIDE_MOUSE;
	CallMouse();
}

void GetMousePosition( void)
{
	inax = GET_MOUSE_POSITION;
	CallMouse();
	DecodeMousePosition();
	MouseLeftButtonDown = outbx & 1;
	MouseRightButtonDown = outbx & 2;
}

void GetMousePositionRelative( void)
{
	inax = GET_MOUSE_POSITION_RELATIVE;
	CallMouse();
	MouseXMickeyRelative = outcx;
	MouseYMickeyRelative = outdx;
}

void SetMousePosition(int Horiz, int Vert)
{
	inax = SET_MOUSE_POSITION;
	incx = Horiz;
	indx = Vert;
	CallMouse();
}

void CenterMouseCursor( void)
{
	SetMousePosition( MidMouseXGraph, MidMouseYGraph);
}

int MouseLeftButtonPressCount( void)
{
	inax = GET_MOUSE_PRESS_INFO;
	inbx = LEFT_BUTTON;
	CallMouse();
	if( outbx)
		/* get position at last press */
		DecodeMousePosition();
	MouseLeftButtonDown = outax & 1;
	MouseRightButtonDown = outax & 2;
	return outbx;
}

int MouseRightButtonPressCount( void)
{
	inax = GET_MOUSE_PRESS_INFO;
	inbx = RIGHT_BUTTON;
	CallMouse();
	if( outbx)
		/* get position at last press */
		DecodeMousePosition();
	MouseLeftButtonDown = outax & 1;
	MouseRightButtonDown = outax & 2;
	return outbx;
}

int MouseLeftButtonReleaseCount( void)
{
	inax = GET_MOUSE_RELEASE_INFO;
	inbx = LEFT_BUTTON;
	CallMouse();
	if( outbx)
		/* get position at last press */
		DecodeMousePosition();
	MouseLeftButtonDown = outax & 1;
	MouseRightButtonDown = outax & 2;
	return outbx;
}

int MouseRightButtonReleaseCount( void)
{
	inax = GET_MOUSE_RELEASE_INFO;
	inbx = RIGHT_BUTTON;
	CallMouse();
	if( outbx)
		/* get position at last press */
		DecodeMousePosition();
	MouseLeftButtonDown = outax & 1;
	MouseRightButtonDown = outax & 2;
	return outbx;
}

/*
void TestMouse( void)
{
	clrscr();
	printf( "Test of mouse module: press any key to exit...\n");
	while( !KeyStroke)
	{
		DisplayMouse();
		if( MouseLeftButtonPressCount())
			printf( "\nleft button was pressed...");
		else if( MouseRightButtonPressCount())
			printf( "\nright button was pressed...");
		else
		{
			GetMousePositionRelative();
			printf( "\nrelative mouse position: %d %d", MouseXMickeyRelative,
			MouseYMickeyRelative);
			delay( 1000);
		}
	}
	getch();
}
*/
