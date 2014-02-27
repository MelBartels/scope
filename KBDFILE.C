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

void GetDir( void)
{
	Flag ValidName = False;

	FoundFile = True;
	while( FoundFile && !ValidName)
	{
		if( FindFirst)
		{
			FindFirst = False;
			/* returns 0 if successful */
			FoundFile = !findfirst( "*.*", &Ffblk, FA_DIREC);
		}
		else
			FoundFile = !findnext( &Ffblk);
		if( FoundFile)
			if( Ffblk.ff_attrib == FA_DIREC)
				ValidName = True;
	}
}

void GetFileName( char* Name)
{
	Flag ValidName = False;

	FoundFile = True;
	while( FoundFile && !ValidName)
	{
		if( FindFirst)
		{
			FindFirst = False;
			/* returns 0 if successful */
			FoundFile = !findfirst( Name, &Ffblk, 0);
		}
		else
			FoundFile = !findnext( &Ffblk);
		if( FoundFile && !IsProgramFile( Ffblk.ff_name))
			ValidName = True;
	}
}

Flag IsProgramFile( char* Name)
{
	if( strcmpi( AltAltECFile, Name)==0)
		return True;
	if( strcmpi( AltAzECFile, Name)==0)
		return True;
	if( strcmpi( AzAzECFile, Name)==0)
		return True;
	if( strcmpi( AnalysisFile, Name)==0)
		return True;
	if( strcmpi( ConfigFile, Name)==0)
		return True;
	if( strcmpi( GuideAltFile, Name)==0)
		return True;
	if( strcmpi( GuideAzFile, Name)==0)
		return True;
	if( strcmpi( InitHistoryFile, Name)==0)
		return True;
	if( strcmpi( OutGuideFile, Name)==0)
		return True;
	if( strcmpi( PECFile, Name)==0)
		return True;
	if( strcmpi( PMCFile, Name)==0)
		return True;
	if( strcmpi( RecordEquatFile, Name)==0)
		return True;
	if( strcmpi( SlewFile, Name)==0)
		return True;
	if( strcmpi( SlewOutFile, Name)==0)
		return True;

	return False;
}

/* copies into Filename the selected file name, found by matching to 'Name' */
Flag SelectDataFilename( char* Name)
{
	struct FilenameString N[MaxNameArray];
	int Left, Top;
	int Menu;
	int ColumnSpacing;
	int IxB;

	LoadFnames( Name);
	SortLinkFname();

	ValidResponseFlag = No;
	CurrentLinkFname = FirstLinkFname;
	if( CurrentLinkFname != NULL)
		while( True)
		{
			Menu = 'a';
			WriteWindow( DataFileFrame);
			Left = DataFileFrame.Left + 2;
			Top = DataFileFrame.Top + 2;
			ColumnSpacing = (DataFileFrame.Right - DataFileFrame.Left)/3 - 1;
			while( CurrentLinkFname != NULL && (Menu-'a') < MaxNameArray)
			{
				gotoxy( Left, Top);
				if( CurrentLinkFname->FNameType == NameIsADirectory)
				{
					printf( "%c. [%s]\n", Menu, CurrentLinkFname->Name);
					N[Menu-'a'].FNameType = NameIsADirectory;
				}
				else
				{
					printf( "%c. %s\n", Menu, CurrentLinkFname->Name);
					N[Menu-'a'].FNameType = NameIsAFile;
				}
				strcpy( N[Menu-'a'].Name, CurrentLinkFname->Name);
				Menu++;
				/* print next on line below unless time for new column */
				if( (Menu-'a') % 8)
					Top++;
				else
				{
					Top -= 7;
					Left += ColumnSpacing;
				}
				CurrentLinkFname = CurrentLinkFname->NextLinkFname;
			}
			gotoxy( DataFileFrame.Left + 4, DataFileFrame.Bottom - 3);
			printf( "select a-%c or any other key to escape", Menu-1);
			gotoxy( DataFileFrame.Left + 4, DataFileFrame.Bottom - 2);
			printf( "'-' to go back a page");
			if( CurrentLinkFname != NULL)
			{
				gotoxy( DataFileFrame.Left + 4, DataFileFrame.Bottom - 1);
				printf( "<space bar> for more");
			}
      /* Highlight each file and move with cursor or mouse and select  */
         HighlightAndSelect( &DataFileFrame, ColumnSpacing, Menu - 'a');
			RemoveWindow( DataFileFrame);
			/* exit loop if valid response */
			if( Response >= 'a' && Response < Menu)
			{
				ValidResponseFlag = Yes;
				Ix = Response - (int) 'a';
				break;
			}
			else
				/* go back a page if '-' key pressed */
				if( Response == '-')
				{
					/* go back to start of displayed names, plus another page back */
					for( IxB = 0; IxB < MaxNameArray*2 && CurrentLinkFname->PrevLinkFname != NULL; IxB++)
						CurrentLinkFname = CurrentLinkFname->PrevLinkFname;
					Menu = 'a';
				}
				else
					/* read in more if space bar was pressed */
					if( CurrentLinkFname != NULL && Response == ' ')
						Menu = 'a';
					else
						break;
		}
	else
		PressKeyToContMsg( "No data files found");
	   FreeAllLinkFname();
	   if( ValidResponseFlag)
		if( N[Response-'a'].FNameType == NameIsAFile)
		{
			strcpy( Filename, N[Response-'a'].Name);
			return True;
		}
		else
		{
			chdir( N[Response-'a'].Name);
			/* if a directory selected, repeatedly call SelectDataFilename(): return the last called
			function value as the stack is popped so that MenuCoordFile() can handle the result */
			return SelectDataFilename( Name);
		}
	else
	{
		PressKeyToContMsg( "No data file selected");
		return False;
	}
}

void MenuCoordFile( void)
{
	int Loop;

	for( Loop = 0; Loop < SelectObject; Loop++)
		MenuCoordFileSubr( Loop);

   FileType = ReadDataFile;   /* Default file type  */   
}

void MenuCoordFileSubr( const int Loop)
{
   struct Object O;
   struct Index I[MaxCoord];
	int Left, Top;
	int Ix;
   int Count;
	char Menu;
	char LookAheadChar = ' ';
	Flag LookAheadCharRead = No;
   char ObjectFile[10];
   long int OffsetOfRecord;

	FPosPtr = (fpos_t*) malloc( MAX_FPOS_PTR * sizeof( fpos_t));
	if( FPosPtr == NULL)
		BadExit( "Problem with malloc of FPosPtr in MenuCoordFile()");

   Planet = 0;       /* Default PlanetCoords() values  */
   DriftFlag = Off;

	/* this function copies a selected file name into Filename, or changes directories, then copies
	the file name */
	/* change to last data file directory */
	chdir( DataDir);

   switch ( FileType)
   {
     case ReadCometAsteroidFile:
     {
       strcpy( ObjectFile, CometFile);
       break;
     }
     case ReadCometElements:
     case ReadAsteroidElements:
     {
       strcpy( ObjectFile, ElementFile);
       break;
     }
     default:
       strcpy( ObjectFile, DataFile);
   }

	if( DataFileNameSet || SelectDataFilename( ObjectFile))
	{
		Input = fopen( Filename, "r");
		if( Input == NULL)
			BadExit( strcat( "Could not open ", Filename));

		if( !feof( Input))
		{
			FPosPtrIx = 0;
         fgetpos( Input, &FPosPtr[FPosPtrIx++]);

			/* while there are objects in the input file */
			while( ObjectNames( I, &Count))
			{
				/* Restore char read while determining eof on previous screen? */
				/* each line in the data files starts with a blank, so char will always be ' ' so no
				need to restore char at this time */
				if( LookAheadCharRead) { }
				/* save screen & draw border */
				WriteWindow( CoordFrame);
				/* write objects and instructions */
				Left = CoordFrame.Left + 1;
				Top = CoordFrame.Top + 2;
				gotoxy( Left, Top);
				Menu = 'a';
				for( Ix = 1; Ix <= Count; Ix++)
				{
					gotoxy( Left, Top);
               printf( "%c. %s ", Menu++, I[Ix-1].Name);
					if( Ix%8)
						Top++;
					else
					{
						Top -= 7;
						Left += ObjectNameSize+2;
					}
				}
				gotoxy( CoordFrame.Left + 13, CoordFrame.Bottom - 3);
				printf( "select a-%c or any other key to escape", (char) ('a' + Count - 1));
				gotoxy( CoordFrame.Left + 13, CoordFrame.Bottom - 2);
				printf( "'-' to go back a page");
				/* look ahead to see if file is finished */
				FReadChar( Input, &LookAheadChar);
				LookAheadCharRead = Yes;
				if( !feof( Input))
				{
					gotoxy( CoordFrame.Left + 13, CoordFrame.Bottom - 1);
					printf( "<space bar> for more");
				}
			/* Highlight each object and move with cursor or mouse and select  */
            HighlightAndSelect( &CoordFrame, ObjectNameSize+2, Count);
				/* restore screen */
				RemoveWindow( CoordFrame);
				/* exit loop if valid response */
				ValidResponseFlag = No;
				if( Response >= 'a' && Response < ('a' + Count))
				{
					ValidResponseFlag = Yes;
					Ix = Response - (int) 'a';
					break;
				}
				else
					/* go back a page if '-' key pressed */
					if( Response == '-')
					{
						FPosPtrIx -= 2;
						if( FPosPtrIx < 0)
							FPosPtrIx = 0;
                  fsetpos( Input, &FPosPtr[FPosPtrIx]);
						FPosPtrIx++;
					}
					/* read in more if space bar was pressed */
					else
						if( !feof( Input) && Response == ' ')
							fgetpos( Input, &FPosPtr[FPosPtrIx++]);
						else
							break;
			}
         /* save data file directory */
           GetCurDir( DataDir);
         /* return to default directory */
           chdir( DefaultDir);

			/* if valid input, process input */
			if( ValidResponseFlag)
			{
            if( FileType == ReadDataFile)
            {
              GrandTourLoaded = Yes;
		        GrandTourRecNum = 0;
		        strcpy( GrandTourFilename, Filename);

         /* This next statement allows a Grand Tour to start at any point in
            a data file, not just at the beginning.  If <CR> is pressed after the
            datafile is selected, then code operates as before. */
              GrandTourRecNum = (FPosPtrIx - 1)*24 + Ix;
            }
         /* Update Julian Day needed for precession, planet and minor body calcs  */
            UpdateJD();
            
         /* Start of Object position routines. Default is for .dat and .cdf files  */
            ManualInputFlag = Off;   /* Gets turned on in InputOrbitalElements  */

            OffsetOfRecord = I[Ix].CurrentOffset - ftell( Input);

            switch ( Planet)
            {
              case 3:    /* Planets  */
              {
                Planet = Ix + 1;
                if( Planet > 2) Planet++;  /* Earth=3 not on list */
                PlanetCoords();
                break;
              }
              case 10:   /* Comet orbital elements  */
              {
                ReadOrbitalElements( OffsetOfRecord);
                PlanetCoords();
                break;
              }
              case 11:   /* Asteroid orbital elements  */
              {
                 ReadOrbitalElements( OffsetOfRecord);
                 PlanetCoords();
                 break;
              }
              default:
              {
                fseek( Input, OffsetOfRecord, SEEK_CUR);
                ReadRa( Input, &O.RaHMSH);
                ReadDec( Input, &O.DecDMS);
                if( FileType == ReadCometAsteroidFile)
                {
                  ReadRa( Input, &O.DriftHMSH);
                  ReadDec( Input, &O.DriftDMS);
                }
                if( FileType == ReadCometAsteroidFile)
                {
                  RemoveCometDriftFromDrift();
                  CalcRadFromHMSH( &CometDrift.Ra, O.DriftHMSH);
                  CalcRadFromDMS( &CometDrift.Dec, O.DriftDMS);
                  AddCometDriftToDrift();
                  DisplayDrift();
                }
                In.RaHMSH = O.RaHMSH;
                In.DecDMS = O.DecDMS;
                CalcRadFromHMSH( &In.Ra, In.RaHMSH);
                CalcRadFromDMS( &In.Dec, In.DecDMS);
                break;
              }
            }
            if( precessionNutationAberration)
            {
              calcCorrectionsForEpochJD( JD, &In);
            }
            GetHMSH( In.Ra*RadToHundSec + .5, &In.RaHMSH);
            GetDMS( In.Dec*RadToArcsec + .5, &In.DecDMS);
            LastDataObject = In;
            if( Planet == 10 || Planet == 11)
              DisplayMag( I, Ix);
            DisplayIn( Filename, I[Ix].Name);

            Star.Ra[Loop] = In.Ra;
            Star.Dec[Loop] = In.Dec;
            strcpy(Star.Name[Loop], I[Ix].Name);
			}
			else
			{
				WriteWindow( MsgFrame);
				gotoxy( MsgFrame.Left + 5, MsgFrame.Top + 2);
				printf( "No object selected");
				gotoxy( MsgFrame.Left + 5, MsgFrame.Top + 4);
				ContMsgRoutine();
				RemoveWindow( MsgFrame);
				DisplayIn( Filename, NameBlanks);
			}
		}
		fclose( Input);
  	}
}

void DisplayMag( struct Index I[], int Ix)
{
  double Mag;
  double r,R,Phi1,Phi2;
  double Beta;
  char *MagStr;
  int Dec,Sign,L;

  L = strlen( I[Ix].Name)-1;
  while((I[Ix].Name[L]==' ')||(I[Ix].Name[L]=='\t'))  /* Remove trailing spaces */
  {
    L--;
  }
  I[Ix].Name[L+2] = '\0';

  L = strlen( I[Ix].Name);
  if( L < 21)    /* Room to add magnitude to name  */
  {
    r = sqrt( x*x + y*y + z*z);  /* Body-Sun Distance  */
    if( Planet == 11)
    {
      R = sqrt( X*X + Y*Y + Z*Z);  /* Earth-Sun Distance  */
      Beta = acos( (r*r + Distance*Distance - R*R)/(2.0*r*Distance));
      Phi1 = exp( -3.33*pow( tan( Beta/2.0), 0.63));
      Phi2 = exp( -1.87*pow( tan( Beta/2.0), 1.22));
      Mag = 0.05 + VisMag + 5.*log10( r*Distance) - 2.5*log10( (1-Slope)*Phi1 + Slope*Phi2);
    }
    else
      Mag = AbsMag + 5*log10( Distance) + K*log10( r);


    MagStr = fcvt( Mag, 1, &Dec, &Sign);  /* Convert magnitude to string  */
    L = strlen( MagStr);
    MagStr[L+1] = '\0';
    MagStr[L] = MagStr[L-1];
    MagStr[L-1] = '.';
    strcat( I[Ix].Name, MagStr);
  }
}

