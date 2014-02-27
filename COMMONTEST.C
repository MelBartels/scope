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
#include "headertest.h"

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

void NULL_FUNCTION( void) {}

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

