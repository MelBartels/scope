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

void DisplayInitStatusOnScreen( void)
{
	if( Three.Init)
		sprintf( StrBuf, "3");
	else
		if( Two.Init)
			sprintf( StrBuf, "2");
		else
			if( One.Init)
				sprintf( StrBuf, "1");
			else
				sprintf( StrBuf, "0");
	VidMemXY = DisplayXY[DisplayInitStatus];
	TextAttr = DisplayText;
	WriteStrBufToScreen_f_ptr();
}

void DisplayInit( void)
{
	int StartX = InitFrame.Left + 1;
	int StartY = InitFrame.Top + 1;
	int AltX, AzX, RaX, DecX, SidTX;

	CheckForPostInitVars();

	AltX = StartX + 3;
	AzX = AltX + SizeofDegX + 1;
	RaX = AzX + SizeofDegX + 1;
	DecX = RaX + SizeofHMSX + 1;
	SidTX = DecX + SizeofDMSX + 1;

	WriteWindow( InitFrame);

	TextAttr = DefaultText;
	VidMemXY.Y = StartY;
	VidMemXY.X = AltX;

	sprintf( StrBuf, InitText[InitState-1]);
	WriteStrBufToScreen_f_ptr();

	if( One.Init)
	{
		VidMemXY.Y++;
		VidMemXY.X = AltX;
		sprintf( StrBuf, "Alt");
		WriteStrBufToScreen_f_ptr();
		VidMemXY.X = AzX;
		sprintf( StrBuf, "Az");
		WriteStrBufToScreen_f_ptr();
		VidMemXY.X = RaX;
		sprintf( StrBuf, "Ra");
		WriteStrBufToScreen_f_ptr();
		VidMemXY.X = DecX;
		sprintf( StrBuf, "Dec");
		WriteStrBufToScreen_f_ptr();
		VidMemXY.X = SidTX;
		sprintf( StrBuf, "SidT");
		WriteStrBufToScreen_f_ptr();

		VidMemXY.Y++;
		VidMemXY.X = StartX;
		sprintf( StrBuf, "1:");
		WriteStrBufToScreen_f_ptr();
		VidMemXY.X = AltX;
		VidMemDeg( One.Alt);
		VidMemXY.X = AzX;
		VidMemDeg( One.Az);
		VidMemXY.X = RaX;
		VidMemRaHMS( &One);
		VidMemXY.X = DecX;
		VidMemDecDMS( &One);
		VidMemXY.X = SidTX;
		VidMemSidT( &One);
	}
	else
	{
		VidMemXY.Y++;
		VidMemXY.X = StartX;
		sprintf( StrBuf, "no init one");
		WriteStrBufToScreen_f_ptr();
	}
	if( Two.Init)
	{
		VidMemXY.Y++;
		VidMemXY.X = StartX;
		sprintf( StrBuf, "2:");
		WriteStrBufToScreen_f_ptr();
		VidMemXY.X = AltX;
		VidMemDeg( Two.Alt);
		VidMemXY.X = AzX;
		VidMemDeg( Two.Az);
		VidMemXY.X = RaX;
		VidMemRaHMS( &Two);
		VidMemXY.X = DecX;
		VidMemDecDMS( &Two);
		VidMemXY.X = SidTX;
		VidMemSidT( &Two);
	}
	else
	{
		VidMemXY.Y++;
		VidMemXY.X = StartX;
		sprintf( StrBuf, "no init two");
		WriteStrBufToScreen_f_ptr();
	}
	if( Three.Init)
	{
		VidMemXY.Y++;
		VidMemXY.X = StartX;
		sprintf( StrBuf, "3:");
		WriteStrBufToScreen_f_ptr();
		VidMemXY.X = AltX;
		VidMemDeg( Three.Alt);
		VidMemXY.X = AzX;
		VidMemDeg( Three.Az);
		VidMemXY.X = RaX;
		VidMemRaHMS( &Three);
		VidMemXY.X = DecX;
		VidMemDecDMS( &Three);
		VidMemXY.X = SidTX;
		VidMemSidT( &Three);
	}
	else
	{
		VidMemXY.Y++;
		VidMemXY.X = StartX;
		sprintf( StrBuf, "no init three");
		WriteStrBufToScreen_f_ptr();
	}
	VidMemXY.Y++;
	VidMemXY.X = StartX;
	sprintf( StrBuf, "Z1: %3.3f   Z2: %3.3f   Z3: %3.3f", Z1*RadToDeg,
	Z2*RadToDeg, Z3*RadToDeg);
	WriteStrBufToScreen_f_ptr();

	if( Two.Init)
	{
		VidMemXY.Y++;
		VidMemXY.X = StartX;
		sprintf( StrBuf, "alt offset from Init One, Two = %3.2f deg", AltOffsetFromCalcPostInitVars*RadToDeg);
		WriteStrBufToScreen_f_ptr();

		VidMemXY.Y++;
		VidMemXY.X = StartX;
		sprintf( StrBuf, "from telescope's perspective:");
		WriteStrBufToScreen_f_ptr();

		VidMemXY.Y++;
		VidMemXY.X = StartX;
		sprintf( StrBuf, "latitude (zenith)=%3.2f, (equat pole)=%3.2f, longitude=%3.2f deg",
					LatScopeZenith*RadToDeg, LatEquatPole*RadToDeg, LongitudeRad*RadToDeg);
		WriteStrBufToScreen_f_ptr();

		VidMemXY.Y++;
		VidMemXY.X = StartX;
		sprintf( StrBuf, "hour angle offset=%3.2f, azimuth offset=%3.2f deg",
					HAOff*RadToDeg, AzOff*RadToDeg);
		WriteStrBufToScreen_f_ptr();

		VidMemXY.Y++;
		VidMemXY.X=StartX;
		sprintf( StrBuf, "zenith pointing error=%3.2f, dir(east=90,south=180)=%3.2f deg",
					ZenithOffset.A*RadToDeg, ZenithOffset.Z*RadToDeg);
		WriteStrBufToScreen_f_ptr();

		VidMemXY.Y++;
		VidMemXY.X=StartX;
		sprintf( StrBuf, "polar point error (equat pole)= %3.2f, dir(east=90,south=180)=%3.2f deg",
					PolarAlignEquatPole.A*RadToDeg, PolarAlignEquatPole.Z*RadToDeg);
		WriteStrBufToScreen_f_ptr();

		VidMemXY.Y++;
		VidMemXY.X=StartX;
		sprintf( StrBuf, "polar point error (altaz pole)= %3.2f, dir(east=90,south=180)=%3.2f deg",
					PolarAlignAltazPole.A*RadToDeg, PolarAlignAltazPole.Z*RadToDeg);
		WriteStrBufToScreen_f_ptr();
	}
	gotoxy( StartX + 6, VidMemXY.Y += 3);
	ContMsgRoutine();
	RemoveWindow( InitFrame);
}