void ReadRaDec( FILE* File)
{
  char c = ' ';
  int A;

  while( !feof( Input) && c != '+' && c != '-' && c != '.' && !(c >= '0' && c <= '9') && c != -1)
		c = fgetc( File);
	ungetc( c, File);

	fscanf( File, "%d %d %d %d %d %d", &A,&A,&A,&A,&A,&A);
}

void HighlightAndSelect( struct Frame *frameptr, int Spacing, int LineCount)
{
  int HighlightSelect, LastHighlightSelect;


  HighlightSelect = LastHighlightSelect = 0;
  ObjectNameTextAttr( *frameptr, Spacing, HighlightSelect, SelectText);
  /* get response: either keyboard hit or mouse click  */
  while( True)
  {
    if( HighlightSelect != LastHighlightSelect)
    {
      ObjectNameTextAttr( *frameptr, Spacing, LastHighlightSelect, DefaultText);
      ObjectNameTextAttr( *frameptr, Spacing, HighlightSelect, SelectText);
      LastHighlightSelect = HighlightSelect;
    }
    WaitForNewSidT();
    CheckLX200Events();
    if( KeyStroke)
    {
      Response = getch();
      if( Response == Return)
      {
        Response = HighlightSelect + 'a';
        break;
      }
      else
        if( Response == ExtendedKeyboardStroke)
        {
         Response = getch();
         if( Response == UpCursor)
         {
           HighlightSelect--;
           if( HighlightSelect < 0)
             HighlightSelect = LineCount - 1;
         }
         else
           if( Response == DownCursor)
           {
             HighlightSelect++;
             if( HighlightSelect >= LineCount)
               HighlightSelect = 0;
           }
        }
        else
        {
          Response = tolower( Response);
          break;
        }
      }
      if( UseMouseFlag && MouseLeftButtonPressCount())
      {
        if( MouseXText > (*frameptr).Left+2 && MouseXText < (*frameptr).Right-2)
        {
        /* 8 rows go from +2 to +9  */
          if( MouseYText >=(*frameptr).Top+2 && MouseYText <= (*frameptr).Top+9)
          {
          /* get the row  */
            Response = MouseYText - ((*frameptr).Top+2);
          /* get the column: 8 rows spaced 21 chars apart */
            Response += 8 * ((MouseXText - ((*frameptr).Left+2)) / Spacing);
            Response += 'a';
          }
          else
            if( MouseYText == (*frameptr).Bottom-1)
              Response = ' ';
            else
              if( MouseYText == (*frameptr).Bottom-2)
                Response = '-';
              else
                Response = 0;
                break;
        }
        else
          Response = 0;
        break;
      }
    }
}

void ReadOrbitalElements( long int OffsetOfRecord)
{
  char Str1[MaxStrIx+1];
  int Year,Month;
  double Day;
  char y[5],m[3],d[10];
  char man0[15],e[15],p[15],dist[15],n[15],i[15];
  char vis[6],slope[6],absmag[6],k[6];

  OffsetOfRecord += 46;   /* Move file pointer past name string  */
  fseek( Input, OffsetOfRecord, SEEK_CUR);
  fgets( Str1, MaxStrIx, Input);
  if( Planet == 10)
  {
    sscanf( Str1, "%s %s %s %s %s %s %s %s %s %s", &y,&m,&d,&dist,
      &e,&p,&n,&i,&absmag,&k);   /* Ignore any fields after Inclination  */
    AbsMag = atof( absmag);
    K = atof( k);
  }
  else
  {
    sscanf( Str1, "%s %s %s %s %s %s %s %s %s %s %s", &y,&m,&d,&man0,&dist,
      &e,&p,&n,&i,&vis,&slope);
    MeanAn0 = atof( man0)*DegToRad;
    VisMag = atof( vis);
    Slope = atof( slope);
  }

  PDist = SemiAxis = atof( dist);   /* Arbitrary setting of SemiAxis/PDist to avoid bad data trap  */
  Eccen = atof( e);
  Peri = atof( p)*DegToRad;
  Node = atof( n)*DegToRad;
  Incl = atof( i)*DegToRad;
  Year = atoi( y);
  Month = atoi( m);
  Day = atof( d);

  CalcEpoch( Year, Month, Day);
  PeriDate = EpochDate = Epoch;   /* A convenience only, not true  */
  DriftFlag = On;
}

void SelectMinorPlanet( void)
{
  int Ix;
  int Left, Top;
  int Menu = 'a';
  char Action[5][45] = {"Select a comet or asteroid from a file",
                        "Select comet orbital elements from a file",
                        "Select asteroid orbital elements from a file",
                        "Input comet orbital elements",
                        "Input asteroid orbital elements",};


  WriteWindow( DataFileFrame);
  Left = DataFileFrame.Left + 2;
  Top = DataFileFrame.Top + 2;

  for( Ix = 0; Ix<5; Ix++)
  {
    gotoxy( Left, Top);
    printf( "%c. %s", Menu,Action[Ix]);
    Menu++;
    Top++;
  }
  gotoxy( DataFileFrame.Left+4, DataFileFrame.Bottom-3);
  printf( "select a-e or any other key to escape ");

  HighlightAndSelect( &DataFileFrame, 48, Menu - 'a');
  RemoveWindow( DataFileFrame);
  /* exit loop if valid response  */
  if( Response >= 'a' && Response < Menu)
  {
    Ix = Response - (int)'a';

    switch ( Ix)
    {
      case 0:
      {
        FileType = ReadCometAsteroidFile;
        MenuCoordFileSubr( 1);
        break;
      }
      case 1:
      {
        FileType = ReadCometElements;
        MenuCoordFileSubr( 1);
        break;
      }
      case 2:
      {
        FileType = ReadAsteroidElements;
        MenuCoordFileSubr( 1);
        break;
      }
      case 3:
      {
        Planet = 10;
        InputOrbitalElements();
        PlanetCoords();
        break;
      }
      case 4:
      {
        Planet = 11;
        InputOrbitalElements();
        PlanetCoords();
        break;
      }
    }
  }
  else
    PressKeyToContMsg( "No action selected");

  FileType = ReadDataFile;  /* Reset to default file type  */
}

void PlanetCoords( void)
{
   double Ra,Dec,NewRa,NewDec;
   void *P;
   double TDelay;
   double HoldCoordYear = DataFileCoordYear;

   DataFileCoordYear = 2000.0;   /* Gets reset at end of PlanetCoords()  */

   DriftDeltaT = 1.0;   /* Calculate drift over one hour and assume linear movement  */
   TimeInstant = JD;

   P = ReadPSTerms( 3);
   CalcPSRectCoords( P);
   free( P);
   X = -x;
   Y = -y;    /* Coords for Sun  */
   Z = -z;

/* Now need coords for Planet, Comet or Asteroid  */
   switch ( Planet)
   {
     case 10:                  /* Comets  */
     case 11:                  /* Asteroids  */
     {

/* Need to trap for PDist and/or SemiAxis == 0  */
       if( PDist == 0.0 || SemiAxis == 0.0)
         BadExit( "Perihelion Distance and/or Semi-major Axis = 0");
       CalcAuxiliaryTerms();
       CalcBodyRectCoords();
       CalcDistance();
       break;
     }
     default:
     {
       P = ReadPSTerms( Planet);
       CalcPSRectCoords( P);
       free( P);
       CalcDistance();
       break;
     }
   }

/* Calculate New Position for Object corrected for delay due to light speed  */
   TDelay = Distance*0.0057755183;
   TimeInstant -= TDelay;
   switch ( Planet)
   {
     case 10:
     case 11:
     {
       CalcBodyRectCoords();
       CalcDistance();
      break;
     }
     default:
     {
       x -= dX*TDelay;
       y -= dY*TDelay;
       z -= dZ*TDelay;
      CalcDistance();
       break;
     }
   }

   Ra = atan2( (y+Y), (x+X));
   Dec = asin( (z+Z)/Distance);
   if( Ra < 0.0) Ra += OneRev;

   if( DriftFlag)  /* Use this to track comet or asteroid  */
   {
     RemoveCometDriftFromDrift();
     TimeInstant += DriftDeltaT/24.0;
     CalcBodyRectCoords();  /* Calculate position some time later and assume linear movement  */
     CalcDistance();
     NewRa = atan2( (y+Y), (x+X));
     NewDec = asin( (z+Z)/Distance);
     if( NewRa < 0.0)
       NewRa += OneRev;
     if( NewRa - Ra > M_PI)      /* Test for movement across Ra 24/0 line */
       NewRa -= OneRev;
     if( NewRa - Ra < -M_PI)
       NewRa += OneRev;
     CometDrift.Ra = (NewRa - Ra)/DriftDeltaT;
     CometDrift.Dec = (NewDec - Dec)/DriftDeltaT;
     AddCometDriftToDrift();
     DisplayDrift();
   }

   In.Ra = Ra;
   In.Dec = Dec;

   if( ManualInputFlag)       /* Manual Data Input  */
     DisplayIn( "Manual Input", NameBlanks);
   else
/* Now calculate Topocentric Coordinates if not manual input */
   CalcTopocentricCoords();

   DataFileCoordYear = HoldCoordYear;
}

void CalcTopocentricCoords( void)
{
   double T,U,RhoSinPhi,RhoCosPhi;
   double BOverA = 0.99664719;
   double SidTimeAtGreenwich,HourAngle;
   double Omega,L,L1,nutationLongitude,nutationRa;
   double DeltaRa,NewDec;
   double Pi;

   T = (JD - JD2000)/36525.0;

   Omega = ValidRad((125.04452 - 1934.136261*T)*DegToRad);
   L = ValidRad((280.4665 + 36000.7698*T)*DegToRad);
   L1 = ValidRad((218.3165 + 481267.8813*T)*DegToRad);
   nutationLongitude = -17.2*sin( Omega)-1.32*sin( 2*L)-0.23*sin( 2*L1)
                       +0.21*sin( 2*Omega);
   nutationRa = nutationLongitude*CosObliq/15.0;

   SidTimeAtGreenwich = ValidRad((280.46061837 + 360.9856473663*(JD - 2451545.0)
                         + 0.000387933*T*T - T*T*T/38710000.0)*DegToRad
                         + nutationRa*ArcsecToRad);

   U = atan( BOverA*tan( LatitudeDeg*DegToRad));
   RhoSinPhi = BOverA*sin( U) + Height*sin( LatitudeDeg*DegToRad)/6378140.0;
   RhoCosPhi = cos( U) + Height*cos( LatitudeDeg*DegToRad)/6378140.0;

   HourAngle = ValidRad(SidTimeAtGreenwich- LongitudeDeg*DegToRad - In.Ra);

   Pi = 8.794*DegToRad/Distance/3600;

   DeltaRa = atan2( -RhoCosPhi*sin( Pi)*sin( HourAngle),
                   (cos( In.Dec) - RhoCosPhi*sin( Pi)*cos( HourAngle)));
   NewDec = atan2( (sin( In.Dec) - RhoSinPhi*sin( Pi))*cos( DeltaRa),
                    (cos( In.Dec) - RhoCosPhi*sin( Pi)*cos( HourAngle)));

   In.Ra += DeltaRa;
   In.Dec = NewDec;
}

int Unpack( double *ovals, const char *ibuff)
{
   int i;
   unsigned short flags = *(unsigned short *)ibuff;
   const char *iptr = ibuff + 2;

   for( i = 0; i < 6; i++, flags >>= 2)
     switch( flags & 3)
     {
       case 0:
         *ovals++ = (double)*iptr++;
         break;
       case 1:
			*ovals++ = (double)(*(short *)iptr);
			iptr += 2;
			break;
		 case 2:
			*ovals++ = (double)(*(long *)iptr);
			iptr += 4;
			break;
		 case 3:
			*ovals++ = *((double *)iptr);
			iptr += 8;
			break;
	  }
	return (int) (iptr - ibuff);
}

/* PS functions adapted from ProjectPluto  */
/* Based upon PS1996, which is a frequency analysis of JPL DE403  */
void * ReadPSTerms( int Planet)
{
   long Offsets[10], Jump = 0;
   short BlockSize[30];
   int Block, i;
   char *tbuff, *tptr;
   char Filename[] = "ps1996.ptt";
   struct PoissonHeader Header;
   struct Poisson P, *rval;

   Input = fopen( Filename, "rb");
   if( Input == NULL)
     BadExit( strcat( "Could not open ", Filename));


   fseek( Input, 0L, SEEK_SET);
   fread( Offsets, 10, sizeof( long), Input);
   fseek( Input, Offsets[Planet - 1], SEEK_SET);
   fread( &Header, sizeof( struct PoissonHeader), 1, Input);
   Block = (int)floor(( TimeInstant - Header.TZero)/Header.dT);

   P.TZero = Header.TZero + (double)Block * Header.dT;
   P.dT = Header.dT;
   P.BlockCount = Header.BlockCount;
   P.TotalFreqs = Header.TotalFreqs;
   for( i = 0; i < 3; i++)
     P.nf[i] = Header.nf[i];

   rval = (struct Poisson *)malloc( sizeof( struct Poisson) +
                                    Header.TotalFreqs*7*sizeof( double));
   if( rval == NULL)
     BadExit( "Problem with malloc of rval in ReadPSTerms()");

   P.Frequencies = (double *)( rval + 1);
   P.Terms = P.Frequencies + Header.TotalFreqs;
   fread( P.Frequencies, Header.TotalFreqs, sizeof( double), Input);

   fread( BlockSize, Header.BlockCount, sizeof( short), Input);
   for( i = 0; i < Block; i++)
     Jump += (long) BlockSize[i];
   fseek( Input, Jump, SEEK_CUR);
   tbuff = (char *)malloc( BlockSize[Block]);
   if( !tbuff)
     free( rval);

   fread( tbuff, BlockSize[Block], 1, Input);
   memcpy( P.Secular, tbuff, 12 * sizeof( double));
   tptr = tbuff + 12 * sizeof( double);
   for( i = 0; i < P.TotalFreqs; i++)
     tptr += Unpack( P.Terms + i * 6, tptr);
   free( tbuff);
   fclose( Input);
   memcpy( rval, &P, sizeof( struct Poisson));
   return( rval);
}


void CalcPSRectCoords( void *PPtr)
{
   struct Poisson *P = (struct Poisson *)PPtr;
   double Tx = 2. * (TimeInstant - P->TZero) / P->dT - 1.;
   double Fx = Tx * P->dT / 2.;
   double *FrequencyPtr = P->Frequencies;
   double *SecularPtr = P->Secular;
   double *TermPtr = P->Terms;
   int i, j, m;
   double Wx = 1., Xpower[5];
   double Coords[6];

   Xpower[0] = Xpower[1] = 1.;
   for( i = 2; i < 5; i++)
     Xpower[i] = Xpower[i - 1] * Tx;
   for( i = 0; i < 3; i++)       /* secular terms first: */
   {
     Coords[i] = 0.;
     Wx = 1.;
     for( j = 0; j < 4; j++)
     {
       if( j)
         Coords[i + 3] += (double)j * (*SecularPtr) * Xpower[j];
       else
         Coords[i + 3] = 0.;
       Coords[i] += Wx * (*SecularPtr++);
       Wx *= Tx;
     }
     Coords[i + 3] *= 2. / P->dT;
   }

   Wx = 1.;
   for( m = 0; m < 3; m++)
   {
     double NewSums[6];
     double VelocityScale = (double)( m * 2) * Xpower[m] / P->dT;

     for( i = 0; i < 6; i++)
       NewSums[i] = 0.;
     for( j = 0; j < P->nf[m]; j++)
     {
       double Amplitude = *FrequencyPtr++;
       double F = Amplitude * Fx;
       double CosTerm = cos( F);
       double SinTerm = sin( F);

       for( i = 0; i < 3; i++, TermPtr += 2)
       {
         NewSums[i] += TermPtr[0]*CosTerm + TermPtr[1]*SinTerm;
         NewSums[i + 3] += Amplitude*(TermPtr[1]*CosTerm - TermPtr[0]*SinTerm);
       }
     }

     for( i = 0; i < 3; i++)
     {
       Coords[i] += NewSums[i] * Wx;
       Coords[i + 3] += NewSums[i + 3] * Wx;
       if( m)
         Coords[i + 3] += VelocityScale * NewSums[i];
     }
     Wx *= Tx;
   }

   for( i = 0; i <6; i++)
     Coords[i] *= 1.e-10;         /* cvt to AU,  and to AU/day */
   x = Coords[0];
   y = Coords[1];
   z = Coords[2];
   dX = Coords[3];
   dY = Coords[4];
   dZ = Coords[5];
}

void CalcEpoch( int Year, int Month, float Day)
{
  if( Month < 3)
  {
    Month += 12;
    Year--;
  }
  Epoch = floor(365.25*(Year + 4716)) + floor(30.6001*(Month + 1)) + Day - 13 - 1524.5;
}

void UpdateJD( void)
{
   long Yr;
	int M;
	int D;
	int h;
	int m;
	double s;

	getdate( &d);
	Yr = (long) d.da_year;
	M = (int) d.da_mon;
	D = (int) d.da_day;
	gettime( &t);
	h = (int) t.ti_hour;
	m = (int) t.ti_min;
	s = ((double) t.ti_sec) + (t.ti_hund / 100.);

	CalcJD( Yr, M, D, Tz, DST, h, m, s);
}

void CalcDistance( void)
{
/* Get distance between Earth and Planet to calculate light transit time */
  Distance = sqrt((x+X)*(x+X) + (y+Y)*(y+Y) + (z+Z)*(z+Z));
}

void InputOrbitalElements( void)
{
  int Year;
  int Month;
  int IBlank = 0;
  double Day;
  double DBlank;
  double DoubleIn,MeanAn0Deg,PeriDeg,NodeDeg,InclDeg;
  char YearPhrase[19] = "Year of ";
  char MonthPhrase[20] = "Month of ";
  char DayPhrase[18] = "Day of ";
  char CometDate[] = "Perihelion";
  char AsteroidDate[] = "Epoch";
  char DistanceToSun[20];
  char CometDistance[] = "Perihelion Distance";
  char AsteroidDistance[] = "Semimajor Axis";
  Flag ValidData = No;
  Flag Update = No;

  if( Planet == 10)
  {
    strcat( YearPhrase, CometDate);
    strcat( MonthPhrase, CometDate);
    strcat( DayPhrase, CometDate);
    strcpy( DistanceToSun, CometDistance);
  }
  else
  {
    strcat( YearPhrase, AsteroidDate);
    strcat( MonthPhrase, AsteroidDate);
    strcat( DayPhrase, AsteroidDate);
    strcpy( DistanceToSun, AsteroidDistance);
  }

  WriteWindow( CoordFrame);
/* Write some data entry instructions  */
  gotoxy( CoordFrame.Left+7, CoordFrame.Top+1);
  printf( "Enter Year and Month as integers, all other data as decimals.");

  while( !ValidData)
  {
    if( ElementDisplay( Update, 3, YearPhrase, &Year, &DBlank))
    {
      if( ElementDisplay( Update, 4, MonthPhrase, &Month,&DBlank))
      {
        if( ElementDisplay( Update, 5, DayPhrase, &IBlank, &Day))
        {
          if( ElementDisplay( Update, 6, DistanceToSun, &IBlank, &DoubleIn))
          {
            if( ElementDisplay( Update, 7, "Eccentricity", &IBlank, &Eccen))
            {
              if( ElementDisplay( Update, 8, "Argument of Perihelion", &IBlank, &PeriDeg))
              {
                if( ElementDisplay( Update, 9, "Longitude of Ascending Node", &IBlank, &NodeDeg))
                {
                  if( ElementDisplay( Update, 10, "Inclination", &IBlank, &InclDeg))
                  {
                    if( Planet == 11)
                    {
                      if( ElementDisplay( Update, 11, "Mean Anomaly", &IBlank, &MeanAn0Deg))
                      {
                        MeanAn0 = MeanAn0Deg*DegToRad;
                      }
                    }
                    PDist = SemiAxis = DoubleIn;  /* Arbitrary setting to avoid bad data trap  */
                    Peri = PeriDeg*DegToRad;
                    Node = NodeDeg*DegToRad;
                    Incl = InclDeg*DegToRad;
                  }
                }
              }
            }
          }
        }
      }
    }

    gotoxy( CoordFrame.Left+25, CoordFrame.Top+13);
    printf( "Is data correct? (y/n) ");
    Response = getch();
    if( Response == 'Y' || Response == 'y' || Response == Return)
      ValidData = Yes;
    else
      Update = Yes;
  }
  RemoveWindow( CoordFrame);

  CalcEpoch( Year, Month, Day);
  PeriDate = EpochDate = Epoch;
  DriftFlag = On;
  ManualInputFlag = On;               /* Need to display data  */
  UpdateJD();
}

Flag ElementDisplay( Flag Update, int Offset, char Phrase[], int* IElement, double* DElement)
{
  Flag A;

  gotoxy( CoordFrame.Left+12, CoordFrame.Top+Offset);
  printf( Phrase);
  gotoxy( CoordFrame.Left+48, CoordFrame.Top+Offset);
  if( Update)
  {
    if( *IElement)
      printf( "[%4d]                 ", *IElement);
    else
    {
      if( *DElement < 10.0)
        printf( "[%1.7f]                 ", *DElement);
      else if( *DElement < 100.0)
        printf( "[%2.6f]                 ", *DElement);
      else
        printf( "[%3.5f]                 ", *DElement);
    }
    gotoxy( CoordFrame.Left+60, CoordFrame.Top+Offset);
  }
  if( *IElement)
  {
    if( GetInt( &*IElement))
      A = 1;
  }
  else
  {
    if( GetDouble( &*DElement))
      A = 1;
  }

  return A;
}

void CalcAuxiliaryTerms( void)
{
  double F,G,H,P,Q,R;

/* Comet and Asteroid Auxiliary Constants - Calculate only once */

  F = cos( Node);
  G = sin( Node)*CosObliq;
  H = sin( Node)*SinObliq;
  P = -sin( Node)*cos( Incl);
  Q = cos( Node)*cos( Incl)*CosObliq - sin( Incl)*SinObliq;
  R = cos( Node)*cos( Incl)*SinObliq + sin( Incl)*CosObliq;

  A = atan2( F,P);
  if( A < 0.0) A += OneRev;
  B = atan2( G,Q);
  if( B < 0.0) B += OneRev;
  C = atan2( H,R);
  if( C < 0.0) C += OneRev;

  a = sqrt( F*F + P*P);
  b = sqrt( G*G + Q*Q);
  c = sqrt( H*H + R*R);
}

void CalcBodyRectCoords( void)
{
  double AbsEccen = fabs( 1.0 - Eccen);
  double EccenAn,TrueAn,RadiusVect,S;
  double MeanMotion,MeanAn;

  /* Calculate True Anomaly and Radius Vector */
  dJ = TimeInstant - PeriDate;  /* This is for Comets  */
  if( Eccen != 1)
  {
    if( Planet == 10)   /* A comet  */
    {
      SemiAxis = PDist/AbsEccen;
/* Note:  For a comet with a hyperbolic orbit, SemiAxis is infinite.  The
          above just gets MeanAn starting point for iteration  */
      MeanMotion = 0.9856076686/(SemiAxis*sqrt( SemiAxis));
      MeanAn = dJ*MeanMotion*DegToRad;
    }
    else               /* An asteroid  */
    {
      MeanMotion = 0.9856076686/(SemiAxis*sqrt( SemiAxis));
      dJ = TimeInstant - EpochDate;    /* Change dJ if asteroid  */
      MeanAn = MeanAn0 + dJ*MeanMotion*DegToRad;
    }
    MeanAn = fmod( MeanAn, OneRev);
    if( MeanAn > HalfRev)
      MeanAn -= OneRev;       /* MeanAn in range -Pi to +Pi  */
    EccenAn = Kepler( MeanAn);
    TrueAn = 2*atan( sqrt(( 1 + Eccen)/AbsEccen)*tan(EccenAn/2));
    if( Eccen < 1.0)
      RadiusVect = SemiAxis*( 1 - Eccen*cos( EccenAn));
    else
      RadiusVect = PDist*( 1+Eccen)/(1 + Eccen*cos( TrueAn));
  }
  else
  {
    S = Parabola();
    TrueAn = 2*atan( S);
    RadiusVect = PDist*( 1+S*S);
  }

  if( TrueAn < 0) TrueAn += OneRev;

/* Now get x,y,z for body  */
  x = RadiusVect*a*sin( A + Peri + TrueAn);
  y = RadiusVect*b*sin( B + Peri + TrueAn);
  z = RadiusVect*c*sin( C + Peri + TrueAn);
}

/* Following routine adapted from ProjectPluto  */
double Kepler( double MeanAn)
{
  double err,Thresh,Trial,Z;
  double EccenAn,AbsEccen = fabs( 1.0 - Eccen);

  if( !MeanAn)
    return( 0.0);

  if( Eccen < 0.3)
  {
    EccenAn = atan2( sin( MeanAn), cos( MeanAn) - Eccen);
    err = EccenAn - Eccen*sin( EccenAn) - MeanAn;
    EccenAn -= err/( 1.0 - Eccen*cos( EccenAn));
    return( EccenAn);
  }

  Thresh = .00000001*AbsEccen;
  if( Eccen > 0.8 && MeanAn < M_PI/3 || Eccen > 1.0)
  {
    Trial = MeanAn/AbsEccen;

    if( Trial*Trial > 6.0*AbsEccen)    /* Cubic term is dominant  */
    {
      if( MeanAn < M_PI)
        Trial = exp( log( 6.0*MeanAn)/3.0);
      else             /* Hyperbolic with 5th and higher-order terms dominant  */
      {
        Z = MeanAn/Eccen;
        Trial = log( Z + sqrt( Z*Z + 1.0));
      }
    }
    EccenAn = Trial;
  }

  if( Eccen < 1.0)
  {
    err = EccenAn - Eccen*sin( EccenAn) - MeanAn;
    while( fabs( err) > Thresh)
    {
      EccenAn -= err/( 1.0 - Eccen*cos( EccenAn));
      err = EccenAn - Eccen*sin( EccenAn) - MeanAn;
    }
  }
  else
  {
    err = Eccen*sinh( EccenAn) - EccenAn - MeanAn;
    while( fabs( err) > Thresh)
    {
      EccenAn -= err/( Eccen*cosh( EccenAn) - 1.0);
      err = Eccen*sinh( EccenAn) - EccenAn - MeanAn;
    }
  }
  return( EccenAn);


}

double Parabola( void)
{
  double G,W,Y,S;

  W = 0.03649116245*dJ/(PDist * sqrt( PDist));
  G = W/2.0;
  Y = pow( G + sqrt( G*G + 1.0), 1./3.);
  S = Y - 1.0/Y;
  return( S);
}

void AddCometDriftToDrift( void)
{
	Drift.Ra += CometDrift.Ra;
	Drift.Dec += CometDrift.Dec;
	CalcCommonDriftVars();
}

void RemoveCometDriftFromDrift( void)
{
	Drift.Ra -= CometDrift.Ra;
	Drift.Dec -= CometDrift.Dec;
   CometDrift.Ra = 0.0;     /* This routine gets called before next  */
   CometDrift.Dec = 0.0;    /* comet entered so need to clear values */
	CalcCommonDriftVars();
}

// calculate corrections for an epoch JD
void calcCorrectionsForEpochJD(double JD, struct Position* Pos)
{
	CalcPrecessionForJD( Pos, JD);
	CalcNutationAnnualAberrationForJD( Pos, JD);
	Pos->Ra += Pos->Nutation.Z + Pos->AnnualAberration.Z;
	Pos->Ra = ValidRad( Pos->Ra);
	Pos->Dec += Pos->Nutation.A + Pos->AnnualAberration.A;
}