/* use to enter init's with different SidT's */
void EnterInit( const int InitToDo)
{
	struct XY HoldXY;
	/* temporary coordinates */
	struct Position Temp;

	WriteWindow( MsgFrame);

	HoldXY.X = MsgFrame.Left + 2;
	HoldXY.Y = MsgFrame.Top + 1;
	Temp = Current;
	gotoxy( HoldXY.X, HoldXY.Y);
	printf( "Please enter init %1d ", InitToDo);
	HoldXY.X = wherex();
	gotoxy( HoldXY.X, HoldXY.Y);
	if( InputEquat( &Current, HoldXY.X, HoldXY.Y))
	{
		gotoxy( HoldXY.X += 17, HoldXY.Y);
		if( InputAltaz( &Current, HoldXY.X, HoldXY.Y))
		{
			HoldXY.Y += 3;
			gotoxy( HoldXY.X, HoldXY.Y++);
			printf( "SidHr  ");
			if( GetInt( &Current.SidTimeHMSH.Hr))
			{
				gotoxy( HoldXY.X, HoldXY.Y++);
				printf( "SidMin ");
				if( GetInt( &Current.SidTimeHMSH.Min))
				{
					gotoxy( HoldXY.X, HoldXY.Y++);
					printf( "SidSec ");
					if( GetInt( &Current.SidTimeHMSH.Sec))
					{
						Current.SidTimeHMSH.HundSec = 0;
						Current.SidTimeHMSH.Sign = Plus;
						CalcRadFromHMSH( &Current.SidT, Current.SidTimeHMSH);
						/* KBEventInitMatrix calls InitMatrix() then calls DisplayInitStatusOnScreen(),
						but this window covers up the init display on the screen, so wait until window
						removed before displaying init status on screen */
						strcpy( WhyInit, WHY_INIT_MANUAL_ENTRY);
						InitMatrix( InitToDo);
					}
				}
			}
		}
	}
	/* restore Alt, Az */
	Current.Alt = Temp.Alt;
	Current.Az = Temp.Az;

	RemoveWindow( MsgFrame);
	DisplayInitStatusOnScreen();
}

void KillInits( void)
{
	WriteWindow( MsgFrame);
	VidMemXY.X = MsgFrame.Left + 3;
	VidMemXY.Y = MsgFrame.Top + 2;
	gotoxy( VidMemXY.X, VidMemXY.Y);
	printf( "Sure you want to kill all initializations? ");
	GetResponseWithLX200Check();
	RemoveWindow( MsgFrame);
	if( Response == 'Y' || Response == 'y' || Response == Return)
	{
		One.Init = Two.Init = Three.Init = No;
		TrackFlag = No;
		WriteTrackStatus();
		VidMemXY = DisplayXY[DisplayCurrentRa];
		sprintf( StrBuf, "          ");
		WriteStrBufToScreen_f_ptr();
		VidMemXY = DisplayXY[DisplayCurrentDec];
		sprintf( StrBuf, "          ");
		WriteStrBufToScreen_f_ptr();
	}
	DisplayInitStatusOnScreen();
}

void Seterrscale( void)
{
	/* so that max error appears to be 30 deg long on graph; 1 deg error	bar cannot exceed 180 deg */
	errscale = 30 * pixelsdeg / (MaxPointErr * RadToDeg);
	if( errscale > 180 * pixelsdeg)
		errscale = 180 * pixelsdeg;
}