void CalcPrecessionForJD( struct Position* Pos, const double JD)
{
	double PreRa, PreDec;
	double NewRa, NewDec;
	double T,t;
   double Zeta,Eta,Theta;
   double A,B,C;

/* Get starting epoch  */
   Epoch = JD2000 + (DataFileCoordYear - 2000.0)*365.25;

	T = (Epoch - JD2000)/36525.0;    /* Equal to 0.0 if Epoch = J2000.0  */
   t = (JD - Epoch)/36525.0;

	PreRa = Pos->Ra;
	PreDec = Pos->Dec;

   Zeta = (( 2306.2181 + 1.39656*T - 0.000139*T*T)*t
           + ( 0.30188 - 0.000344*T)*t*t + 0.017998*t*t*t)*ArcsecToRad;
   Eta  = (( 2306.2181 + 1.39656*T - 0.000139*T*T)*t
           + ( 1.09468 + 0.000066*T)*t*t + 0.018203*t*t*t)*ArcsecToRad;
   Theta = (( 2004.3109 - 0.8533*T - 0.000217*T*T)*t
           - ( 0.42665 + 0.000217*T)*t*t - 0.041833*t*t*t)*ArcsecToRad;

   A = cos( PreDec)*sin( PreRa + Zeta);
   B = cos( Theta)*cos( PreDec)*cos( PreRa + Zeta) - sin( Theta)*sin( PreDec);
   C = sin( Theta)*cos( PreDec)*cos( PreRa + Zeta) + cos( Theta)*sin( PreDec);

   NewRa = atan2( A,B) + Eta;
   NewRa = ValidRad( NewRa);
   NewDec = acos( sqrt( A*A + B*B));  /* This is abs( NewDec)  */
   if( C < 0.0)
     NewDec = -NewDec;                /* Get correct sign for NewDec  */

	Pos->Ra = NewRa;
	Pos->Dec = NewDec;
}

void CalcNutationAnnualAberrationForJD( struct Position* Pos, const double JD)
{
	double t;

   double sunMeanLongitude;
	double sunMeanAnomaly;
	double sunEquationOfTheCenter;
	double sunTrueLongitude;

	double moonMeanLongitude;
	// never used
	//double moonMeanAnomaly;

	double longitudeMoonAscendingNode;
	double longitudePerihelionEarthOrbit;
	double eccentricityEarthOrbit;
	double obliquityEcliptic;
	double meanObliquityEcliptic;

	double nutationLongitude;
   double nutationObliquity;

	double PreRa;
	double PreDec;
	double DeltaRa;
	double DeltaDec;

	t = (JD - JD2000) / 36525.;

	sunMeanLongitude = ValidRad((280.46646+36000.76983*t+.0003032*t*t)*DegToRad);
	moonMeanLongitude = ValidRad((218.3165 + 481267.8813*t)*DegToRad);
	eccentricityEarthOrbit = .016708634-.000042037*t-.0000001267*t*t;
	longitudePerihelionEarthOrbit = ValidRad(102.93735+1.71946*t+.00046*t*t)*DegToRad;
	sunMeanAnomaly = ValidRad((357.52911+35999.05029*t-.0001537*t*t)*DegToRad);
	// never used
	// moonMeanAnomaly = ValidRad((134.96298+477198.867398*t-0.0086972*t*t)*DegToRad);

	longitudeMoonAscendingNode = ValidRad((125.04452-1934.136261*t+.0020708*t*t+t*t*t/450000.)*DegToRad);
	meanObliquityEcliptic = (23.43929-46.8150/3600*t-.00059/3600*t*t+.001813/3600*t*t*t)*DegToRad;
	sunEquationOfTheCenter = (1.914602-.004817*t-.000014*t*t)*sin(sunMeanAnomaly)
	+(.019993-.000101*t)*sin(2*sunMeanAnomaly)
	+.000289*sin(3*sunMeanAnomaly);
	sunEquationOfTheCenter *= DegToRad;
	sunTrueLongitude = sunMeanLongitude + sunEquationOfTheCenter;

	nutationLongitude = -17.20*sin(longitudeMoonAscendingNode)
	-1.32*sin(2*sunMeanLongitude)
	-.23*sin(2* moonMeanLongitude)
	+.21*sin(2*longitudeMoonAscendingNode);

	nutationObliquity = 9.20*cos(longitudeMoonAscendingNode)
	+.57*cos(2*sunMeanLongitude)
	+.1*cos(2*moonMeanLongitude)
	-.09*cos(2*longitudeMoonAscendingNode);

	obliquityEcliptic = meanObliquityEcliptic + nutationObliquity*ArcsecToRad;

	// nutation

	PreRa = Pos->Ra;
	PreDec = Pos->Dec;

	DeltaRa = (cos(obliquityEcliptic)+sin(obliquityEcliptic)*sin(PreRa)*tan(PreDec))*nutationLongitude
	-(cos(PreRa)*tan(PreDec))*nutationObliquity;
	DeltaRa *= ArcsecToRad;

	DeltaDec = sin(obliquityEcliptic)*cos(PreRa)*nutationLongitude+sin(PreRa)*nutationObliquity;
	DeltaDec *= ArcsecToRad;

	Pos->Nutation.Z = DeltaRa;
	Pos->Nutation.A = DeltaDec;

	// annual aberration

	DeltaRa = -20.49552*((cos(PreRa)*cos(sunTrueLongitude)*cos(obliquityEcliptic)
	+ sin(PreRa)*sin(sunTrueLongitude)) / cos(PreDec))
	+ eccentricityEarthOrbit*20.49552*((cos(PreRa)*cos(longitudePerihelionEarthOrbit)*cos(obliquityEcliptic)
	+ sin(PreRa)*sin(longitudePerihelionEarthOrbit))/cos(PreDec));
	DeltaRa *= ArcsecToRad;

	DeltaDec = -20.49552*(cos(sunTrueLongitude)*cos(obliquityEcliptic)*(tan(obliquityEcliptic)*cos(PreDec)
	- sin(PreRa)*sin(PreDec))
	+ cos(PreRa)*sin(PreDec)*sin(sunTrueLongitude))
	+ eccentricityEarthOrbit*20.49552*(cos(longitudePerihelionEarthOrbit)*cos(obliquityEcliptic)*(tan(obliquityEcliptic)*cos(PreDec)
	- sin(PreRa)*sin(PreDec))+cos(PreRa)*sin(PreDec)*sin(longitudePerihelionEarthOrbit));
	DeltaDec *= ArcsecToRad;

	Pos->AnnualAberration.Z = DeltaRa;
	Pos->AnnualAberration.A = DeltaDec;
}

void ObjectNameTextAttr( const struct Frame F, const int Length, const int Select, const int TextAttr)
{
	int StartX, EndX;

	VidMemXY.Y = Select%8 + F.Top+1;
	StartX = F.Left + Length*(Select/8);
	EndX = StartX + Length;
	for( VidMemXY.X = StartX; VidMemXY.X < EndX;	VidMemXY.X++)
		Screen[VidMemXY.Y][VidMemXY.X].Attr = TextAttr;
}

void ReadRa( FILE* File, struct HMSH* V)
{
	char c = ' ';

	/* check for -0 hr */
	while( !feof( Input) && c != '+' && c != '-' && c != '.' && !(c >= '0' && c <= '9') && c != -1)
		c = fgetc( File);
	ungetc( c, File);

	fscanf( File, "%d", &V->Hr);
	fscanf( File, "%d", &V->Min);
	fscanf( File, "%d", &V->Sec);

	if( c == '-' || V->Hr < 0 || V->Min < 0 || V->Sec < 0)
		V->Sign = Minus;
	else
		V->Sign = Plus;

	if( V->Hr < 0)
		V->Hr = -V->Hr;
	if( V->Min < 0)
		V->Min = -V->Min;
	if( V->Sec < 0)
		V->Sec = -V->Sec;

	V->HundSec = 0;
}

void ReadDec( FILE* File, struct DMS* V)
{
	char c = ' ';

	/* check for -0 deg */
	while( c != '+' && c != '-' && c != '.' && !(c >= '0' && c <= '9') && c != -1)
		c = fgetc( File);
	ungetc( c, File);

	fscanf( File, "%d", &V->Deg);
	fscanf( File, "%d", &V->Min);
	fscanf( File, "%d", &V->Sec);

	if( c == '-' || V->Deg < 0 || V->Min < 0 || V->Sec < 0)
		V->Sign = Minus;
	else
		V->Sign = Plus;

	if( V->Deg < 0)
		V->Deg = -V->Deg;
	if( V->Min < 0)
		V->Min = -V->Min;
	if( V->Sec < 0)
		V->Sec = -V->Sec;
}

/* ability to page down through lists of unlimited size courtesy Dale Eason */
/* used by MenuCoordFile() to fill window until out of data from file */
Flag ObjectNames( struct Index I[], int* Count)
{
	int Ix = 0;

   if( strcmpi( Filename, "Planet.dat") == 0)
     FileType = CalcPlanetCoords;   /* Planet.dat is a dummy file with names only  */

	while( !feof( Input) && Ix < MaxCoord)
	{
     I[Ix].CurrentOffset = ftell( Input);

     switch( FileType)
     {
       case CalcPlanetCoords:
       {
         Planet = 3;

         ReadRaDec( Input);
         FReadStringToCharCountAndNewLine( Input, I[Ix].Name, sizeof( I->Name));
         break;
       }
       case ReadCometElements:
       {
         Planet = 10;

         if( !feof( Input))
           FReadStringToCharCountAndNewLine( Input, I[Ix].Name, sizeof( I->Name));
         break;
       }
       case ReadAsteroidElements:
       {
         Planet = 11;

         if( !feof( Input))
           FReadStringToCharCountAndNewLine( Input, I[Ix].Name, sizeof( I->Name));
         break;
       }
       default:
       {
         ReadRaDec( Input);   /* discard Ra and Dec info  */
         if( FileType == ReadCometAsteroidFile)
           ReadRaDec( Input);  /* discard drift info  */
         FReadStringToCharCountAndNewLine( Input, I[Ix].Name, sizeof( I->Name));
       }
     }
     Ix++;
     *Count = Ix;
   }
	if( feof( Input))
		(*Count)--;

	if( *Count)
		return True;
	else
		return False;
}

void ProcessMenuRestoreLastObject( void)
{
	In = LastDataObject;
	DisplayIn( "last data object", NameBlanks);
	ProcessHPEvents();
}

void LoadInputFileIntoMemory( void)
{
	struct HMSH R;
	struct DMS D;

	Input = fopen( InputFile, "r");
	if( Input != NULL)
	{
		if( FirstRaDecInit != NULL)
			FreeAllRaDecInit();

		while( !feof( Input))
		{
			if( !feof( Input))
				ReadRa( Input, &R);
			if( !feof( Input))
				ReadDec( Input, &D);
			if( !feof( Input))
			{
				FReadToNewLine( Input);
				RaDecInit = InitRaDecInit();
				AddRaDecInit( RaDecInit);
				CalcRadFromHMSH( &RaDecInit->Ra, R);
				CalcRadFromDMS( &RaDecInit->Dec, D);
			}
		}
		fclose( Input);
	}
}

Flag ProcessMenuDataFileClosest( Flag FilenameSetFlag, Flag HowToMatchFlag)
{
	struct Position P, BestP;
	char Name[ObjectNameSize], BestName[ObjectNameSize];
	double Sep;
	double BestSep = MAXDOUBLE;
	Flag CheckBestFlag;
	Flag BestFoundFlag = False;

	if( HowToMatchFlag == DoNotMatchInputFile)
		LoadInputFileIntoMemory();

	/* either Filename is already set, or call a function where user selects a Filename from a file
   name pattern */
	if( FilenameSetFlag || SelectDataFilename( DataFile))
	{
		Input = fopen( Filename, "r");
		if( Input == NULL)
			BadExit( strcat( "Could not open ", Filename));

		P.SidT = Current.SidT;
		while( !feof( Input))
		{
			if( !feof( Input))
				ReadRa( Input, &P.RaHMSH);
			if( !feof( Input))
				ReadDec( Input, &P.DecDMS);
			if( !feof( Input))
			{
				if( FReadStringToCharCountOrNewLine( Input, Name, sizeof( Name)))
					;
				else
					FReadToNewLine( Input);
				CalcRadFromHMSH( &P.Ra, P.RaHMSH);
				CalcRadFromDMS( &P.Dec, P.DecDMS);
				Sep = CalcEquatAngularSep( &Current, &P);
				if( Sep < BestSep)
				{
					switch( HowToMatchFlag)
					{
						case ExactInputFieldsMatchOK:
							CheckBestFlag = Yes;
							break;
						case AvoidMatchInputFields:
							if( P.Ra!=In.Ra || P.Dec!=In.Dec)
								CheckBestFlag = Yes;
							else
								CheckBestFlag = No;
							break;
						case DoNotMatchInputFile:
							CheckBestFlag = Yes;
							RaDecInit = FirstRaDecInit;
							while( RaDecInit != NULL && CheckBestFlag)
							{
								if( P.Ra==RaDecInit->Ra && P.Dec==RaDecInit->Dec)
									CheckBestFlag = No;
								RaDecInit = RaDecInit->NextRaDecInit;
							}
					}
					if( CheckBestFlag)
					{
						BestFoundFlag = True;
						BestSep = Sep;
						BestP = P;
						strcpy( BestName, Name);
					}
				}
			}
		}
		if( BestFoundFlag)
		{
			In.RaHMSH = BestP.RaHMSH;
			In.DecDMS = BestP.DecDMS;
			CalcRadFromHMSH( &In.Ra, In.RaHMSH);
			CalcRadFromDMS( &In.Dec, In.DecDMS);
			DisplayIn( Filename, BestName);
			LastDataObject = In;
		}
		fclose( Input);
	}
	if( HowToMatchFlag == DoNotMatchInputFile)
		FreeAllRaDecInit();

	return BestFoundFlag;
}

/* remove spaces from the end of a string generated with Gets() (in common.c) */
void RemoveBlanks( char* s)
{
	int i = strlen( s);

	while( isspace( s[--i]))
	{
		if (i < 0)
			break;
		s[i]=EndOfStr;
	}
}