void PlotAnalysisErrorsOnGrid( const int xoffset, const int yoffset)
{
	int x, y;
	int start, end;

	CurrentLinkPos = FirstLinkPos;
	while( CurrentLinkPos != NULL)
	{
		Abegin = 180*pixelsdeg - (int) (CurrentLinkPos->P.Alt * pixelsdeg * RadToDeg);
		Adif = (int) (CurrentLinkPos->AZErr.A * pixelsdeg * RadToDeg * errscale);
		Aend = Abegin - Adif;
		Zbegin = (int) (CurrentLinkPos->P.Az * pixelsdeg * RadToDeg);
		Zdif = (int) (CurrentLinkPos->AZErr.Z * pixelsdeg * RadToDeg * errscale);
		Zend = Zbegin + Zdif;
		if( Adif <= 0)
		{
			start = Abegin;
			end = Aend;
		}
		else
		{
			start = Aend;
			end = Abegin;
		}
		for( y = start; y <= end; y++)
			putpixel( xoffset+Zbegin, yoffset+y, altcolor);
		if( Zdif >= 0)
		{
			start = Zbegin;
			end = Zend;
		}
		else
		{
			start = Zend;
			end = Zbegin;
		}
		for( x = start; x <= end; x++)
			putpixel( xoffset+x, yoffset+Abegin, azcolor);
		CurrentLinkPos = CurrentLinkPos->NextLinkPos;
	}
	/* plot the start points, doing them last to avoid being overwritten by the error bars */
	CurrentLinkPos = FirstLinkPos;
	while( CurrentLinkPos != NULL)
	{
		Abegin = 180*pixelsdeg - (int) (CurrentLinkPos->P.Alt * pixelsdeg * RadToDeg);
		Zbegin = (int) (CurrentLinkPos->P.Az * pixelsdeg * RadToDeg);
		putpixel( xoffset+Zbegin, yoffset+Abegin, highlightcolor);
		CurrentLinkPos = CurrentLinkPos->NextLinkPos;
	}
}

void PlotPMCErrorsOnGrid( const int xoffset, const int yoffset)
{
	int x, y;
	int start, end;

	CurrentLinkPMC = FirstLinkPMC;
	while( CurrentLinkPMC != NULL)
	{
		Abegin = 180*pixelsdeg - (int) (CurrentLinkPMC->AZ.A * pixelsdeg * RadToDeg);
		Adif = (int) (CurrentLinkPMC->AZErr.A * pixelsdeg * RadToDeg * errscale);
		Aend = Abegin - Adif;
		Zbegin = (int) (CurrentLinkPMC->AZ.Z * pixelsdeg * RadToDeg);
		Zdif = (int) (CurrentLinkPMC->AZErr.Z * pixelsdeg * RadToDeg * errscale);
		Zend = Zbegin + Zdif;
		if( Adif <= 0)
		{
			start = Abegin;
			end = Aend;
		}
		else
		{
			start = Aend;
			end = Abegin;
		}
		for( y = start; y <= end; y++)
			putpixel( xoffset+Zbegin, yoffset+y, altcolor);
		if( Zdif >= 0)
		{
			start = Zbegin;
			end = Zend;
		}
		else
		{
			start = Zend;
			end = Zbegin;
		}
		for( x = start; x <= end; x++)
			putpixel( xoffset+x, yoffset+Abegin, azcolor);
		CurrentLinkPMC = CurrentLinkPMC->NextLinkPMC;
	}
	/* plot the start points, doing them last to avoid being overwritten by the error bars */
	CurrentLinkPMC = FirstLinkPMC;
	while( CurrentLinkPMC != NULL)
	{
		Abegin = 180*pixelsdeg - (int) (CurrentLinkPMC->AZ.A * pixelsdeg * RadToDeg);
		Zbegin = (int) (CurrentLinkPMC->AZ.Z * pixelsdeg * RadToDeg);
		putpixel( xoffset+Zbegin, yoffset+Abegin, highlightcolor);
		CurrentLinkPMC = CurrentLinkPMC->NextLinkPMC;
	}
}

void GraphErrorBars( const int xoffset, const int yoffset)
{
	int x;

	/* plot 1 degree error bars */
	for( x = 0; x < pixelsdeg * errscale; x++)
	{
		putpixel( xoffset+x, yoffset, highlightcolor);
		putpixel( xoffset, yoffset+x, highlightcolor);
	}
}

void GraphGrid( const int xoffset, const int yoffset)
{
	double x, y;

	for( x=0; x <= pixelsdeg*360; x+=pixelsdeg*30)
		for( y=0; y <= pixelsdeg*360; y++)
			putpixel( xoffset+x, yoffset+y, gridcolor);
	for( y=0; y<=pixelsdeg*360; y+=pixelsdeg*30)
		for( x=0; x<=pixelsdeg*360; x++)
			putpixel( xoffset+x, yoffset+y, gridcolor);
}

void GraphAzLine( const int xoffset, const int yoffset)
{
	int x;

	for( x=0; x <= pixelsdeg*360; x++)
		putpixel( xoffset+x, yoffset, gridcolor);
}

void GraphAltLine( const int xoffset, const int yoffset)
{
	int y;

	for( y=0; y <= pixelsdeg*360; y++)
		putpixel( xoffset, yoffset+y, gridcolor);
}

void GraphAltOnAz( const int xoffset, const int yoffset)
{
	int y;
	int start, end;

	/* plot altitude errors vs azimuth looking for rocker base levelness */
	CurrentLinkPos = FirstLinkPos;
	while( CurrentLinkPos != NULL)
	{
		Abegin = 0;
		Adif = (int) (CurrentLinkPos->AZErr.A * pixelsdeg * RadToDeg * errscale);
		Aend = Abegin - Adif;
		if( Adif <= 0)
		{
			start = Abegin;
			end = Aend;
		}
		else
		{
			start = Aend;
			end = Abegin;
		}
		Zbegin = (int) (CurrentLinkPos->P.Az * pixelsdeg * RadToDeg);
		for( y = start; y <= end; y++)
			putpixel( xoffset+Zbegin, yoffset+y, altcolor);
		CurrentLinkPos = CurrentLinkPos->NextLinkPos;
	}
}

void GraphAzOnAz( const int xoffset, const int yoffset)
{
	int y;
	int start, end;

	CurrentLinkPos = FirstLinkPos;
	while( CurrentLinkPos != NULL)
	{
		Zbegin = 0;
		Zdif = (int) (CurrentLinkPos->AZErr.Z * pixelsdeg * RadToDeg * errscale);
		Zend = Zbegin - Zdif;
		if( Zdif <= 0)
		{
			start = Zbegin;
			end = Zend;
		}
		else
		{
			start = Zend;
			end = Zbegin;
		}
		Abegin = (int) (CurrentLinkPos->P.Az * pixelsdeg * RadToDeg);
		for( y = start; y <= end; y++)
			putpixel( xoffset+Abegin, yoffset+y, azcolor);
		CurrentLinkPos = CurrentLinkPos->NextLinkPos;
	}
}

void GraphAltOnAlt( const int xoffset, const int yoffset)
{
	int x;
	int start, end;
	double S;

	/* plot altitude errors vs altitude looking for tube flexure */
	CurrentLinkPos = FirstLinkPos;
	while( CurrentLinkPos != NULL)
	{
		Abegin = 0;
		Adif = (int) (CurrentLinkPos->AZErr.A * pixelsdeg * RadToDeg * errscale);
		Aend = Abegin + Adif;
		if( Adif >= 0)
		{
			start = Abegin;
			end = Aend;
		}
		else
		{
			start = Aend;
			end = Abegin;
		}
		S = HalfRev - CurrentLinkPos->P.Alt;
		if( S < 0)
			S += OneRev;
		Zbegin = (int) (S * pixelsdeg * RadToDeg);
		for( x = start; x <= end; x++)
			putpixel( xoffset+x, yoffset+Zbegin, altcolor);
		CurrentLinkPos = CurrentLinkPos->NextLinkPos;
	}
}

void GraphAzOnAlt( const int xoffset, const int yoffset)
{
	int x;
	int start, end;
	double S;

	CurrentLinkPos = FirstLinkPos;
	while( CurrentLinkPos != NULL)
	{
		Zbegin = 0;
		Zdif = (int) (CurrentLinkPos->AZErr.Z * pixelsdeg * RadToDeg * errscale);
		Zend = Zbegin + Zdif;
		if( Zdif >= 0)
		{
			start = Zbegin;
			end = Zend;
		}
		else
		{
			start = Zend;
			end = Zbegin;
		}
		S = HalfRev - CurrentLinkPos->P.Alt;
		if( S < 0)
			S += OneRev;
		Abegin = (int) (S * pixelsdeg * RadToDeg);
		for( x = start; x <= end; x++)
			putpixel( xoffset+x, yoffset+Abegin, azcolor);
		CurrentLinkPos = CurrentLinkPos->NextLinkPos;
	}
}

void Graph5( void)
{
	int xoffset, yoffset;

	xoffset = 0;
	yoffset = 0;
	GraphGrid( xoffset, yoffset);
	Seterrscale();
	PlotAnalysisErrorsOnGrid( xoffset, yoffset);
	GraphErrorBars( xoffset, yoffset);

	xoffset = 0;
	yoffset = 400;
	GraphAzLine( xoffset, yoffset);
	GraphAltOnAz( xoffset, yoffset);

	xoffset = 0;
	yoffset = 440;
	GraphAzLine( xoffset, yoffset);
	GraphAzOnAz( xoffset, yoffset);

	xoffset = 400;
	yoffset = 0;
	GraphAltLine( xoffset, yoffset);
	GraphAltOnAlt( xoffset, yoffset);

	xoffset = 440;
	yoffset = 0;
	GraphAltLine( xoffset, yoffset);
	GraphAzOnAlt( xoffset, yoffset);
}

void ProcessMenuGraphAnalysis( void)
{
	int th;
	int tx, ty;

	CloseMouseControl();
	InitGraphics();
	th = 1 + textheight( StrBuf);

	CalcAnalysisErrorsWriteFile();
	Seterrscale();

	tx = 380;
	ty = 380;
	sprintf( StrBuf, "analysis file pointing errors");
	outtextxy( tx, ty+=th, StrBuf);
	sprintf( StrBuf, "vert range: +180 to -180 alt");
	outtextxy( tx, ty+=th, StrBuf);
	sprintf( StrBuf, "horiz range: 0 to 360 az");
	outtextxy( tx, ty+=th, StrBuf);
	sprintf( StrBuf, "grid size: 30 deg");
	outtextxy( tx, ty+=th, StrBuf);
	sprintf( StrBuf, "green = altitude, blue = azimuth");
	outtextxy( tx, ty+=th, StrBuf);
	sprintf( StrBuf, "white bars = 1 deg error");
	outtextxy( tx, ty+=th, StrBuf);
	sprintf( StrBuf, "error(rms) = %3.1f arcminutes", PointErrRMS*RadToArcmin);
	outtextxy( tx, ty+=th, StrBuf);

	Graph5();

	sprintf( StrBuf, "press any key to quit...");
	outtextxy( tx, ty+=th, StrBuf);
	getch();

	CloseGraphics();
	_setcursortype( _NOCURSOR);
	InitMouseControl();

	if( FirstLinkPos != NULL)
		FreeAllLinkPos();
}