/* attempts to find string str in file fname */
Flag FindStr( const char* str, char* fname, int* Rec)
{
	int c;
	int Left, Top;

	*Rec = 0;

	if( IsProgramFile( Ffblk.ff_name))
		return False;

	if( (Input = fopen( fname, "r")) != NULL)
	{
		while( fgets( StrBuf2, sizeof( StrBuf2), Input) != NULL)
		{
			(*Rec)++;
			if (strstr( strlwr( StrBuf2), str) != NULL)
			{
				Left = MsgFrame.Left + 2;
				Top = MsgFrame.Top + 4;
				gotoxy( Left, Top);
				for( Ix = 0; Ix < 50; Ix++)
					printf( " ");
				gotoxy( Left, Top);
				printf( "Found %s", StrBuf2);
				gotoxy( Left, Top + 1);
				for( Ix = 0; Ix < 30; Ix++)
					printf( " ");
				gotoxy( Left, Top + 1);
				printf( "in %s, rec #%d ", fname, *Rec);
				gotoxy( Left, Top + 3);
				printf( "'y' to select, esc to terminate, any key to continue");
				c = getch();
				switch( c)
				{
					case 'Y': case 'y':
						return (int) c;
					case Esc:
						fclose( Input);
						return Esc;
				}
			}
		}
	}
	fclose( Input);
	return False;
}

Flag FindFile( const char* Path, const char* Search, char* File, int* Rec)
{
	char path[MAXPATH];
	char drive[MAXDRIVE];
	char dir[MAXDIR];
	char file[MAXFILE];
	char ext[MAXEXT];
	char xdr[MAXDRIVE];
	char xdi[MAXDIR];
	int done;
	int FindStrResults;

	fnsplit( Path, drive, dir, file, ext);
	if( strlen( file) == 0)
		strcpy( file, "*");
	if( stricmp( ext, ".dat") != 0)
		strcpy( ext,".dat");
	fnmerge( path, drive, dir, file, ext);

	done = findfirst( path, &Ffblk, 0);
	while( !done)
	{
		fnsplit( Ffblk.ff_name, xdr, xdi, file, ext);
		fnmerge( path, drive, dir, file, "dat");
		FindStrResults = FindStr( Search, path, Rec);
		switch( FindStrResults)
		{
			case 'Y': case 'y':
				fnmerge( File, "", "", file, ext);
				return True;
			case Esc:
				return False;
			default:
				done = findnext( &Ffblk);
		}
	}
	return False;
}

Flag ProcessMenuSearch( void)
{
	int Left, Top;
	static char Path[26] = "*.dat";
	char InputPath[26];
	char Search[30];
	char InputSearch[30];
	Flag UserResponseFlag;
	Flag ResultFlag = False;
	int Rec;
	char Name[ObjectNameSize];

	WriteWindow( MsgFrame);
	Left = MsgFrame.Left + 2;
	Top = MsgFrame.Top + 2;
	gotoxy( Left, Top);
	printf( "Data file path (%s): ", Path);
	UserResponseFlag = Gets( InputPath, 25);
	if( UserResponseFlag != UserEscaped)
	{
		RemoveBlanks( InputPath);
		if( UserResponseFlag && strlen( InputPath) > 0)
			strcpy( Path, InputPath);

		gotoxy( Left, Top + 1);
		printf( "Search string: ");
		UserResponseFlag = Gets( InputSearch, 29);
		if( UserResponseFlag != UserEscaped)
		{
			strcpy( Search, InputSearch);
			RemoveBlanks( Search);
			if( FindFile( Path, strlwr( Search), Filename, &Rec))
			{
				rewind( Input);
				for( Ix = 0; Ix < Rec-1; Ix++)
					FReadToNewLine( Input);
				if( !feof( Input))
					ReadRa( Input, &In.RaHMSH);
				if( !feof( Input))
					ReadDec( Input, &In.DecDMS);
				if( !feof( Input))
					if( FReadStringToCharCountOrNewLine( Input, Name, sizeof( Name)))
						;
				fclose( Input);
				CalcRadFromHMSH( &In.Ra, In.RaHMSH);
				CalcRadFromDMS( &In.Dec, In.DecDMS);
				ResultFlag = True;
			}
		}
	}
	RemoveWindow(MsgFrame);
	if( ResultFlag)
	{
		DisplayIn( Filename, Name);
		LastDataObject = In;
	}
	return ResultFlag;
}

void DisplayIn( char* FileName, char* ObjectName)
{
   double HorizonLimit;

	TextAttr = CurrentText;
	VidMemXY = DisplayXY[DisplayInputAlt];
	VidMemDeg( In.Alt);
	VidMemXY = DisplayXY[DisplayInputAz];
	VidMemDeg( In.Az);
	VidMemXY = DisplayXY[DisplayInputRa];
	VidMemRaHMS( &In);
	VidMemXY = DisplayXY[DisplayInputDec];
	VidMemDecDMS( &In);

	TextAttr = CurrentText;
	VidMemXY = DisplayXY[DisplayFile];
	sprintf( StrBuf, "%s", NameBlanks);
	WriteStrBufToScreen_f_ptr();
	VidMemXY = DisplayXY[DisplayFile];
	sprintf( StrBuf, "%s", FileName);
	/* limit # of displayed chars */
	StrBuf[23] = EndOfStr;
	WriteStrBufToScreen_f_ptr();

	VidMemXY = DisplayXY[DisplayObject];
	sprintf( StrBuf, NameBlanks);
	WriteStrBufToScreen_f_ptr();
	VidMemXY = DisplayXY[DisplayObject];
	sprintf( StrBuf, "%s", ObjectName);
	/* limit # of displayed chars */
	StrBuf[23] = EndOfStr;
	WriteStrBufToScreen_f_ptr();

	WriteToInputFile( ObjectName);

   if( Two.Init && HorizonLimitFlag)
   {
     HorizonLimit = atan2( -cos( In.Ra - SidT)*ConfigCosLat, fabs( ConfigSinLat));

     if( ConfigSinLat > 0 && In.Dec < HorizonLimit || ConfigSinLat <= 0 && In.Dec > -HorizonLimit)
     {
       WriteWindow( MsgFrame);
       gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 2);
       printf( "Selected Object Below Horizon");
       sound( 500);
       delay( 100);
       sound( 1000);
	    delay( 200);
	    sound( 500);
	    delay( 100);
	    nosound();
       gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 4);
       ContMsgRoutine();
       RemoveWindow( MsgFrame);
     }
   }
}

void WriteToInputFile( char* ObjectName)
{
	time_t t;

	if (!WriteInitHistoryFlag)
   return;

	Output = fopen( InputFile, "a");
	if( Output == NULL)
		BadExit( strcat( "Could not append to ", InputFile));

	if( In.DecDMS.Sign == Minus)
	{
		 In.DecDMS.Deg = -In.DecDMS.Deg;
		 In.DecDMS.Min = -In.DecDMS.Min;
		 In.DecDMS.Sec = -In.DecDMS.Sec;
	}

	time( &t);

	fprintf( Output, " %d %d %d %d %d %d %s: %s\n",
	In.RaHMSH.Hr, In.RaHMSH.Min, In.RaHMSH.Sec,
	In.DecDMS.Deg, In.DecDMS.Min, In.DecDMS.Sec,
	ObjectName, ctime( &t));

	if( In.DecDMS.Sign == Minus)
	{
		 In.DecDMS.Deg = -In.DecDMS.Deg;
		 In.DecDMS.Min = -In.DecDMS.Min;
		 In.DecDMS.Sec = -In.DecDMS.Sec;
	}

	fclose( Output);
}

void ProcessMenuWriteInputComment( void)
{
	char Comment[ObjectNameSize];

	WriteWindow( MsgFrame);
	VidMemXY.X = MsgFrame.Left + 2;
	VidMemXY.Y = MsgFrame.Top + 3;
	gotoxy( VidMemXY.X, VidMemXY.Y);
	printf( "write a comment to the input.dat file:");
	gotoxy( VidMemXY.X+20, VidMemXY.Y+2);
	Gets( Comment, sizeof( Comment)-1);
	RemoveWindow( MsgFrame);
	WriteToInputFile( Comment);
}

void ResetScrollVars( void)
{
	AutoScrollAlertFlag = Yes;
	FreeAllScroll();
}

void LoadScrollFileFromFile( void)
{
	Input = fopen( ScrollFilename, "r");
	if( Input == NULL)
		BadExit( strcat( "Could not open ", ScrollFilename));

	ResetScrollVars();
	while( !feof( Input))
	{
		// TestReadInputParseString( Input);
		ReadInputParseString( Input);
		ParseStringBuildCmd();
	}
	fclose( Input);
	if( TotalScrollCount)
	{
		DisplayIn( ScrollFilename, NameBlanks);
		ScrollLoaded = Yes;
		DisplayScrollCountsOnScreen();
	}
	else
	{
		PressKeyToContMsg( "scroll file contains no scroll commands");
		DisplayIn( NameBlanks, NameBlanks);
	}
}

void LoadScrollFileFromString( char* Str)
{
	strcpy( ParsedString.Str, Str);
	LoadScrollFileFromParsedString();
}

void LoadScrollFileFromParsedString( void)
{
	ResetScrollVars();
	ParseString();
	ParseStringBuildCmd();
	if( TotalScrollCount)
	{
		DisplayIn( "from string", NameBlanks);
		ScrollLoaded = Yes;
		DisplayScrollCountsOnScreen();
	}
	else
	{
		PressKeyToContMsg( "scroll string contains no scroll commands");
		DisplayIn( NameBlanks, NameBlanks);
	}
}

void ReadRaFromParsedString( struct HMSH* V)
{
	V->Sign = Plus;

	if( ParsedString.Str[ParsedString.BegIx[ParsedString.ReadStrCnt]] == Dash)
		V->Sign = Minus;
	sscanf( &ParsedString.Str[ParsedString.BegIx[ParsedString.ReadStrCnt++]], "%d", &V->Hr);

	if( ParsedString.Str[ParsedString.BegIx[ParsedString.ReadStrCnt]] == Dash)
		V->Sign = Minus;
	sscanf( &ParsedString.Str[ParsedString.BegIx[ParsedString.ReadStrCnt++]], "%d", &V->Min);

	if( ParsedString.Str[ParsedString.BegIx[ParsedString.ReadStrCnt]] == Dash)
		V->Sign = Minus;
	sscanf( &ParsedString.Str[ParsedString.BegIx[ParsedString.ReadStrCnt++]], "%d", &V->Sec);
	V->HundSec = 0;

	if( V->Hr < 0)
		V->Hr = -V->Hr;
	if( V->Min < 0)
		V->Min = -V->Min;
	if( V->Sec < 0)
		V->Sec = -V->Sec;
}

void ReadDecFromParsedString( struct DMS* V)
{
	if( ParsedString.Str[ParsedString.BegIx[ParsedString.ReadStrCnt]] == Dash)
		V->Sign = Minus;
	sscanf( &ParsedString.Str[ParsedString.BegIx[ParsedString.ReadStrCnt++]], "%d", &V->Deg);

	if( ParsedString.Str[ParsedString.BegIx[ParsedString.ReadStrCnt]] == Dash)
		V->Sign = Minus;
	sscanf( &ParsedString.Str[ParsedString.BegIx[ParsedString.ReadStrCnt++]], "%d", &V->Min);

	if( ParsedString.Str[ParsedString.BegIx[ParsedString.ReadStrCnt]] == Dash)
		V->Sign = Minus;
	sscanf( &ParsedString.Str[ParsedString.BegIx[ParsedString.ReadStrCnt++]], "%d", &V->Sec);

	if( V->Deg < 0)
		V->Deg = -V->Deg;
	if( V->Min < 0)
		V->Min = -V->Min;
	if( V->Sec < 0)
		V->Sec = -V->Sec;
}

void ReadScrollEquatCoordFromParsedString( struct SCROLL* Scroll)
{
	ReadRaFromParsedString( &Scroll->P->RaHMSH);
	CalcRadFromHMSH( &Scroll->P->Ra, Scroll->P->RaHMSH);
	ReadDecFromParsedString( &Scroll->P->DecDMS);
	CalcRadFromDMS( &Scroll->P->Dec, Scroll->P->DecDMS);
	sscanf( &ParsedString.Str[ParsedString.BegIx[ParsedString.ReadStrCnt++]], "%lf", &Scroll->Sec);
	BuildScrollNoteFromParsedString( Scroll);
}

void BuildScrollNoteFromParsedString( struct SCROLL* Scroll)
{
	for( Ix = 0; Ix < sizeof( Scroll->Note)-1 && Ix+ParsedString.BegIx[ParsedString.ReadStrCnt] < MaxStrIx; Ix++)
	{
		Scroll->Note[Ix] = ParsedString.Str[Ix+ParsedString.BegIx[ParsedString.ReadStrCnt]];
		if( IsWhiteSpace( Scroll->Note[Ix]))
			Scroll->Note[Ix] = ' ';
	}
	for( ; Ix < sizeof( Scroll->Note); Ix++)
		Scroll->Note[Ix] = EndOfStr;
}

void ParseStringBuildCmd( void)
{
	SCROLL_TYPE ScrollType;
	struct SCROLL* Scroll;

	ParsedString.ReadStrCnt = 0;

	if( ParsedString.Str[ParsedString.BegIx[ParsedString.ReadStrCnt]] != ';' &&
		 ParsedString.Str[ParsedString.BegIx[ParsedString.ReadStrCnt]] != '\'')
	{
		ScrollType = no_type;
		strcpy( Name, &ParsedString.Str[ParsedString.BegIx[ParsedString.ReadStrCnt++]]);

		if( (strncmpi( Name, "drift_altaz", sizeof( Name))) == 0)
			ScrollType = drift_altaz;
		if( (strncmpi( Name, "drift_equat", sizeof( Name))) == 0)
			ScrollType = drift_equat;
		if( (strncmpi( Name, "abs_altaz", sizeof( Name))) == 0)
			ScrollType = absolute_altaz;
		if( (strncmpi( Name, "off_altaz", sizeof( Name))) == 0)
			ScrollType = offset_altaz;
		if( (strncmpi( Name, "a", sizeof( Name))) == 0)
			ScrollType = absolute_equat;
		if( (strncmpi( Name, "o", sizeof( Name))) == 0)
			ScrollType = offset_equat;
		if( (strncmpi( Name, "1", sizeof( Name))) == 0)
			ScrollType = init1;
		if( (strncmpi( Name, "2", sizeof( Name))) == 0)
			ScrollType = init2;
		if( (strncmpi( Name, "3", sizeof( Name))) == 0)
			ScrollType = init3;
		if( (strncmpi( Name, "1i", sizeof( Name))) == 0)
			ScrollType = init1usinginput;
		if( (strncmpi( Name, "2i", sizeof( Name))) == 0)
			ScrollType = init2usinginput;
		if( (strncmpi( Name, "3i", sizeof( Name))) == 0)
			ScrollType = init3usinginput;
		if( (strncmpi( Name, "f1", sizeof( Name))) == 0)
			ScrollType = file1;
		if( (strncmpi( Name, "f2", sizeof( Name))) == 0)
			ScrollType = file2;
		if( (strncmpi( Name, "f3", sizeof( Name))) == 0)
			ScrollType = file3;
		if( (strncmpi( Name, "trackon", sizeof( Name))) == 0)
			ScrollType = trackon;
		if( (strncmpi( Name, "trackoff", sizeof( Name))) == 0)
			ScrollType = trackoff;
		if( (strncmpi( Name, "auto_scroll_alert_off", sizeof( Name))) == 0)
			ScrollType = auto_scroll_alert_off;
		if( (strncmpi( Name, "analyze", sizeof( Name))) == 0)
			ScrollType = analyze;
		if( (strncmpi( Name, "alt_offset", sizeof( Name))) == 0)
			ScrollType = alt_offset;
		if( (strncmpi( Name, "auto_scroll", sizeof( Name))) == 0)
			ScrollType = auto_scroll;
		if( (strncmpi( Name, "auto_scroll_off", sizeof( Name))) == 0)
			ScrollType = auto_scroll_off;
		if( (strncmpi( Name, "move_zero_pec", sizeof( Name))) == 0)
			ScrollType = move_zero_pec;
		if( (strncmpi( Name, "save1", sizeof( Name))) == 0)
			ScrollType = save1;
		if( (strncmpi( Name, "restore1", sizeof( Name))) == 0)
			ScrollType = restore1;
		if( (strncmpi( Name, "save2", sizeof( Name))) == 0)
			ScrollType = save2;
		if( (strncmpi( Name, "restore2", sizeof( Name))) == 0)
			ScrollType = restore2;
		if( (strncmpi( Name, "move_equat", sizeof( Name))) == 0)
			ScrollType = move_equat;
		if( (strncmpi( Name, "msarcsecsec", sizeof( Name))) == 0)
			ScrollType = msarcsecsec;
		if( (strncmpi( Name, "handpad_mode", sizeof( Name))) == 0)
			ScrollType = handpad_mode;
		if( (strncmpi( Name, "hs", sizeof( Name))) == 0)
			ScrollType = halfstep;
		if( (strncmpi( Name, "new_equat", sizeof( Name))) == 0)
			ScrollType = new_equat;
		if( (strncmpi( Name, "new_altaz", sizeof( Name))) == 0)
			ScrollType = new_altaz;
		if( (strncmpi( Name, "set_equat", sizeof( Name))) == 0)
			ScrollType = set_equat;
		if( (strncmpi( Name, "set_altaz", sizeof( Name))) == 0)
			ScrollType = set_altaz;
		if( (strncmpi( Name, "reset_equat", sizeof( Name))) == 0)
			ScrollType = reset_equat;
		if( (strncmpi( Name, "reset_altaz", sizeof( Name))) == 0)
			ScrollType = reset_altaz;
		if( (strncmpi( Name, "reset_home", sizeof( Name))) == 0)
			ScrollType = reset_home;
		if( (strncmpi( Name, "data_file", sizeof( Name))) == 0)
			ScrollType = data_file;
		if( (strncmpi( Name, "move_file", sizeof( Name))) == 0)
			ScrollType = move_file;
		if( (strncmpi( Name, "prompt", sizeof( Name))) == 0)
			ScrollType = prompt;

		switch( ScrollType)
		{
			case drift_altaz:
			case absolute_altaz:
			case offset_altaz:
			case new_altaz:
				Scroll = InitScroll( Yes, No);
				Scroll->ScrollType = ScrollType;
				sscanf( &ParsedString.Str[ParsedString.BegIx[ParsedString.ReadStrCnt++]], "%lf", &Scroll->P->Alt);
				/* 9999 means use current value when scroll file is executing, so leave 9999 intact*/
				if( Scroll->P->Alt == 9999)
					;
				else
					Scroll->P->Alt *= DegToRad;
				sscanf( &ParsedString.Str[ParsedString.BegIx[ParsedString.ReadStrCnt++]], "%lf", &Scroll->P->Az);
				if( Scroll->P->Az == 9999)
					;
				else
					Scroll->P->Az *= DegToRad;
				sscanf( &ParsedString.Str[ParsedString.BegIx[ParsedString.ReadStrCnt++]], "%lf", &Scroll->Sec);
				BuildScrollNoteFromParsedString( Scroll);
				AddScroll( Scroll);
				break;
			case drift_equat:
			case absolute_equat:
			case offset_equat:
			case new_equat:
			case init1:
			case init2:
			case init3:
				Scroll = InitScroll( Yes, No);
				Scroll->ScrollType = ScrollType;
				ReadScrollEquatCoordFromParsedString( Scroll);
				AddScroll( Scroll);
				break;
			case init1usinginput:
			case init2usinginput:
			case init3usinginput:
			case file1:
			case file2:
			case file3:
			case trackon:
			case trackoff:
			case auto_scroll_alert_off:
			case analyze:
			case alt_offset:
			case auto_scroll:
			case auto_scroll_off:
			case move_zero_pec:
			case save1:
			case restore1:
			case save2:
			case restore2:
         case move_equat:
			case set_equat:
			case set_altaz:
			case reset_equat:
			case reset_altaz:
			case reset_home:
			case data_file:
			case move_file:
			case prompt:
				Scroll = InitScroll( No, No);
				Scroll->ScrollType = ScrollType;
				BuildScrollNoteFromParsedString( Scroll);
				AddScroll( Scroll);
				break;
			case msarcsecsec:
			case handpad_mode:
				Scroll = InitScroll( No, No);
				Scroll->ScrollType = ScrollType;
				sscanf( &ParsedString.Str[ParsedString.BegIx[ParsedString.ReadStrCnt++]], "%d", &Scroll->i1);
				BuildScrollNoteFromParsedString( Scroll);
				AddScroll( Scroll);
				break;
			case halfstep:
				Scroll = InitScroll( No, Yes);
				Scroll->ScrollType = ScrollType;
				sscanf( &ParsedString.Str[ParsedString.BegIx[ParsedString.ReadStrCnt++]], "%ld", &Scroll->L->A);
				sscanf( &ParsedString.Str[ParsedString.BegIx[ParsedString.ReadStrCnt++]], "%ld", &Scroll->L->Z);
				BuildScrollNoteFromParsedString( Scroll);
				AddScroll( Scroll);
				break;
		}
	}
}

void DisplayScrollCountsOnScreen( void)
{
	TextAttr = CurrentText;
	VidMemXY = DisplayXY[DisplayScrollCounts];
	sprintf( StrBuf, "%3d/%3d", CurrentScrollCount, TotalScrollCount);
	WriteStrBufToScreen_f_ptr();
}

void ClearScrollCountsDisplayArea( void)
{
	TextAttr = CurrentText;
	VidMemXY = DisplayXY[DisplayScrollCounts];
	sprintf( StrBuf, "       ");
	WriteStrBufToScreen_f_ptr();
}