void ProcessMenuPurgeAnalysis( void)
{
	WriteWindow( MsgFrame);
	VidMemXY.Y = MsgFrame.Top + 2;
	gotoxy( MsgFrame.Left+3, VidMemXY.Y++);
	printf( "Empty the analysis.dat file (y/n)? ");
	GetResponseWithLX200Check();
	printf( "%c", Response);
	if( Response == 'Y' || Response == 'y' || Response == Return)
	{
		gotoxy( MsgFrame.Left+3, VidMemXY.Y++);
		system( "del analysis.old");
		gotoxy( MsgFrame.Left+3, VidMemXY.Y++);
		system( "copy analysis.dat analysis.old");
		gotoxy( MsgFrame.Left+3, VidMemXY.Y++);
		system( "del analysis.dat");
		Output = fopen( AnalysisFile, "w");
		if( Input != NULL)
			fclose( Output);
	}
	RemoveWindow( MsgFrame);
}

void ProcessMenuAnalysisBestZ1Z2Z3( void)
{
	struct Position Temp;
	double HoldZ1Deg, HoldZ2Deg, HoldZ3Deg;

	Temp = Current;
	HoldZ1Deg = Z1Deg;
	HoldZ2Deg = Z2Deg;
	HoldZ3Deg = Z3Deg;

	WriteWindow( MsgFrame);
	VidMemXY.Y = MsgFrame.Top + 2;
	gotoxy( MsgFrame.Left+3, VidMemXY.Y++);

	if( ComputeBestZ123FromAnalysisFile())
	{
		printf( "BestZ1Deg %3.3f, BestZ2Deg %3.3f, BestZ3Deg %3.3f", BestZ1*RadToDeg, BestZ2*RadToDeg, BestZ3*RadToDeg);
		gotoxy( MsgFrame.Left+3, VidMemXY.Y++);
		printf( "   RMS error is %3.2f arcmin", BestPointErrRMS*RadToArcmin);
	}
	else
		printf( "unable to compute best Z123 from analysis file");

	gotoxy( MsgFrame.Left+3, VidMemXY.Y+=2);
	printf( "adopt new Z1, Z2, Z3 values (y/n)? ");
	GetResponseWithLX200Check();
	if( Response == 'Y' || Response == 'y' || Response == Return)
	{
		Z1Deg = BestZ1*RadToDeg;
		Z2Deg = BestZ2*RadToDeg;
		Z3Deg = BestZ3*RadToDeg;
	}
	else
	{
		Z1Deg = HoldZ1Deg;
		Z2Deg = HoldZ2Deg;
		Z3Deg = HoldZ3Deg;
	}
	printf( "%c", Response);
	SetMountErrorsDeg( Z1Deg, Z2Deg, Z3Deg);
	Current = One;
	InitMatrix( 1);
	Current = Temp;
	RemoveWindow( MsgFrame);
}

void ProcessMenuGraphZ1Z2Z3( void)
{
	Flag QuitFlag;
	struct Position Temp;
	int th;
	int tx, ty;

	CloseMouseControl();
	InitGraphics();
	th = 1 + textheight( StrBuf);

	do
	{
		CalcAnalysisErrorsWriteFile();
		Seterrscale();

		tx = 380;
		ty = 380;
		sprintf( StrBuf, "green = altitude, blue = azimuth");
		outtextxy( tx, ty+=th, StrBuf);
		sprintf( StrBuf, "white bars = 1 deg error");
		outtextxy( tx, ty+=th, StrBuf);
		sprintf( StrBuf, "'1' to change axis alignment Z1");
		outtextxy( tx, ty+=th, StrBuf);
		sprintf( StrBuf, "'2' to change azimuth offset Z2");
		outtextxy( tx, ty+=th, StrBuf);
		sprintf( StrBuf, "'3' to change altitude offset Z3");
		outtextxy( tx, ty+=th, StrBuf);
		sprintf( StrBuf, "type any other key to quit");
		outtextxy( tx, ty+=th, StrBuf);
		sprintf( StrBuf, "Z1=%2.2f Z2=%2.2f Z3=%2.2f deg", Z1Deg, Z2Deg, Z3Deg);
		outtextxy( tx, ty+=th, StrBuf);
		sprintf( StrBuf, "error(rms) = %3.1f arcminutes", PointErrRMS*RadToArcmin);
		outtextxy( tx, ty+=th, StrBuf);

		Graph5();

		QuitFlag = No;
		GetResponseWithLX200Check();
		switch( Response)
		{
			case '1':
				sprintf( StrBuf, "new Z1 deg (is %3.3f) ", Z1*RadToDeg);
				outtextxy( tx, ty-=th*10.5, StrBuf);
				gotoxy( 70, 23);
				GetDouble( &Z1Deg);
				break;
			case '2':
				sprintf( StrBuf, "new Z2 deg (is %3.3f) ", Z2*RadToDeg);
				outtextxy( tx, ty-=th*10.5, StrBuf);
				gotoxy( 70, 23);
				GetDouble( &Z2Deg);
				break;
			case '3':
				sprintf( StrBuf, "new Z3 deg (is %3.3f) ", Z3*RadToDeg);
				outtextxy( tx, ty-=th*10.5, StrBuf);
				gotoxy( 70, 23);
				GetDouble( &Z3Deg);
				break;
			default:
				QuitFlag = Yes;
		}
		if( !QuitFlag)
		{
			SetMountErrorsDeg( Z1Deg, Z2Deg, Z3Deg);
			Temp = Current;
			Current = One;
			InitMatrix( 1);
			Current = Temp;
			clearviewport();
		}
	}while( !QuitFlag);

	CloseGraphics();
	_setcursortype( _NOCURSOR);
	InitMouseControl();

	if( FirstLinkPos != NULL)
		FreeAllLinkPos();
}