void ProcessScroll( void)
{
	int TicksForMove;
	int TickCount;
	struct AZDouble MovePerTick;
	struct AZDouble StartAltaz;


	if( !RestartScrollFlag)
	{
		ScrollUnderway = Yes;
		AutoScrollFlag = No;
		WriteHandpadStatus();
		CurrentScroll = NULL;
		/* Resets CurrentScrollCount on restart with LeftKey */
		CurrentScrollCount = 0;
	}

	RestartScrollFlag = No;

	while( ScrollFlag)
	{
		AlignMs_f_ptr();

		/* get the next Scroll */
		if( CurrentScroll == NULL)
			CurrentScroll = FirstScroll;
		else
			CurrentScroll = CurrentScroll->NextScroll;
		/* if still NULL (end of scroll tour) and auto repeat set, start over */
		if( CurrentScroll == NULL && HandpadFlag == ScrollAutoTour)
		{
			CurrentScroll = FirstScroll;
			CurrentScrollCount = 0;
		}

		/* if finished, clear out display scroll count area, else continue on with scroll commands */
		if( CurrentScroll == NULL)
		{
			ScrollFlag = No;
			CurrentScrollCount = 0;
			ClearScrollCountsDisplayArea();
		}
		else
		{
			CurrentScrollCount++;
			DisplayScrollCountsOnScreen();
			if( !ScrollAutoTour || (ScrollAutoTour && AutoScrollAlertFlag))
			{
				/* give 1/2 sec to move hand pad mode switch back to center position, otherwise moving
				hand paddle switch back to center position while the slew is starting will result in
				cancelled slew */
				sound( 200);
				delay( 55);
				nosound();
				delay( 445);
			}

			Buttons = Off;
			ReadHandpad_f_ptr();

			CurrentScrollFlag = Yes;

			/* display scroll action to be done */
			VidMemXY = DisplayXY[DisplayObject];
			sprintf( StrBuf, NameBlanks);
			WriteStrBufToScreen_f_ptr();
			TextAttr = CurrentText;
			VidMemXY = DisplayXY[DisplayObject];
			switch( CurrentScroll->ScrollType)
			{
				case drift_altaz:
					sprintf( StrBuf, "drfaz:%s", CurrentScroll->Note);
					break;
				case offset_altaz:
					sprintf( StrBuf, "offaz:%s", CurrentScroll->Note);
					break;
				case absolute_altaz:
					sprintf( StrBuf, "absaz:%s", CurrentScroll->Note);
					break;
				case drift_equat:
					sprintf( StrBuf, "drfeq:%s", CurrentScroll->Note);
					break;
				case offset_equat:
					sprintf( StrBuf, "offeq:%s", CurrentScroll->Note);
					break;
				case absolute_equat:
					sprintf( StrBuf, "abseq:%s", CurrentScroll->Note);
					break;
				case init1:
					sprintf( StrBuf, "init1:%s", CurrentScroll->Note);
					break;
				case init2:
					sprintf( StrBuf, "init2:%s", CurrentScroll->Note);
					break;
				case init3:
					sprintf( StrBuf, "init3:%s", CurrentScroll->Note);
					break;
				case init1usinginput:
					sprintf( StrBuf, "in1in:%s", CurrentScroll->Note);
					break;
				case init2usinginput:
					sprintf( StrBuf, "in2in:%s", CurrentScroll->Note);
					break;
				case init3usinginput:
					sprintf( StrBuf, "in3in:%s", CurrentScroll->Note);
					break;
				case file1:
					sprintf( StrBuf, "file1:%s", CurrentScroll->Note);
					break;
				case file2:
					sprintf( StrBuf, "file2:%s", CurrentScroll->Note);
					break;
				case file3:
					sprintf( StrBuf, "file3:%s", CurrentScroll->Note);
					break;
				case trackon:
					sprintf( StrBuf, "tckon:%s", CurrentScroll->Note);
					break;
				case trackoff:
					sprintf( StrBuf, "tckof:%s", CurrentScroll->Note);
					break;
				case auto_scroll_alert_off:
					sprintf( StrBuf, "autof:%s", CurrentScroll->Note);
					break;
				case analyze:
					sprintf( StrBuf, "analy:%s", CurrentScroll->Note);
					break;
				case alt_offset:
					sprintf( StrBuf, "altof:%s", CurrentScroll->Note);
					break;
				case auto_scroll:
					sprintf( StrBuf, "autos:%s", CurrentScroll->Note);
					break;
				case auto_scroll_off:
					sprintf( StrBuf, "auts0:%s", CurrentScroll->Note);
					break;
				case move_zero_pec:
					sprintf( StrBuf, "m0pec:%s", CurrentScroll->Note);
					break;
				case save1:
					sprintf( StrBuf, "save1:%s", CurrentScroll->Note);
					break;
				case restore1:
					sprintf( StrBuf, "rest1:%s", CurrentScroll->Note);
					break;
				case save2:
					sprintf( StrBuf, "save2:%s", CurrentScroll->Note);
					break;
				case restore2:
					sprintf( StrBuf, "rest2:%s", CurrentScroll->Note);
					break;
				case move_equat:
					sprintf( StrBuf, "move_equat:%s", CurrentScroll->Note);
					break;
				case msarcsecsec:
					sprintf( StrBuf, "msarc:%s", CurrentScroll->Note);
					break;
				case handpad_mode:
					sprintf( StrBuf, "hndpd:%s", CurrentScroll->Note);
					break;
				case halfstep:
					sprintf( StrBuf, "hstep:%s", CurrentScroll->Note);
					break;
				case new_equat:
					sprintf( StrBuf, "newEQ:%s", CurrentScroll->Note);
					break;
				case new_altaz:
					sprintf( StrBuf, "newAZ:%s", CurrentScroll->Note);
					break;
				case set_equat:
					sprintf( StrBuf, "setEQ:%s", CurrentScroll->Note);
					break;
				case set_altaz:
					sprintf( StrBuf, "setAZ:%s", CurrentScroll->Note);
					break;
				case reset_equat:
					sprintf( StrBuf, "rstEQ:%s", CurrentScroll->Note);
					break;
				case reset_altaz:
					sprintf( StrBuf, "rstAZ:%s", CurrentScroll->Note);
					break;
				case reset_home:
					sprintf( StrBuf, "rstHm:%s", CurrentScroll->Note);
					break;
				case data_file:
					sprintf( StrBuf, "in file:%s", CurrentScroll->Note);
					break;
				case move_file:
					sprintf( StrBuf, "mv file:%s", CurrentScroll->Note);
					break;
				case prompt:
					sprintf( StrBuf, "%s", CurrentScroll->Note);
					break;
				default:
					sprintf( StrBuf, "unkno:", CurrentScroll->Note);
			}
			/* limit # of displayed chars */
			StrBuf[23] = EndOfStr;
			WriteStrBufToScreen_f_ptr();

			switch( CurrentScroll->ScrollType)
			{
				case drift_altaz:
					/* drift expected to be in units per minute of time */
					if( CurrentScroll->Sec <= 0)
						CurrentScroll->Sec = 60;
					Drift.Alt = CurrentScroll->P->Alt/(60.*CurrentScroll->Sec);
					Drift.Az = CurrentScroll->P->Az/(60.*CurrentScroll->Sec);
					CalcCommonDriftVars();
					DisplayDrift();
					break;
				case drift_equat:
					/* drift expected to be in units per hour of time */
					if( CurrentScroll->Sec <= 0)
						CurrentScroll->Sec = 3600;
					Drift.Ra = CurrentScroll->P->Ra/(3600.*CurrentScroll->Sec);
					Drift.Dec = CurrentScroll->P->Dec/(3600.*CurrentScroll->Sec);
					CalcCommonDriftVars();
					DisplayDrift();
					break;
				case offset_altaz:
				case absolute_altaz:
					/* 9999 means use current value */
					if( CurrentScroll->P->Alt == 9999)
						CurrentScroll->P->Alt = Current.Alt;
					if( CurrentScroll->P->Az == 9999)
						CurrentScroll->P->Az = Current.Az;
					if( CurrentScroll->Sec > 0)
					{
						/* spread move over CurrentScroll->Sec time, calc # of ticks to use */
						TicksForMove = CurrentScroll->Sec * ClockTicksSec;
						/* if offset coordinates */
						if( CurrentScroll->ScrollType == offset_altaz)
						{
							MovePerTick.A = CurrentScroll->P->Alt / (double) TicksForMove;
							MovePerTick.Z = CurrentScroll->P->Az / (double) TicksForMove;
						}
						/* else absolute coordinates */
						else
						{
							MovePerTick.A = (CurrentScroll->P->Alt - Current.Alt) / (double) TicksForMove;
							MovePerTick.Z = (CurrentScroll->P->Az - Current.Az) / (double) TicksForMove;
						}
						TickCount = 0;

						/* if microstepping not possible, then slew: otherwise a total of TicksForMove
						slews will occur */
						if( fabs( MovePerTick.A) > MaxMsDistanceRadTick.A ||
						fabs( MovePerTick.Z) > MaxMsDistanceRadTick.Z)
						{
							Delta.A = CurrentScroll->P->Alt - Current.Alt;
							Delta.Z = CurrentScroll->P->Az - Current.Az;
							/* slew called from here: */
							SetDirDistanceStepsThenMove();
						}
						else
						{
							StartAltaz.A = Current.Alt;
							StartAltaz.Z = Current.Az;
							/* keep control here, microstepping to 'scroll to' position */
							while( TickCount < TicksForMove)
							{
								TickCount++;

								/* compare this with the similar microstep scrolling in offset_equat/
								absolute_equat: here altaz microstepping is done then new coordinates
								calculated, there, for the equat microstepping, Current equat is updated
								and the scope 'tracks' to the newly updated coordinates via
								MoveToCurrentRaDec() which is called from  Move_Update_Handpad_Subr() */
								Delta.A = StartAltaz.A + MovePerTick.A * TickCount - Current.Alt;
								Delta.Z = StartAltaz.Z + MovePerTick.Z * TickCount - Current.Az;
								SetDirDistanceStepsThenMove();
								/* here we are microstepping, so check for LX200 port requests like
								returning the new equatorial coordinates, and, go through the task
								controller, doing such things as displaying the new coordinates */
								CheckLX200Events();
								SequentialTaskController();
							}
						}
					}
					else
					{
						/* if offset coordinates */
						if( CurrentScroll->ScrollType == offset_altaz)
						{
							Delta.A = CurrentScroll->P->Alt;
							Delta.Z = CurrentScroll->P->Az;
						}
						else
						{
							Delta.A = CurrentScroll->P->Alt - Current.Alt;
							Delta.Z = CurrentScroll->P->Az - Current.Az;
						}
						SetDirDistanceStepsThenMove();
					}
					PauseUntilNewSidTime();
					HPEventGetEquat();
					break;
				case offset_equat:
				case absolute_equat:
					SetInputToScroll( CurrentScroll);
					if( CurrentScroll->Sec > 0)
					{
						/* spread move over CurrentScroll->Sec time, calc # of ticks to use */
						TicksForMove = CurrentScroll->Sec * ClockTicksSec;
						/* if offset equatorial coordinates */
						if( CurrentScroll->ScrollType == offset_equat)
						{
							MovePerTick.A = CurrentScroll->P->Ra / (double) TicksForMove;
							MovePerTick.Z = CurrentScroll->P->Dec / (double) TicksForMove;
						}
						/* else absolute equatorial coordinates */
						else
						{
							MovePerTick.A = (CurrentScroll->P->Ra - Current.Ra) / (double) TicksForMove;
							MovePerTick.Z = (CurrentScroll->P->Dec - Current.Dec) / (double) TicksForMove;
						}
						TickCount = 0;
						/* if microstepping not possible, then slew: otherwise a total of TicksForMove
						slews will occur */
						if( fabs( MovePerTick.A) > MaxMsDistanceRadTick.A ||
						fabs( MovePerTick.Z) > MaxMsDistanceRadTick.Z)
						{
							Current.Ra = In.Ra;
							Current.Dec = In.Dec;
						}
						else
							/* keep control here, microstepping to 'scroll to' position */
							while( TickCount < TicksForMove && ScrollFlag)
							{
								TickCount++;

								Current.Ra += MovePerTick.A;
								Current.Dec += MovePerTick.Z;
								Move_Update_Handpad_Subr();
							}
					}
					/* else time for move in seconds is 0, so slew */
					else
					{
						Current.Ra = In.Ra;
						Current.Dec = In.Dec;
					}
					break;
				case init1:
				case init2:
				case init3:
					if( CurrentScroll->ScrollType == init1 ||
					(CurrentScroll->ScrollType == init2 && One.Init) ||
					(CurrentScroll->ScrollType == init3 && Two.Init))
					{
						SetInputToScroll( CurrentScroll);
						Current.Ra = In.Ra;
						Current.Dec = In.Dec;
						strcpy( WhyInit, WHY_INIT_SCROLL);
						switch( CurrentScroll->ScrollType)
						{
							case init1:
								KBEventInitMatrix( 1);
								break;
							case init2:
								KBEventInitMatrix( 2);
								break;
							case init3:
								KBEventInitMatrix( 3);
						}
					}
					break;
				case init1usinginput:
				case init2usinginput:
				case init3usinginput:
					if( CurrentScroll->ScrollType == init1usinginput ||
					(CurrentScroll->ScrollType == init2usinginput && One.Init) ||
					(CurrentScroll->ScrollType == init3usinginput && Two.Init))
					{
						Current.Ra = In.Ra;
						Current.Dec = In.Dec;
						strcpy( WhyInit, WHY_INIT_SCROLL);
						switch (CurrentScroll->ScrollType)
						{
						case init1usinginput :
							KBEventInitMatrix( 1);
							break;
						case init2usinginput :
							KBEventInitMatrix( 2);
							break;
						case init3usinginput :
							KBEventInitMatrix( 3);
						}
					}
					break;
				case file1:
					strcpy( Filename, CurrentScroll->Note);
					ProcessMenuDataFileClosest( FilenameSet, ExactInputFieldsMatchOK);
					Current.Ra = In.Ra;
					Current.Dec = In.Dec;
					break;
				case file2:
					strcpy( Filename, CurrentScroll->Note);
					ProcessMenuDataFileClosest( FilenameSet, AvoidMatchInputFields);
					Current.Ra = In.Ra;
					Current.Dec = In.Dec;
					break;
				case file3:
					strcpy( Filename, CurrentScroll->Note);
					ProcessMenuDataFileClosest( FilenameSet, DoNotMatchInputFile);
					Current.Ra = In.Ra;
					Current.Dec = In.Dec;
					break;
				case trackon:
					TrackFlag = On;
					WriteTrackStatus();
					break;
				case trackoff:
					TrackOff();
					WriteTrackStatus();
					break;
				case auto_scroll_alert_off:
					AutoScrollAlertFlag = No;
					break;
				case analyze:
					if( Two.Init)
						WriteAnalysisFile();
					break;
				case alt_offset:
					if( Two.Init)
					{
						CalcAltOffsetIteratively( One, Two);
						if( AltOffset != MAXDOUBLE)
						{
							AddAltOffset();
							HPEventGetEquat();
						}
					}
					break;
				case auto_scroll:
					AutoScrollFlag = True;
					break;
				case auto_scroll_off:
					AutoScrollFlag = False;
					break;
				case move_zero_pec:
					ProcessMenuMoveZeroPEC();
					break;
				case save1:
					ProcessSav1();
					break;
				case restore1:
					ProcessRes1();
					break;
				case save2:
					ProcessSav2();
					break;
				case restore2:
					ProcessRes2();
					break;
            case move_equat:
               ProcessMenuMoveEquat();
               break;   
				case msarcsecsec:
					MsArcsecSec = CurrentScroll->i1;
					if( MsArcsecSec > MaxMsSpeed)
						MsArcsecSec = MaxMsSpeed;
					InitMsTickVars( MsArcsecSec);
					WriteMsArcsecSec();
					break;
				case handpad_mode:
					HandpadFlag = CurrentScroll->i1;
					WriteHandpadStatus();
					break;
				case halfstep:
					Dir.A = CW;
					if( CurrentScroll->L->A < 0)
					{
						CurrentScroll->L->A = -CurrentScroll->L->A;
						Dir.A = CCW;
					}
					Dir.Z = CW;
					if( CurrentScroll->L->Z < 0)
					{
						CurrentScroll->L->Z = -CurrentScroll->L->Z;
						Dir.Z = CCW;
					}
					Steps.A = CurrentScroll->L->A;
					Steps.Z = CurrentScroll->L->Z;
					KBEventMoveHs();
					PauseUntilNewSidTime();
					HPEventGetEquat();
					break;
				case new_equat:
					In.Ra = CurrentScroll->P->Ra;
					In.Dec = CurrentScroll->P->Dec;
					ProcessMenuResetEquat();
					break;
				case new_altaz:
					In.Alt = CurrentScroll->P->Alt;
					In.Az = CurrentScroll->P->Az;
					ProcessMenuResetAltaz();
					break;
				case set_equat:
					ProcessMenuInputEquat();
					break;
				case reset_equat:
					ProcessMenuResetEquat();
					break;
				case set_altaz:
					ProcessMenuInputAltaz();
					break;
				case reset_altaz:
					ProcessMenuResetAltaz();
					break;
				case reset_home:
					ProcessMenuResetHome();
					break;
				case data_file:
					strcpy( Filename, CurrentScroll->Note);
					DataFileNameSet = Yes;
					MenuCoordFile();
					DataFileNameSet = No;
					break;
				case move_file:
					strcpy( Filename, CurrentScroll->Note);
					DataFileNameSet = Yes;
					MenuCoordFile();
					DataFileNameSet = No;
					Current.Ra = In.Ra;
					Current.Dec = In.Dec;
					break;
				case prompt:
					break;
			}

			if( AutoScrollFlag)
			{
				if( KeyStroke)
				{
					AutoScrollFlag = No;
					ScrollFlag = No;
					ClearScrollCountsDisplayArea();
				}
			}
			else
				/* either: after 'scroll to' finished, mark time by tracking, or, slew to position */
				while( CurrentScrollFlag && ScrollFlag)
				{
					Move_Update_Handpad_Subr();
					/* if LeftKey, start next scroll */
					if( Buttons & LeftKey)
						CurrentScrollFlag = No;
				}
		}
	}
	TextAttr = CurrentText;
	VidMemXY = DisplayXY[DisplayObject];
	sprintf( StrBuf, NameBlanks);
	WriteStrBufToScreen_f_ptr();
	ScrollUnderway = No;
	WriteHandpadStatus();
}

void SetInputToScroll( struct SCROLL* Scroll)
{
	if( Scroll->ScrollType == offset_equat)
	{
		In.Ra = Current.Ra + Scroll->P->Ra;
		In.Dec = Current.Dec + Scroll->P->Dec;
	}
	else
	{
		In.Ra = Scroll->P->Ra;
		In.Dec = Scroll->P->Dec;
	}
	In.RaHMSH = Scroll->P->RaHMSH;
	In.DecDMS = Scroll->P->DecDMS;
	/* don't DisplayIn() as it will wipe out just displayed scroll counts and scroll action */
}

void ToggleGrandTourRightKey( void)
{
	if( HandpadFlag == GrandTour)
		if( GrandTourResetEquatFlag == No)
			GrandTourResetEquatFlag = Yes;
		else
			GrandTourResetEquatFlag = No;
}

void RestartScroll( void)
{
	int LinkCount;
	int CurrentScrollCountHold = CurrentScrollCount;

	if( CurrentScroll == NULL)
	{
		WriteWindow( MsgFrame);
		gotoxy( MsgFrame.Left+12, MsgFrame.Top+3);
		printf( "Scroll File not loaded");
		gotoxy( MsgFrame.Left+12, MsgFrame.Top+5);
		ContMsgRoutine();
		RemoveWindow( MsgFrame);
	}
	else
	{
		WriteWindow( MsgFrame);
		gotoxy( MsgFrame.Left+12, MsgFrame.Top+3);
		printf( "Restart Scroll at Record [%3d]", CurrentScrollCount);
		gotoxy( MsgFrame.Left+42, MsgFrame.Top+3);
		GetInt( &CurrentScrollCount);

		if( CurrentScrollCount > TotalScrollCount)
			CurrentScrollCount = CurrentScrollCountHold;

		/* ProcessScroll() starts on next record */
		CurrentScrollCount--;
		RemoveWindow( MsgFrame);

		/* Now go through linked list to get CurrentScroll */
		if( CurrentScrollCount < 1)
			CurrentScroll = NULL;
		else
		{
			CurrentScroll = FirstScroll;
			LinkCount = 1;

			while( CurrentScrollCount != LinkCount)
			{
				CurrentScroll = CurrentScroll->NextScroll;
				LinkCount++;
			}
		}

		RestartScrollFlag = Yes;
		ScrollFlag = Yes;

		/* In case a keyboard command wrote over the scrollfile name */
		DisplayIn( ScrollFilename, NameBlanks);

		ProcessScroll();
	}
}

void GetGrandTourRec( void)
{
	struct Object O;
	int RecNum = 1;

	/* scan through file, ending with read of desired record */
	while( !feof( Input))
	{
		ReadRa( Input, &O.RaHMSH);
		if( !feof( Input))
			ReadDec( Input, &O.DecDMS);
		if( !feof( Input))
			if( FReadStringToCharCountOrNewLine( Input, O.Name, sizeof( O.Name)))
				;
			else
				FReadToNewLine( Input);

		if( RecNum == GrandTourRecNum)
			break;
		else
			RecNum++;
	}
	if( !feof( Input) && RecNum == GrandTourRecNum)
	{
		/* set Current to grand tour coordinates, then set input coordinates to current coordinates */
		CalcRadFromHMSH( &Current.Ra, O.RaHMSH);
		CalcRadFromDMS( &Current.Dec, O.DecDMS);
		In = Current;
		DisplayIn( GrandTourFilename, O.Name);
	}
	else
	{
		GrandTourRecNum--;
		PressKeyToContMsg( "grand tour: at end of data file");
	}
}

void GetGrandTourRecClosestCurrentEquat( void)
{
	int RecNum, BestRecNum;
	double Angsep, BestAngsep;
	char FullyQualifiedFilename[NameSize];

	strcpy( FullyQualifiedFilename, DataDir);
	strcat( FullyQualifiedFilename, "\\");
	strcat( FullyQualifiedFilename, GrandTourFilename);

	Input = fopen( FullyQualifiedFilename, "r");
	if( Input == NULL)
		PressKeyToContMsg( "data file not loaded");
	else
	{
		RecNum = 1;
		BestRecNum = 1;
		BestAngsep = MAXDOUBLE;

		/* scan through file, ending with read of desired record */
		while( !feof( Input))
		{
			ReadRa( Input, &In.RaHMSH);
			if( !feof( Input))
				ReadDec( Input, &In.DecDMS);
			if( !feof( Input))
				if( FReadStringToCharCountOrNewLine( Input, Name, sizeof( Name)))
					;
				else
					FReadToNewLine( Input);
			if( !feof( Input))
			{
				CalcRadFromHMSH( &In.Ra, In.RaHMSH);
				CalcRadFromDMS( &In.Dec, In.DecDMS);
				Angsep = CalcEquatAngularSep( &Current, &In);
				if( Angsep < BestAngsep)
				{
					BestAngsep = Angsep;
					BestRecNum = RecNum;
				}
			}
			RecNum++;
		}
		fclose( Input);

		GrandTourRecNum = BestRecNum;

		Input = fopen( FullyQualifiedFilename, "r");
		if( Input == NULL)
			BadExit( strcat( "Could not open ", FullyQualifiedFilename));
		GetGrandTourRec();
		fclose( Input);

		GrandTourRecNum--;
	}
}

void ProcessGrandTour( void)
{
	char FullyQualifiedFilename[NameSize];

	strcpy( FullyQualifiedFilename, DataDir);
	strcat( FullyQualifiedFilename, "\\");
	strcat( FullyQualifiedFilename, GrandTourFilename);

	/* time to reset handpad key */
	delay( 1000);

	Input = fopen( FullyQualifiedFilename, "r");
	if( Input == NULL)
		BadExit( strcat( "Could not open ", FullyQualifiedFilename));

	if( Buttons & LeftKey || KeyboardLeftButton || LX200LeftButton)
	{
		KeyboardLeftButton = LX200LeftButton = False;
		GrandTourRecNum++;
		GetGrandTourRec();
	}
	/* Buttons & RightKey */
	else
	{
		KeyboardRightButton = LX200RightButton = False;
		if( GrandTourResetEquatFlag == No)
		{
			if( GrandTourRecNum == 0)
			{
				PressKeyToContMsg( "Grand Tour: first object not moved to yet");
				DisplayIn( GrandTourFilename, NameBlanks);
			}
			else
			{
				GrandTourRecNum--;
				if( GrandTourRecNum < 1)
				{
					PressKeyToContMsg( "Grand Tour: at first object");
					GrandTourRecNum = 1;
				}
				else
					GetGrandTourRec();
			}
		}
		else
			ProcessMenuResetEquat();
	}
	fclose( Input);
	/* set to yes in ProcessHPEvents() */
	GrandTourFlag = No;
}

void Move_Update_Handpad_Subr( void)
{
	if( Buttons & UpKey || Buttons & DownKey || Buttons & CCWKey || Buttons & CWKey)
		ProcessHPEventsMoveMotors();
	else
	{
		CheckMiscEvents();
		SequentialTaskController();
	}
	ReadHandpad_f_ptr();

	/* if RightKey or KeyStoke, finished with all Scroll */
	if( Buttons & RightKey)
	{
		ScrollFlag = No;
		ClearScrollCountsDisplayArea();
	}
	else
		/* certain keystrokes are defined to emulate the handpad mode switches */
		if( KeyStroke)
		{
			Response = getch();
			if( Response == ExtendedKeyboardStroke)
				Response = getch();
			if( Response == 'l' || Response == 'L' || Response == '<')
				Buttons = LeftKey;
			else
				if( Response == '>')
					Buttons = RightKey;
				else
				{
					ScrollFlag = No;
					ClearScrollCountsDisplayArea();
				}
		}
}

/* Polar Alignment is split into five parts to allow operator intervention
between stages.  The process is started via the Init Menu or via hotkey '*'.
Subsequent stages are entered via hotkey '*' or via the handpad left key.
In Stage 0, two stars are selected from the databases and the coordinates
of the first star entered as input.  After slewing to star#1, the operator
then centers this star in the eyepiece with the handpad. In Stage 1, a ResetEquatorial
is executed, the coordinates of the second star entered as input and a slew
to those coordinates executed. The operator then centers this star in the
eyepiece using the handpad. In Stage 2, the apparent coordinates of the second
star are read and the offset of the polar axis calculated. Then a set of
coordinates are calculated for the second star which are offset so that when
the polar axis is moved to correct alignment, the star will be centered in
the eyepiece. Finally, a slew to these coordinates takes place. The operator
then adjusts the polar axis. In Stage 3, a Reset Equatorial is performed
using Star 2's correct coordinates.  Finally, a slew back to the first star
takes place. In Stage 4, the operator is given the choice of repeating the
process or of quitting.

If the handpad is to be used, the Select HP Option "Polar Alignment" should be
set prior to the start of the polar alignment process */

void ProcessHPPolarAlign( void)
{
	 /* This function entered via the Sequential Task Controller which constantly
	 monitors the state of the handpad and keyboard */

	 delay( 200);
	 if( Buttons & LeftKey || KeyboardLeftButton || LX200LeftButton)
	 {
		KeyboardLeftButton = LX200LeftButton = False;

		switch( PolarAlignmentStage)
		{
		case 1:
			ProcessPolarAlign1();
			break;
		case 2:
			ProcessPolarAlign2();
			break;
		case 3:
			ProcessPolarAlign3();
			break;
		case 4:
			ProcessPolarAlign4();
			break;
		}
		HPPolarAlignFlag = No;
	 }
    else
		HPPolarAlignFlag = No;
}

void ProcessPolarAlign0( void)
{
   time_t t;

	/* prepare for handpad control */
	if( HandpadFlag == HandpadPolarAlign)
		HPPolarAlignLoaded = Yes;
	TrackFlag = On;
	WriteTrackStatus();
	/* get two alignment stars */
	SelectObject = 2;
	MenuCoordFile();
	/* reset MenuCoordFile() back to retrieving a single object at a time */
	SelectObject = 1;
	/* if select of stars not successful */
	if( !ValidResponseFlag)
		PolarAlignmentStage = 0;
	else
	{
      Output = fopen( PolarAlignFile, "a");
	   if( Output == NULL)
		  BadExit( strcat( "Could not append to ", PolarAlignFile));

      time( &t);
	   /* ctime() ends with \n\0 */
      fprintf( Output, "\nStarting Polar Alignment Procedure on %s", ctime( &t));
	   fprintf( Output, "\nStar 1 = %s    Star 2 = %s", Star.Name[0],Star.Name[1]);
      fprintf( Output, "\nStar 1 Ra/Dec = %2.5f %2.5f Star 2 Ra/Dec = %2.5f %2.5f\n",
                          Star.Ra[0],Star.Dec[0],Star.Ra[1],Star.Dec[1]);
      fclose( Output);

      /* ready for next hotkey step */
		PolarAlignmentStage = 1;
		/* setup for first initialization */
		In.Ra = Star.Ra[0];
		In.Dec = Star.Dec[0];
		/* go to Star 1 position */
		ProcessMenuMoveEquat();
		DisplayIn("Polar Align Star 1", Star.Name[0]);
	}
}

void ProcessPolarAlign1( void)
{
   /* Record sidereal time  */
   Star.SidT[0] = SidT;

	/* make Star 1 current (coordinates already in 'In' input position) */
	ProcessMenuResetEquat();

	/* go to Star 2 position */
	In.Ra = Star.Ra[1];
	In.Dec = Star.Dec[1];
	ProcessMenuMoveEquat();
	DisplayIn("Polar Align Star 2", Star.Name[1]);

	PolarAlignmentStage++;
}

void ProcessPolarAlign2( void)
{
   /* After centering Star2, have info to calculate Polar Axis Error  */
	ProcessPolarAlignCalc( 1);

	PolarAlignmentStage++;
}

void ProcessPolarAlign3( void)
{
	/* adjust polar axis, reset equatorial and move	back to Star 1 */
   In.Ra = Star.Ra[1];
   In.Dec = Star.Dec[1];
   ProcessMenuResetEquat();
	In.Ra = Star.Ra[0];
	In.Dec = Star.Dec[0];
	ProcessMenuMoveEquat();
	DisplayIn("Polar Align Star 1", Star.Name[0]);

	PolarAlignmentStage++;
}

void ProcessPolarAlign4( void)
{
   /* Now need to center Star1, handpad left and calculate
      residual polar axis error  */
   ProcessPolarAlignCalc( 0);

	/* ask for repeat or quit */
	WriteWindow( MsgFrame);
   gotoxy( MsgFrame.Left+2, MsgFrame.Top+2);
   printf( "Elevation = %4.2f arcmin   Azimuth = %4.2f arcmin", ElevationError*RadToArcmin,AzimuthError*RadToArcmin);
	gotoxy( MsgFrame.Left+2, MsgFrame.Top+4);
	printf( "Do you want to repeat the Polar Alignment Procedure?");
	gotoxy( MsgFrame.Left+7, MsgFrame.Top+6);
	printf( "Press Y to repeat, any other key to quit");
	sound( 500);
	delay( 100);
	sound( 1000);
	delay( 200);
	sound( 500);
	delay( 100);
	nosound();
	GetResponseWithLX200Check();
	RemoveWindow( MsgFrame);

	if (Response == 'y' || Response == 'Y')
		/* get ready for next pass */
		PolarAlignmentStage = 1;
	else
	{
		PolarAlignmentStage = 0;
		/* inactivate handpad */
		HPPolarAlignLoaded = No;
		HandpadFlag = HandpadOff;
		WriteHandpadStatus();
      Output = fopen( PolarAlignFile, "a");
      if( Output == NULL)
        BadExit( strcat( "Could not append to ", PolarAlignFile));

      fprintf( Output, "\nResidual Polar Alignment Error\n");
      fprintf( Output, "Elevation = %4.2f Arcmin   Azimuth = %4.2f Arcmin\n", ElevationError*RadToArcmin,AzimuthError*RadToArcmin);
      fclose( Output);     
	}
}

void ProcessPolarAlignCalc( int Mode)
{
   double H1,H2;
   double Sin1, Sin2;
	double Cos1, Cos2;
	double Tan1, Tan2;
	double DeltaDec;
	double DeltaRa;
   double D;
   int A,B;

   if( Mode)
   {
     A = 0;
     B = 1;
   }
   else
   {
     A = 1;
     B = 0;
   }

   /* Get sidereal time  */
   Star.SidT[B] = SidT;

	/* Get Star apparent position */
	Star.Ra[2] = Current.Ra;
	Star.Dec[2] = Current.Dec;

	/* Now have information to calculate polar axis offset.

      Hour Angle(H) = Sidereal Time at location - Ra  */

   H1 = Star.SidT[A] - Star.Ra[A];
   H2 = Star.SidT[B] - Star.Ra[B];
	Sin1 = sin( -H1);
	Sin2 = sin( -H2);

	Cos1 = cos( -H1);
	Cos2 = cos( -H2);

	Tan1 = tan( Star.Dec[A]);
	Tan2 = tan( Star.Dec[B]);

	/* difference between Star 2 and observed */
	DeltaDec = Star.Dec[2] - Star.Dec[B];
	DeltaRa = Star.Ra[2] - Star.Ra[B];

   /* Check for movement across the 0/24 hr line  */
   if( DeltaRa < -M_PI) DeltaRa += OneRev;
   else if( DeltaRa > M_PI) DeltaRa -= OneRev;

   D = ConfigCosLat*((Tan1 + Tan2)*cos( H2 + H1) - Tan1*cos(2*H1) - Tan2*cos(2*H2));

	/* polar axis error */
	ElevationError = (ConfigCosLat*DeltaRa*(Sin2-Sin1) - ConfigCosLat*DeltaDec*(Tan2*Cos2 - Tan1*Cos1))/D;
	AzimuthError = (-DeltaRa*(Cos2-Cos1) + DeltaDec*(Tan2*Sin2 - Tan1*Sin1))/D;

   if( Mode)
   {
     Output = fopen( PolarAlignFile, "a");
     if( Output == NULL)
       BadExit( strcat( "Could not append to ", PolarAlignFile));

     fprintf( Output, "Apparent Star 2 Position = %2.5f %2.5f\n", Star.Ra[2],Star.Dec[2]);
     fprintf( Output, "Elevation Error = %4.2f Arcmin   Azimuth Error = %4.2f Arcmin\n", ElevationError*RadToArcmin,AzimuthError*RadToArcmin);
     fclose( Output);

     /* Now reset current to Star2  */
     In.Ra = Star.Ra[1];
     In.Dec = Star.Dec[1];
     ProcessMenuResetEquat();

	  In.Ra += ElevationError*Tan2*Sin2 + AzimuthError*( -fabs( ConfigSinLat) + ConfigCosLat*Tan2*Cos2);
	  In.Dec -= ElevationError*Cos2 + AzimuthError*ConfigCosLat*Sin2;

     /* Check again for movement across the 0/24 hr line  */
     if( In.Ra < 0.0) In.Ra += OneRev;
     else if( In.Ra > OneRev) In.Ra -= OneRev;

	  /* go to Star2 offset position */
	  ProcessMenuMoveEquat();
	  DisplayIn("Adjust Mount", Star.Name[1]);
   }
}