void ProcessMenuAltAzEC( void)
{
	int x;
	int th;
	int tx, ty;
	int xoffset, yoffset;

	CloseMouseControl();
	InitGraphics();
	th = 1 + textheight( StrBuf);

	CalcAnalysisErrorsWriteFile();
	Seterrscale();

	tx = 0;
	ty = 0;
	sprintf( StrBuf, "altitude vs azimuth error correction");
	outtextxy( tx, ty+=th, StrBuf);
	sprintf( StrBuf, "horiz range: 0 to 360 az");
	outtextxy( tx, ty+=th, StrBuf);
	sprintf( StrBuf, "white bar = 1 deg error");
	outtextxy( tx, ty+=th, StrBuf);

	sprintf( StrBuf, "analysis file corrections");
	outtextxy( tx, ty+=th*13, StrBuf);
	xoffset = 0;
	yoffset = 200;
	GraphAzLine( xoffset, yoffset);
	GraphAltOnAz( xoffset, yoffset);

	/* plot 1 degree error bar */
	for( x = 0; x < pixelsdeg * errscale; x++)
		putpixel( xoffset, yoffset+x, highlightcolor);

	sprintf( StrBuf, "current altitude vs azimuth correction curve");
	outtextxy( tx, ty+=th*21, StrBuf);
	xoffset = 0;
	yoffset = 400;
	GraphAzLine( xoffset, yoffset);
	GraphAltAzEC( xoffset, yoffset);

	sprintf( StrBuf,
	"Convert analysis file corrections to altitude vs azimuth correction curve (y/n)? ");
	outtextxy( tx, ty+=th*12, StrBuf);
	GetResponseWithLX200Check();
	if( Response == 'Y' || Response == 'y' || Response == Return)
	{
		sprintf( StrBuf, "yes");
		outtextxy( tx, ty+=th, StrBuf);
		AnalysisToAltAzEC();
		SaveAltAzECFile();
		xoffset = 0;
		yoffset = 200;
		GraphAzLine( xoffset, yoffset);
		GraphAltAzEC( xoffset, yoffset);
	}
	else
	{
		sprintf( StrBuf, "no");
		outtextxy( tx, ty+=th, StrBuf);
	}
	sprintf( StrBuf, "press any key to quit...");
	outtextxy( tx, ty+=th, StrBuf);
	getch();

	CloseGraphics();
	_setcursortype( _NOCURSOR);
	InitMouseControl();

	if( FirstLinkPos != NULL)
		FreeAllLinkPos();
}

void GraphAltAzEC( const int xoffset, const int yoffset)
{
	int IxB;
	int ystart, yend, xstart, xend;

	setcolor( YELLOW);
	for( Ix = 0; Ix < ECSize; Ix++)
	{
		ystart = yoffset - AltAzEC[Ix] * errscale * pixelsdeg;
		IxB = Ix + 1;
		if( IxB == ECSize)
			IxB = 0;
		yend = yoffset - AltAzEC[IxB] * errscale * pixelsdeg;
		xstart = xoffset + Ix * EC_Resolution_Deg * pixelsdeg;
		xend = xstart + EC_Resolution_Deg * pixelsdeg;
		line( xstart, ystart, xend, yend);
	}
	setcolor( WHITE);
}

void ProcessMenuAltAltEC( void)
{
	int x;
	int th;
	int tx, ty;
	int xoffset, yoffset;

	CloseMouseControl();
	InitGraphics();
	th = 1 + textheight( StrBuf);

	CalcAnalysisErrorsWriteFile();
	Seterrscale();

	tx = 0;
	ty = 0;
	sprintf( StrBuf, "altitude vs altitude error correction");
	outtextxy( tx, ty+=th, StrBuf);
	sprintf( StrBuf, "vertical range: +180 to -180 altitude");
	outtextxy( tx, ty+=th, StrBuf);
	sprintf( StrBuf, "white bar = 1 deg error");
	outtextxy( tx, ty+=th, StrBuf);

	sprintf( StrBuf, "analysis file corrections  //  current altitude vs altitude correction curve");
	outtextxy( tx, ty+=th*2, StrBuf);
	xoffset = 200;
	yoffset = 100;
	GraphAltLine( xoffset, yoffset);
	GraphAltOnAlt( xoffset, yoffset);

	/* plot 1 degree error bar */
	for( x = 0; x < pixelsdeg * errscale; x++)
		putpixel( xoffset+x, yoffset, highlightcolor);

	xoffset = 400;
	yoffset = 100;
	GraphAltLine( xoffset, yoffset);
	GraphAltAltEC( xoffset, yoffset);

	sprintf( StrBuf,
	"Convert analysis file corrections to altitude vs altitude correction curve (y/n)? ");
	outtextxy( tx, ty+=th*2, StrBuf);
	GetResponseWithLX200Check();
	if( Response == 'Y' || Response == 'y' || Response == Return)
	{
		sprintf( StrBuf, "yes");
		outtextxy( tx, ty+=th, StrBuf);
		AnalysisToAltAltEC();
		SaveAltAltECFile();
		xoffset = 200;
		yoffset = 100;
		GraphAltAltEC( xoffset, yoffset);
	}
	else
	{
		sprintf( StrBuf, "no");
		outtextxy( tx, ty+=th, StrBuf);
	}
	sprintf( StrBuf, "press any key to quit...");
	outtextxy( tx, ty+=th, StrBuf);
	getch();

	CloseGraphics();
	_setcursortype( _NOCURSOR);
	InitMouseControl();

	if( FirstLinkPos != NULL)
		FreeAllLinkPos();
}

void GraphAltAltEC( const int xoffset, const int yoffset)
{
	int IxB;
	int ystart, yend, xstart, xend;

	setcolor( YELLOW);
	for( Ix = 0; Ix < ECSize; Ix++)
	{
		xstart = xoffset + AltAltEC[Ix] * errscale * pixelsdeg;
		IxB = Ix + 1;
		if( IxB == ECSize)
			IxB = 0;
		xend = xoffset + AltAltEC[IxB] * errscale * pixelsdeg;
		ystart = yoffset + (360 - Ix * EC_Resolution_Deg) * pixelsdeg;
		yend = ystart - EC_Resolution_Deg * pixelsdeg;
		line( xstart, ystart, xend, yend);
	}
	setcolor( WHITE);
}

void ProcessMenuAzAzEC( void)
{
	int x;
	int th;
	int tx, ty;
	int xoffset, yoffset;

	CloseMouseControl();
	InitGraphics();
	th = 1 + textheight( StrBuf);

	CalcAnalysisErrorsWriteFile();
	Seterrscale();

	tx = 0;
	ty = 0;
	sprintf( StrBuf, "azimuth vs azimuth error correction");
	outtextxy( tx, ty+=th, StrBuf);
	sprintf( StrBuf, "horiz range: 0 to 360 az");
	outtextxy( tx, ty+=th, StrBuf);
	sprintf( StrBuf, "white bar = 1 deg error");
	outtextxy( tx, ty+=th, StrBuf);

	sprintf( StrBuf, "analysis file corrections");
	outtextxy( tx, ty+=th*13, StrBuf);
	xoffset = 0;
	yoffset = 200;
	GraphAzLine( xoffset, yoffset);
	GraphAzOnAz( xoffset, yoffset);

	/* plot 1 degree error bar */
	for( x = 0; x < pixelsdeg * errscale; x++)
		putpixel( xoffset, yoffset+x, highlightcolor);

	sprintf( StrBuf, "current azimuth vs azimuth correction curve");
	outtextxy( tx, ty+=th*21, StrBuf);
	xoffset = 0;
	yoffset = 400;
	GraphAzLine( xoffset, yoffset);
	GraphAzAzEC( xoffset, yoffset);

	sprintf( StrBuf,
	"Convert analysis file corrections to azimuth vs azimuth correction curve (y/n)? ");
	outtextxy( tx, ty+=th*12, StrBuf);
	GetResponseWithLX200Check();
	if( Response == 'Y' || Response == 'y' || Response == Return)
	{
		sprintf( StrBuf, "yes");
		outtextxy( tx, ty+=th, StrBuf);
		AnalysisToAzAzEC();
		SaveAzAzECFile();
		xoffset = 0;
		yoffset = 200;
		GraphAzLine( xoffset, yoffset);
		GraphAzAzEC( xoffset, yoffset);
	}
	else
	{
		sprintf( StrBuf, "no");
		outtextxy( tx, ty+=th, StrBuf);
	}
	sprintf( StrBuf, "press any key to quit...");
	outtextxy( tx, ty+=th, StrBuf);
	getch();

	CloseGraphics();
	_setcursortype( _NOCURSOR);
	InitMouseControl();

	if( FirstLinkPos != NULL)
		FreeAllLinkPos();
}

void GraphAzAzEC( const int xoffset, const int yoffset)
{
	int IxB;
	int ystart, yend, xstart, xend;

	setcolor( YELLOW);
	for( Ix = 0; Ix < ECSize; Ix++)
	{
		ystart = yoffset - AzAzEC[Ix] * errscale * pixelsdeg;
		IxB = Ix + 1;
		if( IxB == ECSize)
			IxB = 0;
		yend = yoffset - AzAzEC[IxB] * errscale * pixelsdeg;
		xstart = xoffset + Ix * EC_Resolution_Deg * pixelsdeg;
		xend = xstart + EC_Resolution_Deg * pixelsdeg;
		line( xstart, ystart, xend, yend);
	}
	setcolor( WHITE);
}

void ProcessMenuAnalysisToPMC( void)
{
	WriteWindow( MsgFrame);
	gotoxy( MsgFrame.Left + 3, MsgFrame.Top + 2);
	printf( "append analysis file to PMC file (y/n)? ");
	GetResponseWithLX200Check();
	printf( "%c", Response);
	if( Response == 'Y' || Response == 'y' || Response == Return)
	{
		CalcAnalysisErrorsWriteFile();
		AppendAnalysisToPMC();
	}
	delay( 500);
	RemoveWindow( MsgFrame);
}

void ProcessMenuAltAzECOnOff( void)
{
	UseAltAzECFlag = !UseAltAzECFlag;
	if( UseAltAzECFlag)
	{
		SetCurrentAltazToAccumMs();
		PauseUntilNewSidTime();
	}
	else
	{
		sprintf( StrBuf, "      ");
		VidMemXY = DisplayXY[DisplayAltAzEC];
		TextAttr = DisplayText;
		WriteStrBufToScreen_f_ptr();
	}
}

void ProcessMenuAltAltECOnOff( void)
{
	UseAltAltECFlag = !UseAltAltECFlag;
	if( UseAltAltECFlag)
	{
		SetCurrentAltazToAccumMs();
		PauseUntilNewSidTime();
	}
	else
	{
		sprintf( StrBuf, "      ");
		VidMemXY = DisplayXY[DisplayAltAltEC];
		TextAttr = DisplayText;
		WriteStrBufToScreen_f_ptr();
	}
}

void ProcessMenuAzAzECOnOff( void)
{
	UseAzAzECFlag = !UseAzAzECFlag;
	if( UseAzAzECFlag)
	{
		SetCurrentAltazToAccumMs();
		PauseUntilNewSidTime();
	}
	else
	{
		sprintf( StrBuf, "      ");
		VidMemXY = DisplayXY[DisplayAzAzEC];
		TextAttr = DisplayText;
		WriteStrBufToScreen_f_ptr();
	}
}

void ProcessMenuPMCOnOff( void)
{
	PointingModelFlag = !PointingModelFlag;
	if( PointingModelFlag)
	{
		if( LoadPMC())
			if( LinkPMCCount)
			{
				CalcPMCErrors();
				SetCurrentAltazToAccumMs();
				PauseUntilNewSidTime();
			}
			else
			{
				PressKeyToContMsg( "empty PMC.DAT");
				PointingModelFlag = No;
			}
		else
		{
			PressKeyToContMsg( "no PMC.DAT file found");
			PointingModelFlag = No;
		}
	}
	else
	{
		sprintf( StrBuf, "            ");
		VidMemXY = DisplayXY[DisplayPMC];
		TextAttr = DisplayText;
		WriteStrBufToScreen_f_ptr();
	}
}

void ProcessMenuGraphPMC( void)
{
	int th;
	int tx, ty;
	int xoffset, yoffset;

	CloseMouseControl();
	InitGraphics();
	th = 1 + textheight( StrBuf);

	/* LinkPos is freed before loading in a file */
	if( LoadPMC())
		if( LinkPMCCount)
		{
			CalcPMCErrors();
			Seterrscale();

			tx = 380;
			ty = 380;
			sprintf( StrBuf, "PMC graph");
			outtextxy( tx, ty+=th, StrBuf);
			sprintf( StrBuf, "vert range: +180 to -180 alt");
			outtextxy( tx, ty+=th, StrBuf);
			sprintf( StrBuf, "horiz range: 0 to 360 az");
			outtextxy( tx, ty+=th, StrBuf);
			sprintf( StrBuf, "grid size: 30 deg");
			outtextxy( tx, ty+=th, StrBuf);
			sprintf( StrBuf, "green = altitude, blue = azimuth");
			outtextxy( tx, ty+=th, StrBuf);
			sprintf( StrBuf, "white bars = 1 deg error");
			outtextxy( tx, ty+=th, StrBuf);
			sprintf( StrBuf, "error(rms) = %3.1f arcminutes", PointErrRMS*RadToArcmin);
			outtextxy( tx, ty+=th, StrBuf);


			xoffset = 0;
			yoffset = 0;
			GraphGrid( xoffset, yoffset);

			PlotPMCErrorsOnGrid( xoffset, yoffset);
			GraphErrorBars( xoffset, yoffset);
		}
		else
		{
			tx = 1;
			ty = 1;
			sprintf( StrBuf, "empty PMC file: nothing to graph");
			outtextxy( tx, ty, StrBuf);
		}
	else
	{
		tx = 1;
		ty = 1;
		sprintf( StrBuf, "could not find PMC file");
		outtextxy( tx, ty, StrBuf);
	}
	sprintf( StrBuf, "press any key to quit...");
	outtextxy( tx, ty+=th, StrBuf);
	getch();

	CloseGraphics();
	_setcursortype( _NOCURSOR);
	InitMouseControl();

	if( !PointingModelFlag && FirstLinkPMC != NULL)
		FreeAllLinkPMC();
}

void ProcessMenuSetZ1Z2Z3( void)
{
	double Z1Deg, Z2Deg, Z3Deg;
	struct Position Temp;

	Z1Deg = Z1*RadToDeg;
	Z2Deg = Z2*RadToDeg;
	Z3Deg = Z3*RadToDeg;

	WriteWindow( MsgFrame);
	VidMemXY.Y = MsgFrame.Top + 2;
	gotoxy( MsgFrame.Left+3, VidMemXY.Y);
	printf( "Please enter new Z1 deg (is %3.3f) ", Z1*RadToDeg);
	gotoxy( MsgFrame.Left+38, VidMemXY.Y);
	if( GetDouble( &Z1Deg))
	{
		gotoxy( MsgFrame.Left+3, VidMemXY.Y+=1);
		printf( "Please enter new Z2 deg (is %3.3f) ", Z2*RadToDeg);
		gotoxy( MsgFrame.Left+38, VidMemXY.Y);
		if( GetDouble( &Z2Deg))
		{
			gotoxy( MsgFrame.Left+3, VidMemXY.Y+=1);
			printf( "Please enter new Z3 deg (is %3.3f) ", Z3*RadToDeg);
			gotoxy( MsgFrame.Left+38, VidMemXY.Y);
			if( GetDouble( &Z3Deg))
			{
				SetMountErrorsDeg( Z1Deg, Z2Deg, Z3Deg);
				if( Two.Init)
				{
					Temp = Current;
					Current = One;
					InitMatrix( 1);
					Current = Temp;
					gotoxy( MsgFrame.Left+3, VidMemXY.Y+=2);
					printf( "re-init'd with new Z1, Z2, and Z3");
					delay( 1000);
				}
			}
		}
	}
	RemoveWindow( MsgFrame);
}

