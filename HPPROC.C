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

/* handpad area... */

void InitHPEvent( void)
{
	if( AutoAltPECPin != 15 && AutoAltPECPin != 16 && AutoAltPECPin != 17 && AutoAltPECPin != 101213l)
		BadExit( "AutoAltPECPin must be 15, 16, 17, or 101213");
	if( AutoAzPECPin != 15 && AutoAzPECPin != 16 && AutoAzPECPin != 17 && AutoAzPECPin != 101213l)
		BadExit( "AutoAzPECPin must be 15, 16, 17, or 101213");

	InsideProcessHPEventsFocusMethod_Pulse_Dir = No;
	DisplayAltAutoSynchFlag = No;
	DisplayAzAutoSynchFlag = No;
	TrackOff();
	MoveToCurrentRaDecFlag = No;
	TrackByRateFlag = No;
	GuideFlag = No;
	GuideArrayFlag = No;
	GrandTourFlag = No;
	ScrollFlag = No;
	HPPolarAlignFlag = No;

	InitiatedType = NotInitiated;
	HandpadFlag = StartingHandpadMode;
	HandpadAuxButtonPressed = No;
	HandpadFocusButtonPressed = No;
	HandpadAuxControlOnFlag = No;
	HandpadFRMotorControlOnFlag = No;
	HandpadFocusMotorControlOnFlag = No;
	HandpadButtonTimer = 0;
	RecordEquatTimer = 0;
	ConsecutiveSlews = 0;

	Drift.Alt = DriftAltArcsecPerMin * ArcsecToRad;
	Drift.Az = DriftAzArcsecPerMin * ArcsecToRad;
	Drift.Ra = DriftRaDegPerHr * DegToRad;
	Drift.Dec = DriftDecDegPerHr * DegToRad;
	CalcCommonDriftVars();

	CalcBacklashVars();
	SetBacklashDirA( CW);
	SetBacklashDirZ( CW);

	InitAltAzEC();
	InitAltAltEC();
	InitAzAzEC();

	PMC.A = PMC.Z = 0;
	if( PointingModelFlag == Yes)
		if( LoadPMC())
			if( LinkPMCCount)
				CalcPMCErrors();
			else
			{
				PointingModelFlag = No;
				if( DisplayOpeningMsgs)
					printf( "\nNothing in pmc.dat file, turning PMC off");
			}
		else
		{
			PointingModelFlag = No;
			if( DisplayOpeningMsgs)
				printf( "\nDid not find pmc.dat file, turning PMC off");
		}

	/* set microstepping index:
	(AccumMs.A%MsInWindings)/Ms is winding #, so multiply this by MaxPWM*Ms to get index */
	MsIx.A = (int)(MaxPWM*Ms*((AccumMs.A%MsInWindings)/Ms));
	if( MsIx.A < 0)
		MsIx.A += MaxMsIx + 1;
	MsIx.Z = (int)(MaxPWM*Ms*((AccumMs.Z%MsInWindings)/Ms));
	if( MsIx.Z < 0)
		MsIx.Z += MaxMsIx + 1;

	SetCurrentAltazToAccumMs();
	HPEventGetEquat();

	if( FocusFastStepsSec < 1)
		FocusFastStepsSec = 1;
	if( FocusSlowStepsSec < 1)
		FocusSlowStepsSec = 1;
	if( FocusFastStepsSec > 500)
		FocusFastStepsSec = 500;
	if( FocusSlowStepsSec > 500)
		FocusSlowStepsSec = 500;

	InitFR();

	if( FocusMethod == FocusMethod_Pulse_1_14 || FocusMethod == FocusMethod_OnOff_16_17_Slow1_14)
	{
		FRMotorOn = No;
		FRMotorTrackOn = No;
		if( DisplayOpeningMsgs)
		{
			printf( "\nField rotation motor control turned off.");
			if( FocusMethod == FocusMethod_Pulse_1_14)
				printf( "\nFocuser using parallel port pins 1+14 for pulse/dir controller.");
			else
				if( FocusMethod == FocusMethod_OnOff_16_17_Slow1_14)
					printf( "\nFocuser using parallel port pins 16+17 and 1+14 for 2 speed on/off control.");
		}
	}
	else
	{
		FRMotorOn = Yes;
		if( FRStepSize == 0)
			FRMotorTrackOn = No;
		else
			FRMotorTrackOn = Yes;
		if( DisplayOpeningMsgs)
			if( FRMotorTrackOn)
				printf( "\nfield rotation motor tracking on");
			else
				printf( "\nfield rotation motor tracking off because FR step size = 0");
		if( DisplayOpeningMsgs)
			printf( "\nField rotation using parallel port pins 1+14 for pulse/dir controller.");
		if( FocusMethod == FocusMethod_Pulse_16_17)
		{
			if( DisplayOpeningMsgs)
				printf( "\nFocuser using parallel port pins 16+17 for pulse/dir controller.");
		}
		else
			if( FocusMethod == FocusMethod_OnOff_16_17)
			{
				if( DisplayOpeningMsgs)
					printf( "\nFocuser using parallel port pins 16+17 for on/off control.");
			}
	}
}

void CalcBacklashVars( void)
{
	BacklashFlag = No;
	NegBacklash.A = NegBacklash.Z = False;
	if( AltBacklashArcmin != 0)
	{
		BacklashFlag = Yes;
		if( AltBacklashArcmin < 0)
		{
			AltBacklashArcmin = -AltBacklashArcmin;
			NegBacklash.A = True;
		}
	}
	if( AzBacklashArcmin != 0)
	{
		BacklashFlag = Yes;
		if( AzBacklashArcmin < 0)
		{
			AzBacklashArcmin = -AzBacklashArcmin;
			NegBacklash.Z = True;
		}
	}
	/* set backlash */
	BacklashRad.A = AltBacklashArcmin * ArcminToRad;
	BacklashMs.A = (long) (0.5 + (BacklashRad.A/MsRad.A));
	BacklashRad.A = (double) BacklashMs.A * MsRad.A;
	/* to ensure that ActualBacklash is between 0 and Backlash */
	AddBacklashA( 0);
	BacklashRad.Z = AzBacklashArcmin * ArcminToRad;
	BacklashMs.Z = (long) (0.5 + (BacklashRad.Z/MsRad.Z));
	BacklashRad.Z = (double) BacklashMs.Z * MsRad.Z;
	AddBacklashZ( 0);
}

void InitFR( void)
{
	FRStepsToMove = 0;
	FRStepSize = FRStepSizeArcsec * ArcsecToRad;
	FRPulse = Off;
	if( Two.Init)
	{
		CalcFieldR();
		FRMotorAngle = FieldR;
	}
	else
		FRMotorAngle = 0;
	FRDiagPulseHighCW = FRDiagPulseHighCCW = FRDiagPulseLow = FRDiagReset = 0;

	MaxFRStepsPerTick = 1000. / (ClockTicksSec * FRStepSpeedMilliSec);
}

/* AccumMs is the telescope position derived from counting the revolutions of the motors while
Current. is the actual telescope's pointing position */
void SetCurrentAltazToAccumMs( void)
{
	struct Position Temp;

	/* set to AccumMs */
	Current.Alt = AccumMs.A*MsRad.A;
	Current.Az = AccumMs.Z*MsRad.Z;

	/* ActualBacklashRad is the amount of backlash that has been taken up in the CCW direction;
	if > 0, motors have been spun extra CCW to takeup backlash up with no change in scope's apparent
	pointing direction resulting in Cfg.ServoParms[].ActualPosition being too low, so must add this
	value to Cfg.ServoParms[].ActualPosition to arrive at True pointing position;
	ie, if actualbacklash 0 and motor at 20, then scope at 20;
		 if motor moves CCW 10 and if backlash 5, then motor moves from 20 to 10, but scope only moves
		 from 20 to 15, and actualbacklash goes to 5, since 5 of the move was taken up by the backlash */
	if( BacklashFlag)
	{
		Current.Alt += ActualBacklashRad.A;
		Current.Az += ActualBacklashRad.Z;
	}
	/* positive PEC, Drift, and Guide, indicate excess CW movement, that is, the scope with respect
	to the stars has drifted CW, therefore the Current. position that AccumMs would yield is too
	small and must be increased to match the altaz coordinates as derived from the stars;
	in the same manner, positive QSC means excess CW movement (halfsteps too large);
	positive AccumGuide means that the a CCW button had to be pressed to compensate for excess CW
	movement	(CCW button reduces AccumMs and would result in lower Current.	unless AccumGuide added
	in to	compensate); AccumDrift is based on a guiding session where a drift rate of AccumGuide over
	time is calculated */
	if( PECFlag)
	{
		GetPECIx();
		Current.Alt += PECToAdd.A * TenthsArcsecToRad;
		Current.Az += PECToAdd.Z * TenthsArcsecToRad;
	}
	if( UseQSC)
	{
		CalcQSC();
		Current.Alt += QSC.A;
		Current.Az += QSC.Z;
	} 
	if( DriftAltazFlag)
	{
		/* don't add in drift while slewing */
		if( SlewState == SlewDone)
		{
			AccumDrift.A += DriftTick.Alt;
			AccumDrift.Z += DriftTick.Az;
		}
		Current.Alt += AccumDrift.A;
		Current.Az += AccumDrift.Z;
	}
	if( GuideFlag)
	{
		Current.Alt += AccumGuide.A;
		Current.Az += AccumGuide.Z;
		if( GuideArrayFlag == WritingToGuideArray)
			WriteToGuideArray();
	}
	/* compensate for refraction after arriving at pointing position as represented by sky coordinates;
	refraction causes an object near the horizon to appear higher than it really is;
	therefore Cfg.Current altaz coord, must show a higher value, ie,
	if AccumMs coordinate is 0 deg, then Cfg.Current altaz/True pointing/ sky position should read -34.5 arcmin */
	if( RefractFlag == 1)
	{
		CalcRefractScopeToSky( Current.Alt);
		Current.Alt -= Refract;
	}
	if( RefractFlag == 2)
	{
		Temp = Current;
		// current altaz -> current equat -> site equat -> site altaz -> refraction values
		GetEquat();
		SitePos.Ra = Current.Ra;
		SitePos.Dec = Current.Dec;
		SitePos.SidT = Current.SidT;
		GetSiteAltaz();
		CalcSiteRefractScopeToSky();
		Current = Temp;
		Current.Az += RefractRa;
		Current.Alt -= RefractDec;
	}

	/* in ProcessMenuAnalyzePointing(), CalcPMC(), SetAltAzECValue(), SetAltAltECValue(), and
	SetAzAzValue(), positive corrective values mean scope ended up too far CW, therefore AccumMs is
	too much and Current. must be reduced;
	this is different than PEC, Drift, and Guide where positive values mean that the scope drifted
	too far CW, CCW button was pressed to compensate, and thus the AccumMs is too small and therefore
	must be increased in order	to	arrive at the True pointing position */
	if( UseAltAzECFlag)
	{
		SetAltAzECValue();
		Current.Alt -= AltAzECRad;
	}
	if( UseAltAltECFlag)
	{
		SetAltAltECValue();
		Current.Alt -= AltAltECRad;
	}
	if( UseAzAzECFlag)
	{
		SetAzAzECValue();
		Current.Az -= AzAzECRad;
	}
	if( PointingModelFlag)
	{
		CalcPMC( Current.Alt, Current.Az);
		Current.Alt -= PMC.A;
		Current.Az -= PMC.Z;
	}
	ValidAz( &Current);
}

void SetAccumMsToCurrentAltaz( void)
{
	struct AZDouble HoldAZ;
	struct AZInt Ix;
	struct Position Temp;

	/* use Hold to calculate Alt and Az for AccumAltMs and AccumAzMs:
	this leaves Current. values unchanged */
	HoldAZ.A = Current.Alt;
	HoldAZ.Z = Current.Az;

	if( PointingModelFlag)
	{
		CalcPMC( HoldAZ.A, HoldAZ.Z);
		HoldAZ.A += PMC.A;
		HoldAZ.Z += PMC.Z;
	}
	if( UseAzAzECFlag)
	{
		SetAzAzECValue();
		HoldAZ.Z += AzAzECRad;
	}
	if( UseAltAltECFlag)
	{
		SetAltAltECValue();
		HoldAZ.A += AltAltECRad;
	}
	if( UseAltAzECFlag)
	{
		SetAltAzECValue();
		HoldAZ.A += AltAzECRad;
	}
	if( RefractFlag == 1)
	{
		CalcRefractSkyToScope( Current.Alt);
		HoldAZ.A += Refract;
	}
	if( RefractFlag == 2)
	{
		Temp = Current;
		Current.Alt = HoldAZ.A;
		Current.Az = HoldAZ.Z;
		// current altaz -> current equat -> site equat -> site altaz -> refraction values
		GetEquat();
		SitePos.Ra = Current.Ra;
		SitePos.Dec = Current.Dec;
		SitePos.SidT = Current.SidT;
		GetSiteAltaz();
		CalcSiteRefractSkyToScope();
		Current = Temp;
		HoldAZ.Z -= RefractRa;
		HoldAZ.A += RefractDec;
	}
	if( GuideFlag)
	{
		HoldAZ.A -= AccumGuide.A;
		HoldAZ.Z -= AccumGuide.Z;
	}
	if( DriftAltazFlag)
	{
		HoldAZ.A -= AccumDrift.A;
		HoldAZ.Z -= AccumDrift.Z;
	}
	if( UseQSC)
	{
		CalcQSC();
		HoldAZ.A -= QSC.A;
		HoldAZ.Z -= QSC.Z;
	}
	if( PECFlag)
	{
		GetPECIx();
		HoldAZ.A -= PECToAdd.A * TenthsArcsecToRad;
		HoldAZ.Z -= PECToAdd.Z * TenthsArcsecToRad;

		/* preserve PEC array indexes for use later in function */
		Ix = PECIx;
	}
	if( BacklashFlag)
	{
		HoldAZ.A -= ActualBacklashRad.A;
		HoldAZ.Z -= ActualBacklashRad.Z;
	}

	AccumMs.A = (long) (0.5 + HoldAZ.A/MsRad.A);
	/* set to nearest full step */
	AccumMs.A -= AccumMs.A%Ms;

	AccumMs.Z = (long) (0.5 + HoldAZ.Z/MsRad.Z);
	/* set to nearest full step */
	AccumMs.Z -= AccumMs.Z%Ms;

	/* since motor shafts have not moved, because the AccumMs has changed, the PEC array offsets need
	to be reset to correctly reflect motor shaft	angles based on AccumMs values */
	if( PECFlag)
	{
		GetPECIx();

		PECIxOffset.A += (PECIx.A - Ix.A);
		while( PECIxOffset.A < 0)
			PECIxOffset.A += PECSize;
		while( PECIxOffset.A >= PECSize)
			PECIxOffset.A -= PECSize;

		PECIxOffset.Z += (PECIx.Z - Ix.Z);
		while( PECIxOffset.Z < 0)
			PECIxOffset.Z += PECSize;
		while( PECIxOffset.Z >= PECSize)
			PECIxOffset.Z -= PECSize;
	}
}

void CalcQSC( void)
{
	/* # of quartersteps in the sequence of windings */
	struct AZInt QsInWindings;
	/* microstep # within a quarterstep */
	struct AZInt MsInQs;
	struct AZInt NextQsInWindings;

	/* quarterstep # in the windings sequence */
	QsInWindings.A = (MsIx.A/MaxPWM)/(Ms/4);
	if( QsInWindings.A < 0)
		QsInWindings.A += MsInWindings/(Ms/4);
	/* microstep # inside the quarterstep */
	MsInQs.A = (MsIx.A/MaxPWM)%(Ms/4);
	if( MsInQs.A < 0)
		MsInQs.A += Ms/4;
	NextQsInWindings.A = QsInWindings.A + 1;
	if( NextQsInWindings.A >= MotorWindings*4)
		NextQsInWindings.A = 0;
	QSC.A = QSCvalues[QsInWindings.A].A +
	(QSCvalues[NextQsInWindings.A].A - QSCvalues[QsInWindings.A].A)
	* (double)MsInQs.A/(double)(Ms/4);
	/* HsRad.A*2 == fullstep size in radians */
	QSC.A *= HsRad.A*2;

	/* quarterstep # in the windings sequence */
	QsInWindings.Z = (MsIx.Z/MaxPWM)/(Ms/4);
	if( QsInWindings.Z < 0)
		QsInWindings.Z += MsInWindings/(Ms/4);
	/* microstep # inside the quarterstep */
	MsInQs.Z = (MsIx.Z/MaxPWM)%(Ms/4);
	if( MsInQs.Z < 0)
		MsInQs.Z += Ms/4;
	NextQsInWindings.Z = QsInWindings.Z + 1;
	if( NextQsInWindings.Z >= MotorWindings*4)
		NextQsInWindings.Z = 0;
	QSC.Z = QSCvalues[QsInWindings.Z].Z +
	(QSCvalues[NextQsInWindings.Z].Z - QSCvalues[QsInWindings.Z].Z)
	* (double)MsInQs.Z/(double)(Ms/4);
	/* HsRad.Z*2 == fullstep size in radians */
	QSC.Z *= HsRad.Z*2;
}

void TrackOff( void)
{
	TrackFlag = Off;
	/* power down motors if using pulse/dir control method */
	if( TrackFlag == No && MotorControlMethod == MotorControl_PulseDir)
		SteppersOff_f_ptr();
}

/* called by KBEventMoveHs() */
void HPEventMoveHs( void)
{
	if( Steps.A)
		SetBacklashDirA( Dir.A);
	if( Steps.Z)
		SetBacklashDirZ( Dir.Z);

	MoveHs_f_ptr();
	SetCurrentAltazToAccumMs();
}

void HPEventMoveMs( void)
{
	DisplayButtonsStatus();
	MoveMs_f_ptr();
	SetCurrentAltazToAccumMs();
}

/* moves in (altaz) Ms until handpad buttons change */
void AltazMoveMs( void)
{
	struct AZLongV HoldSteps = Steps;

	if( Steps.A)
		SetBacklashDirA( Dir.A);
	if( Steps.Z)
		SetBacklashDirZ( Dir.Z);

	InitHandpad = Handpad;
	HandpadOKFlag = Yes;
	while( HandpadOKFlag)
	{
		HPEventMoveMs();
		HPEventGetEquat();
		CheckLX200Events();
		SequentialTaskController();
		/* add undone steps to steps to do for next clock tick */
		Steps.A += HoldSteps.A;
		Steps.Z += HoldSteps.Z;
		SetHandpadOKFlag();
	}
	AlignMs_f_ptr();
	SetCurrentAltazToAccumMs();
	HPEventGetEquat();
}

void SetFRControlLines( void)
{
	FRPulseBit = PPortPin1;
	FRDir = PPortPin14;
}

void ReverseFRDir( void)
{
	if( FRDir == Off)
		FRDir = PPortPin14;
	else
		FRDir = Off;
}

// set focus and FR unused pport lines
void GetUnusedPPortLines( void)
{
	BiDirInNibbleValue = BiDirInNibble();
	UnusedFRPPortLines = BiDirInNibbleValue & PPortPins16_17;

	/* PPortPins1_14 == 3, and PPortPins16_17 == 12;
		enumeration:	FocusMethod_OnOff_16_17,
							FocusMethod_OnOff_16_17_Slow1_14,
							FocusMethod_Pulse_1_14,
							FocusMethod_Pulse_16_17	*/
	switch( FocusMethod)
	{
		case FocusMethod_Pulse_1_14:
			UnusedFocusPPortLines = BiDirInNibbleValue & PPortPins16_17;
			break;
		case FocusMethod_Pulse_16_17:
		case FocusMethod_OnOff_16_17:
			UnusedFocusPPortLines = BiDirInNibbleValue & PPortPins1_14;
			break;
		case FocusMethod_OnOff_16_17_Slow1_14:
		default:
			UnusedFocusPPortLines = 0;
	}

	if (MotorWindings == 5)
	{
		UnusedFRPPortLines &= PPortPins1_14;
		UnusedFocusPPortLines &= PPortPins1_14;
	}
}

/* called from start of MoveMs(): sets up the field rotation motor for tracking */
void SetFRStepsDirMoveMs( void)
{
	static int HoldFRDir;
	double MaxFRStepsPerPWM;

	FRAngleToMove = FieldR - FRMotorAngle;

	/* bring it within -M_PI to +M_PI */
	while( FRAngleToMove < -M_PI)
		FRAngleToMove += OneRev;
	while( FRAngleToMove > M_PI)
		FRAngleToMove -= OneRev;

	SetFRControlLines();

	if( SectoredFRDrive)
		/* angle of max # of FR steps per bios clock tick */
		ResetFRMotorAngleThreshold = (PWMRepsTick / 2) * FRStepSize;
	else
		ResetFRMotorAngleThreshold = OneRev;

	/* if move too big, reset current motor position: prevents damage	to	finite motion field rotation
	systems */
	if( FRAngleToMove > ResetFRMotorAngleThreshold || FRAngleToMove < -ResetFRMotorAngleThreshold)
	{
		FRAngleToMove = 0;
		FRMotorAngle = FieldR;
		FRDiagReset++;
	}
	/* determine direction */
	if( FRAngleToMove < 0)
	{
		FRAngleToMove = -FRAngleToMove;
		ReverseFRDir();
	}
	/* reverse motor direction if necessary */
	if( ReverseFRMotor)
		ReverseFRDir();
	FRStepsToMove = FRAngleToMove / FRStepSize;

	/* only allow FR stepping and setting of direction if at least one step to move, otherwise,
	revert to last direction */
	if( FRStepsToMove>=1.)
		HoldFRDir = FRDir;
	else
	{
		FRStepsToMove = 0;
		FRDir = HoldFRDir;
	}

	/* calc FR steps per each PWM */
	FRStepIncrPerPWM = FRStepsToMove / PWMRepsTick;

	/* don't exceed FRStepSpeedMilliSec if slewing FR motor */
	MaxFRStepsPerPWM = MaxFRStepsPerTick / PWMRepsTick;
	if( FRStepIncrPerPWM > MaxFRStepsPerPWM)
		FRStepIncrPerPWM = MaxFRStepsPerPWM;
	/* max number of FR pulses per clock tick is PWMRepsTick/2 because of the on/off bit toggling */
	if( FRStepIncrPerPWM > .5)
		FRStepIncrPerPWM = .5;
	FRStepsDone = 0;
}

/* called from SetPulseFRPulseFocusInMoveMs() which is called from start of MoveMs(): sets up the
field rotation motor for movement via handpaddle */
void SetFRStepsDirHandpad( void)
{
	SetFRControlLines();
	if( Buttons & CCWKey)
		ReverseFRDir();
	if( ReverseFRMotor)
		ReverseFRDir();

	FRStepsToMove = 1000. / (FRStepSpeedMilliSec * ClockTicksSec);

	FRStepIncrPerPWM = FRStepsToMove / PWMRepsTick;
	/* max number of FR pulses per clock tick is PWMRepsTick/2 because of the on/off bit toggling */
	if( FRStepIncrPerPWM > .5)
		FRStepIncrPerPWM = .5;
	FRStepsDone = 0;
}

void SetFocusControlLines( void)
{
	if( FocusMethod == FocusMethod_Pulse_1_14)
	{
		FocusPulseBit = PPortPin1;
		FocusDir = PPortPin14;
	}
	else
		if( FocusMethod == FocusMethod_Pulse_16_17)
		{
			FocusPulseBit = PPortPin16;
			FocusDir = PPortPin17;
		}
		else
		{
			FocusPulseBit = Off;
			FocusDir = Off;
		}
}

void ReverseFocusDir( void)
{
	if( FocusDir == Off)
	{
		if( FocusMethod == FocusMethod_Pulse_1_14)
			FocusDir = PPortPin14;
		else
			if( FocusMethod == FocusMethod_Pulse_16_17)
				FocusDir = PPortPin17;
			else
				FocusDir = Off;
	}
	else
		FocusDir = Off;
}

/* called from SetPulseFRPulseFocusInMoveMs() which is called from start of MoveMs(): sets up the
focus motor for movement */
void SetFocusStepsDirMoveMs( void)
{
	SetFocusControlLines();
	if( InitiatedType == HandpadInitiated && Buttons & CCWKey ||
	InitiatedType == LX200Initiated && LX200_Focus_Cmd == FocusIn ||
	InitiatedType == MouseInitiated && (MouseMoveResult == MouseMovedLeft || MouseMoveResult == MouseMovedDown))
		ReverseFocusDir();
	if( ReverseFocusMotor)
		ReverseFocusDir();
	/* set speed */
	if( MsSpeed ||
	InitiatedType == MouseInitiated && (MouseMoveResult == MouseMovedUp || MouseMoveResult == MouseMovedDown) ||
	InitiatedType == LX200Initiated && LX200_Focus_Speed_Cmd == FocusSetSlow)
	{
		FocusFastDisplayFlag = False;
		FocusStepsToMove = (double) FocusSlowStepsSec / ClockTicksSec;
	}
	else
	{
		FocusFastDisplayFlag = True;
		FocusStepsToMove = (double) FocusFastStepsSec / ClockTicksSec;
	}
	FocusStepIncrPerPWM = FocusStepsToMove / (double) PWMRepsTick;
	/* max number of Focus pulses per clock tick is PWMRepsTick/2 because of the on/off bit toggling */
	if( FocusStepIncrPerPWM > .5)
		FocusStepIncrPerPWM = .5;
	FocusStepsDone = 0;
}

/* called from start of MoveMs() */
void SetPulseFRPulseFocusInMoveMs( void)
{
	PulseFR = False;
	PulseFocus = False;
	if( FRMotorOn && FRStepSize > 0)
	{
		/* if handpad is in field rotation motor mode and motor enabled by mode switch and a direction
		button is pressed, then field rotation motor movement caused by handpad - not due to field
		rotation motor tracking */
		if( HandpadFRMotorControlOnFlag && (Buttons & CCWKey || Buttons & CWKey))
		{
			SetFRStepsDirHandpad();
			PulseFR = True;
		}
		else
			/* if field rotation motor tracking turned on via menu control, calculate the field
			rotation angle along with the handpad buttons guide angle and set the field rotation
			steps and direction to move */
			if( FRMotorTrackOn)
			{
				CalcFieldR();
				CalcGuideFRAngle();
				SetFRStepsDirMoveMs();
				PulseFR = True;
			}
	}
	if( (FocusMethod == FocusMethod_Pulse_1_14 || FocusMethod == FocusMethod_Pulse_16_17) &&
	((HandpadFocusMotorControlOnFlag && (Buttons & CCWKey || Buttons & CWKey)) ||
	InitiatedType == LX200Initiated && LX200_Focus_Cmd != FocusQuit ||
	InitiatedType == MouseInitiated && MouseMode == MouseModeFocus && MouseLeftButtonDown))
	{
		SetFocusStepsDirMoveMs();
		PulseFocus = True;
	}
	if( PulseFR || PulseFocus)
		GetUnusedPPortLines();
}

/* called from MoveMs() at end of each PWM */
void PulseFRFocusPerPWM( void)
{
	BiDirOutNibbleValue = 0;

	if( PulseFR)
	{
		BiDirOutNibbleValue += FRDir;
		if( FRPulse)
		{
			FRPulse = Off;
			BiDirOutNibbleValue += FRPulseBit;
			FRStepsDone++;
			FRDiagPulseLow++;
		}
		else
		{
			HoldFRSteps += FRStepIncrPerPWM;
			if( HoldFRSteps >= 1 && FRStepsToMove > FRStepsDone)
			{
				HoldFRSteps -= 1;
				if( (FRDir && !ReverseFRMotor) || (!FRDir && ReverseFRMotor))
				{
					FRDiagPulseHighCW++;
					FRMotorAngle += FRStepSize;
				}
				else
				{
					FRDiagPulseHighCCW++;
					FRMotorAngle -= FRStepSize;
				}
				FRPulse = On;
			}
		}
	}

	if( PulseFocus)
	{
		BiDirOutNibbleValue += FocusDir;
		if( FocusPulse)
		{
			FocusPulse = Off;
			BiDirOutNibbleValue += FocusPulseBit;
			FocusStepsDone++;
			FocusDiagPulseLow++;
			if( (FocusDir && !ReverseFocusMotor) || (!FocusDir && ReverseFocusMotor))
				FocusPosition++;
			else
				FocusPosition--;
		}
		else
		{
			HoldFocusSteps += FocusStepIncrPerPWM;
			if( HoldFocusSteps >= 1 && FocusStepsToMove > FocusStepsDone)
			{
				HoldFocusSteps -= 1;
				if( (FocusDir && !ReverseFocusMotor) || (!FocusDir && ReverseFocusMotor))
					FocusDiagPulseHighCW++;
				else
					FocusDiagPulseHighCCW++;
				FocusPulse = On;
			}
		}
	}
	/* leave untouched unused control lines:
	if FR on and focus on, add in nothing,
	else if FR on and focus off, then add in parallel port pins 16 and 17,
	else if FR off and focus on, then add in parallel port pins 16 and 17 if focus method is by pins
		1 and 14, or add in pins 1 and 14 if focus method is by pins 16 and 17,
	else if FR off and focus off, add in all 4 pins */
	if( PulseFR && PulseFocus)
		;
	else
		if( PulseFR && !PulseFocus)
			BiDirOutNibbleValue += UnusedFRPPortLines;
		else
			if( !PulseFR && PulseFocus)
				BiDirOutNibbleValue += UnusedFocusPPortLines;
			else
				if( !PulseFR && !PulseFocus)
					BiDirOutNibbleValue += (UnusedFRPPortLines + UnusedFocusPPortLines);

	if( PulseFR && FRPulse)
		if( FRDir)
			DisplayBiDirOut |= DisplayFRCWBit;
		else
			DisplayBiDirOut |= DisplayFRCCWBit;
	if( PulseFocus && FocusPulse)
		if( FocusDir)
			DisplayBiDirOut |= DisplayFocusOutBit;
		else
			DisplayBiDirOut |= DisplayFocusInBit;

	BiDirOutNibble();
}

/* for siderostat or uranostat or equat mounts: normally the az would be flipped 180 deg when moving
to other side of zenith, however, certain mounts cannot make this flip because the polar axis would
rotate to unacceptable positions, hence the need for this check */
void CheckSiderostatAltaz( void)
{
	if( Siderostat)
		if( Current.Az < AzLowLimit || Current.Az > AzHighLimit)
		{
			Current.Alt = M_PI - Current.Alt;
			Current.Az += M_PI;
			ValidAz( &Current);
		}
}

void ProcessLimitError( const char* Msg)
{
	if( HandpadFlag == GrandTour || HandpadFlag == ScrollTour || HandpadFlag == ScrollAutoTour)
	{
		WriteWindow( MsgFrame);
		gotoxy( MsgFrame.Left + 2, MsgFrame.Top + 2);
		printf( "%s", Msg);
		sound( 500);
		delay( 100);
		nosound();
		delay( 100);
		sound( 500);
		delay( 100);
		nosound();
		delay( 700);
		RemoveWindow( MsgFrame);
		PauseUntilNewSidTime();
	}
	else
		PressKeyToContMsg( Msg);
}


/*
Current.Az increases as scope moves west of meridian;
when on meridian, if no GEM flip, Current.Az == 0 else if GEM flip, Current.Az = 180;
if not GEM flip (scope is east facing west), then commonly 0<Current.Az<90deg
if GEM flip, then commonly 90deg<Current.Az<180deg (scope is flipped 180 degrees, plus scope is
aimed at opposite horizon);
since in equatorial alignment, az of 0 is set to south, then in northern hemisphere, az will
increase as scopes tracks, but in southern hemisphere, az will decrease, hence, GEM flip check
based on az values will be opposite that of northern hemisphere;
ie, assume flip fuzz of 1 deg:
if northern hemisphere,
	not flipped, aimed typically at 45, ok if coord between 359->0->180, otherwise flip if coord between 180 and 359,
	now flip scope by rotating az by 180 and aim scope to east, typically at 135 (meridian is now 180),
		ok if coord between 0 and 181, otherwise unflip if coord between 181 and 360,
if southern hemisphere,
	not flipped, aimed typically at 315, ok if coord between 180 and 1, otherwise flip if coord between 1 and 180,
	now flip scope by rotating az by 180 and aim scope to east, typically at 225 (meridian is 180),
		ok if coord between 179 and 0, otherwise unflip if coord between 0 and 179.

Fuzz area is defined as the area beyond the meridian where the telescope is allowed
to go before flipping.  While it may be possible to get closer to the mount for some
values of declination, maximum fuzz area is defined as that area where there's no
restriction on declination movement.
*/

Flag GEMneedsFlipping( void)
{
 	if( LatitudeDeg >= 0)
		if( GEMflippedFlag)
		{
			if( Current.Az > HalfRev+AutoGEMFlipOnFuzzRad && Current.Az < OneRev)
				return True;
		}
		else
		{
        if( AutoGEMFlipOffFuzzRad >= 0)
        {
			 if( Current.Az > HalfRev && Current.Az < OneRev-AutoGEMFlipOffFuzzRad)
				return True;
        }
        else
        {
          if( Current.Az > HalfRev || Current.Az < -AutoGEMFlipOffFuzzRad)
            return True;
        }
		}
	else
		if( GEMflippedFlag)
		{
			if( Current.Az > 0 && Current.Az < HalfRev-AutoGEMFlipOnFuzzRad)
				return True;
		}
		else
      {
        if( AutoGEMFlipOffFuzzRad >= 0)
        {
			  if( Current.Az > AutoGEMFlipOffFuzzRad && Current.Az < HalfRev)
				  return True;
        }
        else
        {
           if( Current.Az > OneRev + AutoGEMFlipOffFuzzRad || Current.Az < HalfRev)
              return True;
		  }
      }           
	return False;
}

/* scope is brought to position calculated at start of clock tick by microstepping until next clock
tick (scope is always 1 clock tick behind during tracking or microstepping) if possible, or by
halfstepping if move too large;
if slew does not bring scope close enough to initiate microstepping, then next pass through function
will also slew, eventually moving scope close enough to desired position to track by microstepping */
void MoveToCurrentRaDec( void)
{
	struct AZDouble HoldAZ;

	MoveToCurrentRaDecFlag = Yes;
	if( Two.Init)
		if( TrackFlag)
		{
			HoldAZ.A = Current.Alt;
			HoldAZ.Z = Current.Az;
			GetAltaz();
			CheckSiderostatAltaz();

			if( AutoGEMFlip && GEMneedsFlipping())
			{
			  if( SafetyZoneConflict())
			  {
				 Current.Alt = HoldAZ.A;
				 Current.Az = HoldAZ.Z;
				 TrackOff();
				 WriteTrackStatus();
				 ProcessLimitError( "Tried to move into Safety Zone");
			  }
			  else
			  {
				 GEMflippedFlag = !GEMflippedFlag;
				 GetAltaz();
				 GEMflippedFlag = !GEMflippedFlag;
			  }
			}

			if( AltLimitFlag && Current.Alt < AltLowLimit)
			{
				Current.Alt = HoldAZ.A;
				Current.Az = HoldAZ.Z;
				TrackOff();
				WriteTrackStatus();
				ProcessLimitError( "Tried to move below AltLowLimt");
			}
			else
				if( AltLimitFlag && Current.Alt > AltHighLimit)
				{
					Current.Alt = HoldAZ.A;
					Current.Az = HoldAZ.Z;
					TrackOff();
					WriteTrackStatus();
					ProcessLimitError( "Tried to move above AltHighLimt");
				}
				else
					if( AzLimitFlag && Current.Az < AzLowLimit)
					{
						Current.Alt = HoldAZ.A;
						Current.Az = HoldAZ.Z;
						TrackOff();
						WriteTrackStatus();
						ProcessLimitError( "Tried to move below AzLowLimt");
					}
					else
						if( AzLimitFlag && Current.Az > AzHighLimit)
						{
							Current.Alt = HoldAZ.A;
							Current.Az = HoldAZ.Z;
							TrackOff();
							WriteTrackStatus();
							ProcessLimitError( "Tried to move above AzHighLimt");
						}
						else
						{
							/* Delta = new - old:
							Hold., representing current telescope pointing, is corrected for various
							errors, but the position to move to, Current. is not; therefore any change in
							overall correction will not be taken care of until the next bios clock tick
							(when in effect the change in corrective values will be added to the move to
							make, ie, if the scope is to move 10 steps per tick, and if the corrective
							factors go from 0 to 5, then the scope will move 10 steps, but will in
							actuality be pointed at 15, so scope has overshot by 5, and the next move will
							be 5, bringing the scope onto target, thus the error will be the overall change
							in corrective values from bios clock tick to tick); this is done to save CPU
							cycles because of the intensive nature of calculating all the corrective
							factors twice (where the scope is truly pointed to after the move that has just
							occured, and where the scope would be truly pointed to after the proposed move)
							instead of once per bios clock tick, also, it is not conceivable that the
							change in these factors would exceed 1 arcsec over 1/18 of a second; finally, a
							delay can also occur when the MsRepsTick value in config.dat is much larger
							than the ActualMsRepsTick - here steps are not completed before running out of
							time and therefore will not be made up until the next bios clock tick */
							Delta.A = Current.Alt - HoldAZ.A;
							Delta.Z = Current.Az - HoldAZ.Z;
							Current.Alt = HoldAZ.A;
							Current.Az = HoldAZ.Z;
							/* if ok, then calc the move and make it so */
							if( !SetDirDistanceStepsThenMove())
							{
								TrackOff();
								WriteTrackStatus();
								PauseUntilNewSidTime();
								HPEventGetEquat();
							}
						}
		}
		else
		{
			PauseUntilNewSidTime();
			HPEventGetEquat();
		}
	else
		PauseUntilNewSidTime();

	MoveToCurrentRaDecFlag = No;
}

/*
incorporate backlash to move to make, making it one move; if backlash handled at a lower level, then there will be 2
moves, one for backlash and one for the original move to make;

if considering change in target positions as calculated from coordinate translation routines:
   if moving CW, then do nothing;
   if moving CCW, then lower Target by Backlash;
	the goal when moving CW is to have zero ActualBacklash, therefore, Target remains unchanged, ie, if motor is less
	than or below or behind scope because of backlash that has yet to be taken up, then motor will move Target - Scope
	+ ActualBacklash, and since motor is behind scope by ActualBacklash, Target does not need to be changed;
	the goal when moving CCW is to have ActualBacklash equal to Backlash, therefore, Target needs to be reduced by
	Backlash, ie, motor should move to Target - Backlash in order to takeout all backlash in CCW direction;

	ie, backlash of 10
	motor  scope  actbklsh  move  target  target+bklsh  motordistance  scopedistance  endmotor  endscope  endactbklsh
	 20     20     0         10    30      30 (30)       10             10             30        30        0
	 30     30     0        -10    20      10 (20-10)   -20            -10             10        20       10
	 10     20    10        -10    10       0 (10-10)   -10            -10              0        10       10
	  0     10    10         10    20      20 (20)       20             10             20        20        0
	 20     20     0        -10    10       0 (10-10)   -20            -10
		 move stopped before completion: motor distance traveled before stop is -7:     13        20        7
	 13     20     7         10    30      30 (30)       17             10             30        30        0
	 30     30     0        -10    20      10 (20-10)   -20            -10
		 move stopped before completion: motor distance traveled before stop is -7:     23        30        7
	 23     30     7        -10    20      10 (20-10)   -13            -10             10        20       10

if considering change in motor positions as calculated from coordinate translation routines then altered to include
error corrections by SetAccumMsToCurrentAltaz():
	SetAccumMsToCurrentAltaz() is unaware of direction of motion, subtracting actual backlash regardless of
	direction, so is not aware when backlash should be taken up;
	this results in backlash being taken up when motor reverses direction as follows:
		at first motor is commanded to move at standard tracking rate,
		however, AddBacklashA() and AddBacklashZ() called at end of all movement functions captures the motion
			by increasing the ActualBacklash, thus	reporting no motor movement, even though motor is moving,
		the tracking routine sees the increasing discrepancy between desired position and motor position and responds
			by increasing the commanded velocity,
		this builds to a crescendo until backlash fully taken up,
		at which time, the control algorithm commands the previously apparently unmoving motor to catch up to the
			desired target position;

	consequently, the control algorithm needs to be informed ahead of time of the backlash to take up, hence, the
	following code which is called immediately prior to calling the particular tracking algorithm in use:

	based on consistent direction, the backlash to be taken up is added to the position difference sent to the
	tracking algorithm;

if considering change in target positions as calculated from altazimuth move routines, simply subtract the required
backlash to be taken up to the target position;

if backlash fully taken up in CW direction, then ActualBacklash will be 0,
if backlash fully taken up in CCW direction, then ActualBacklash will be Backlash,
so start with backlash fully taken up in CW direction;
*/

void AddBacklashToDeltaAltazMove( void)
{
	if( Delta.A >= 0)
		Delta.A += ActualBacklashRad.A;
	else
		Delta.A -= BacklashRad.A - ActualBacklashRad.A;
	if( Delta.Z >= 0)
		Delta.Z += ActualBacklashRad.Z;
	else
		Delta.Z -= BacklashRad.Z - ActualBacklashRad.Z;
}

void AddBacklashToDeltaTracking( void)
{
	int Ix;
	Flag BacklashUnanimous;

	/* the altitude/declination motor */
	/* record direction if there is sufficient motor movement */
	if( fabs( Delta.A) > ArcsecToRad)
	{
		/* increment index to array of backlash direction values */
		LastBacklashDirIx.A++;
		if( LastBacklashDirIx.A >= LastBacklashDirSize)
			LastBacklashDirIx.A = 0;
		/* add latest direction to array */
		LastBacklashDir[LastBacklashDirIx.A].A = Dir.A;
	}
	/* see if backlash direction is consistent in the array */
	BacklashUnanimous = True;
	for( Ix = 1; Ix < LastBacklashDirSize; Ix++)
		if( LastBacklashDir[Ix].A != LastBacklashDir[0].A)
		{
			BacklashUnanimous = False;
			break;
		}
	/* if backlash takeup direction consistent, then add backlash takeup to movement to make */
	if( BacklashUnanimous)
	{
		UnanimousBacklashDir.A = LastBacklashDir[0].A;
		/* Dir. is direction of movement to make with Delta. always a positive value, so make Delta.
		+- to reflect motion without regard to Dir. */
		if( Dir.A == CCW)
			Delta.A = -Delta.A;
		/* if CW, then backlash to takeup is ActualBacklash, else if CCW, then backlash to takeup is
		Backlash - ActualBacklash */
		if( UnanimousBacklashDir.A == CW)
			Delta.A += ActualBacklashRad.A;
		else
			Delta.A -= BacklashRad.A - ActualBacklashRad.A;
		/* set new Dir. and make Delta. a positive value, per situation before this function called */
		if( Delta.A >= 0)
			Dir.A = CW;
		else
		{
			Delta.A = -Delta.A;
			Dir.A = CCW;
		}
	}

	/* the azimuth/right ascension motor */
	if( fabs( Delta.Z) > ArcsecToRad)
	{
		LastBacklashDirIx.Z++;
		if( LastBacklashDirIx.Z >= LastBacklashDirSize)
			LastBacklashDirIx.Z = 0;
		LastBacklashDir[LastBacklashDirIx.Z].Z = Dir.Z;
	}
	BacklashUnanimous = True;
	for( Ix = 1; Ix < LastBacklashDirSize; Ix++)
		if( LastBacklashDir[Ix].Z != LastBacklashDir[0].Z)
		{
			BacklashUnanimous = False;
			break;
		}
	if( BacklashUnanimous)
	{
		UnanimousBacklashDir.Z = LastBacklashDir[0].Z;
		if( Dir.Z == CCW)
			Delta.Z = -Delta.Z;
		if( UnanimousBacklashDir.Z == CW)
			Delta.Z += ActualBacklashRad.Z;
		else
			Delta.Z -= BacklashRad.Z - ActualBacklashRad.Z;
		if( Delta.Z >= 0)
			Dir.Z = CW;
		else
		{
			Delta.Z = -Delta.Z;
			Dir.Z = CCW;
		}
	}
}

Flag SetDirDistanceStepsThenMove( void)
{
	/* if tracking, add backlash after Hs/Ms selection further down in function so that if microstep
	tracking, backlash takeup will be done with microstepping, not halfstepping (halfstep means that
	both axis put into halfstep slew mode, ruining precision tracking to sub-arcsecond accuracy in
	axis that does not need the backlash takeup);
	if not tracking, then add backlash to takeup directly to Delta values */
	if( !MoveToCurrentRaDecFlag && BacklashFlag)
		AddBacklashToDeltaAltazMove();

	/* altitude... */
	if( Delta.A > HalfRev)
   /* ie, a move of 270 should result in a move of -90 */
		Delta.A = OneRev - Delta.A;
	if( Delta.A < -HalfRev)
		Delta.A = -OneRev - Delta.A;

	if( Delta.A < 0)
	{
		Delta.A = -Delta.A;
		Dir.A = CCW;
	}
	else
		Dir.A = CW;

	/* azimuth (increases CW)...
	4 possibilities of calculating az steps to move:
		start		end		delta		+ 360 if nec.	 > 180?	result
	1. 350,     200,     -150,    210,           yes,     -150
	2. 200,     350,     150,                    no,      150
	3. 350,     100,     -250,    110,           no,      110
	4. 100,     350,     250,                    yes,     -110 */
	if( Delta.Z < 0)
		Delta.Z += OneRev;
	/* if > 1/2 rev, go other way */
	if( Delta.Z > M_PI)
		/* get distance to go (- or +) */
		Delta.Z -= OneRev;
	if( Delta.Z < 0)
	{
		Delta.Z = -Delta.Z;
		Dir.Z = CCW;
	}
	else
		Dir.Z = CW;

	/* check to see if distance can be covered by microstepping */
	if( Delta.A <= MaxMsDistanceRadTrack.A && Delta.Z <= MaxMsDistanceRadTrack.Z)
	{
		if( MoveToCurrentRaDecFlag && BacklashFlag)
			AddBacklashToDeltaTracking();

		Steps.A = (long) (0.5 + Delta.A/MsRad.A);
		Steps.Z = (long) (0.5 + Delta.Z/MsRad.Z);

		ConsecutiveSlews = 0;
		if( SlewBeep == SlewUnderway)
			SlewBeep = SlewFinished;
		HPEventMoveMs();
		return True;
	}
	else
	{
		if( MoveToCurrentRaDecFlag && BacklashFlag)
			AddBacklashToDeltaTracking();

		Steps.A = (long) (0.5 + (Delta.A/HsRad.A));
		Steps.Z = (long) (0.5 + (Delta.Z/HsRad.Z));

		/* if too many steps, confirm move */
		if( ConfirmHsMove())
		{
			ConsecutiveSlews++;
			if( SlewBeep != SlewBeepOn)
				SlewBeep = SlewUnderway;
			KBEventMoveHs();
			return True;
		}
		else
			return False;
	}
}

void HPEventGetEquat( void)
{
	if( Two.Init)
		GetEquat();
}

void UpdateDriftCalculatedFromGuide( void)
{
	double TimeMin = (EndDriftT - StartDriftT) * RadToMin;

	Drift.Alt += AccumGuide.A/TimeMin;
	Drift.Az += AccumGuide.Z/TimeMin;

	CalcCommonDriftVars();
}

/* record equatorial position when handpad mode switch activated */
void RecordEquat( void)
{
	Output = fopen( RecordEquatFile, "a");
	if( Output == NULL)
		BadExit( strcat( "Could not open ", RecordEquatFile));

	GetHMSH( RadToHundSec*Current.Ra + 0.5, &Current.RaHMSH);
	GetDMS( RadToArcsec*Current.Dec + 0.5, &Current.DecDMS);
	if( Current.DecDMS.Sign == Minus)
	{
		Current.DecDMS.Deg = -Current.DecDMS.Deg;
		Current.DecDMS.Min = -Current.DecDMS.Min;
		Current.DecDMS.Sec = -Current.DecDMS.Sec;
	}
	fprintf( Output, "%3d %3d %3d   %3d %3d %3d   handpad_record\n",
	Current.RaHMSH.Hr, Current.RaHMSH.Min, Current.RaHMSH.Sec,
	Current.DecDMS.Deg, Current.DecDMS.Min, Current.DecDMS.Sec );

	fclose( Output);
}

Flag IACA_Event( void)
{
	if( NewIACA())
	{
		GetIACA( &In.Ra, &In.Dec);
		GetHMSH( In.Ra*RadToHundSec+.5, &In.RaHMSH);
		GetDMS( In.Dec*RadToArcsec+.5, &In.DecDMS);
		DisplayIn( "from IACA", NameBlanks);
		return True;
	}
	else
		return False;
}

int FindClosestInit( void)
{
	double Init1Distance, Init2Distance, Init3Distance;

	Init1Distance = CalcEquatAngularSep( &Current, &One);
	Init2Distance = CalcEquatAngularSep( &Current, &Two);
	Init3Distance = CalcEquatAngularSep( &Current, &Three);
	if( Init1Distance < Init2Distance && Init1Distance < Init3Distance)
		return 1;
	else
		if( Init2Distance < Init3Distance)
			return 2;
		else
			return 3;
}

void SetEndDriftT( void)
{
	EndDriftT = Current.SidT;
	/* if cross 24 hr == 0hr boundary */
	while( EndDriftT < StartDriftT)
		EndDriftT += OneRev;
}

void AddDragToDrift( void)
{
	Drift.Alt += Drag.Alt;
	Drift.Az += Drag.Az;
	Drift.Ra += Drag.Ra;
	Drift.Dec += Drag.Dec;
	CalcCommonDriftVars();
}

void RemoveDragFromDrift( void)
{
	Drift.Alt -= Drag.Alt;
	Drift.Az -= Drag.Az;
	Drift.Ra -= Drag.Ra;
	Drift.Dec -= Drag.Dec;
	CalcCommonDriftVars();
}

void CalcCommonDriftVars( void)
{
	/* drift per bios clock tick */
	DriftTick.Alt = Drift.Alt/ClockTicksMin;
	DriftTick.Az = Drift.Az/ClockTicksMin;
	DriftTick.Ra = Drift.Ra/ClockTicksHr;
	DriftTick.Dec = Drift.Dec/ClockTicksHr;

	if( Drift.Alt != 0 || Drift.Az != 0)
		DriftAltazFlag = Yes;
	else
		DriftAltazFlag = No;
	if( Drift.Ra != 0 || Drift.Dec != 0)
		DriftEquatFlag = Yes;
	else
		DriftEquatFlag = No;

	GetHMSH( RadToHundSec*Drift.Ra + .5, &Drift.RaHMSH);
	GetDMS( RadToArcsec*Drift.Dec + .5, &Drift.DecDMS);
}

void CheckLX200Events( void)
{
	if( LX200ComPort)
	{
		// for slewing, allow lx200 port to process as long as data is available
		if( MoveHsUnderway)
			while(ReadLX200Input())
				;
		else
			ReadLX200Input();

		if( SlewState == SlewDone && (   LX200_Motor_Cmd == StartSlew
												|| LX200_Motor_Cmd == MoveDirRateNorth
												|| LX200_Motor_Cmd == MoveDirRateSouth
												|| LX200_Motor_Cmd == MoveDirRateEast
												|| LX200_Motor_Cmd == MoveDirRateWest)
											  )
			ProcessLX200_Motor_Cmd();
	}
}

void CheckMiscEvents( void)
{
	if( IACA_Flag && IACA_Event())
		;
	else
	{
		CheckLX200Events();
		if( TrackByRateFlag)
			TrackByRate();
		else
			MoveToCurrentRaDec();
	}

	/* handle consecutive slews when scope should be microstepping tracking;
	by resetting current equatorial to current altazimuth, tracking can start fresh from current
	position instead of repeatedly trying to slew to previous current equatorial coordinates */
	if( ConsecutiveSlews >= MaxConsecutiveSlews)
	{
		ConsecutiveSlews = 0;
		PauseUntilNewSidTime();
		HPEventGetEquat();
		In = Current;
		DisplayIn( "consecutive slews", NameBlanks);
	}
}

/* Look at parallel port pin 15; toggling the pin by grounding will trigger autosynch when grounding
is removed; if AutoAzPECSyncLowHighFlag, then autosynch occurs at moment of grounding.
Look at parallel port pin 17; raising the pin to +5VDC then grounding the pin will trigger autosynch
when first grounded; if AutoAltPECSyncLowHighFlag, autosynch occurs at moment of +5VDC */

void SetAutoAltPECSynchSignal( void)
{
	switch( AutoAltPECPin)
	{
		case 15:
			READ_AUTO_PEC_SYNC_15;
			break;
		case 16:
			READ_AUTO_PEC_SYNC_16;
			break;
		case 17:
			READ_AUTO_PEC_SYNC_17;
			break;
		case 101213l:
			AutoPECSyncSignal = AutoPECSynch101213Detected;
			break;
	}
	AutoAltPECSyncSignal = AutoPECSyncSignal;
}

void SetAutoAzPECSynchSignal( void)
{
	switch( AutoAzPECPin)
	{
		case 15:
			READ_AUTO_PEC_SYNC_15;
			break;
		case 16:
			READ_AUTO_PEC_SYNC_16;
			break;
		case 17:
			READ_AUTO_PEC_SYNC_17;
			break;
		case 101213l:
			AutoPECSyncSignal = AutoPECSynch101213Detected;
			break;
	}
	AutoAzPECSyncSignal = AutoPECSyncSignal;
}

void CheckAutoAltPECDeBounce( void)
{
	if( AutoAltPECDeBounce == 2)
		if( PECIx.A > PECSize/4 && PECIx.A < PECSize*3/4)
			AutoAltPECDeBounce = 1;
}

void CheckAutoAzPECDeBounce( void)
{
	if( AutoAzPECDeBounce == 2)
		if( PECIx.Z > PECSize/4 && PECIx.Z < PECSize*3/4)
			AutoAzPECDeBounce = 1;
}

void CheckAltPECSynch( void)
{
	SetAutoAltPECSynchSignal();
	if( AutoAltPECSyncLowHighFlag)
		AutoAltPECSyncSignal = !AutoAltPECSyncSignal;
	if( AutoAltPECSyncSignal && !LastAutoAltPECSyncSignal
	&& (!AutoAltPECSyncDirFlag || AutoAltPECSyncDirFlag == CCW && Dir.A == CCW
	|| AutoAltPECSyncDirFlag == CW && Dir.A == CW))
	{
		/* synch sensor has just turned on with Alt motor moving in CW direction */
		LastPECIxOffset.A = PECIxOffset.A;
		SetPECIxOffsetA();
		/* update the synch display */
		DisplayAltAutoSynchFlag = Yes;
		WritePECSynchStatus();
		DisplayAltAutoSynchFlag = No;
		/* temporarily change the pec display to indicate synch point */
		TextAttr = SelectText;
		DisplayAltPEC();
		/* set debounce if on */
		if( AutoAltPECDeBounce)
			AutoAltPECDeBounce = 2;
	}
	LastAutoAltPECSyncSignal = AutoAltPECSyncSignal;
}

void CheckAzPECSynch( void)
{
	SetAutoAzPECSynchSignal();
	if( AutoAzPECSyncLowHighFlag)
		AutoAzPECSyncSignal = !AutoAzPECSyncSignal;
	if( AutoAzPECSyncSignal && !LastAutoAzPECSyncSignal
	&& (!AutoAzPECSyncDirFlag || AutoAzPECSyncDirFlag == CCW && Dir.Z == CCW
	|| AutoAzPECSyncDirFlag == CW && Dir.Z == CW))
	{
		/* synch sensor has just turned on with Az motor moving in CW direction */
		LastPECIxOffset.Z = PECIxOffset.Z;
		SetPECIxOffsetZ();
		/* update the synch display */
		DisplayAzAutoSynchFlag = Yes;
		WritePECSynchStatus();
		DisplayAzAutoSynchFlag = No;
		/* temporarily change the pec display to indicate synch point */
		TextAttr = SelectText;
		DisplayAzPEC();
		/* set debounce if on */
		if( AutoAzPECDeBounce)
			AutoAzPECDeBounce = 2;
	}
	LastAutoAzPECSyncSignal = AutoAzPECSyncSignal;
}

void ProcessHPEventsGuide( void)
{
	if( HandpadFlag == GuideStayRotateOn)
	{
		if( Buttons & UpKey)
		{
			AccumGuide.A -= cos( GuideFRAngle) * GuideRadTick;
			AccumGuide.Z -= sin( GuideFRAngle) * GuideRadTick;
		}
		if( Buttons & DownKey)
		{
			AccumGuide.A += cos( GuideFRAngle) * GuideRadTick;
			AccumGuide.Z += sin( GuideFRAngle) * GuideRadTick;
		}
		if( Buttons & CWKey)
		{
			AccumGuide.Z -= cos( GuideFRAngle) * GuideRadTick;
			AccumGuide.A += sin( GuideFRAngle) * GuideRadTick;
		}
		if( Buttons & CCWKey)
		{
			AccumGuide.Z += cos( GuideFRAngle) * GuideRadTick;
			AccumGuide.A -= sin( GuideFRAngle) * GuideRadTick;
		}
	}
	else
	{
		if( Buttons & UpKey)
			AccumGuide.A -= GuideRadTick;
		if( Buttons & DownKey)
			AccumGuide.A += GuideRadTick;
		if( Buttons & CWKey)
			AccumGuide.Z -= GuideRadTick;
		if( Buttons & CCWKey)
			AccumGuide.Z += GuideRadTick;
	}
}

/* for moving the FR motor while tracking is off */
void ProcessHPEventsFR( void)
{
	SetFRControlLines();
	/* set field rotation motor direction */
	if( Buttons & CCWKey)
		ReverseFRDir();
	/* reverse motor direction if necessary */
	if( ReverseFRMotor)
		ReverseFRDir();
	/* get state of port so as to not disturb parallel port pins 16 and 17 */
	GetUnusedPPortLines();
	/* setup handpad vars */
	InitHandpad = Handpad;
	HandpadOKFlag = Yes;
	/* keep control here */
	while( HandpadOKFlag && !KeyStroke)
	{
		delay( FRStepSpeedMilliSec/2);
		if( FRDir)
			DisplayBiDirOut |= DisplayFRCWBit;
		else
			DisplayBiDirOut |= DisplayFRCCWBit;
		/* raise field rotation motor pulse on parallel port pin 1 */
		BiDirOutNibbleValue = UnusedFRPPortLines + FRPulseBit + FocusDir;
		BiDirOutNibble();
		if( FRDir || (!FRDir && ReverseFRMotor))
		{
			FRDiagPulseHighCW++;
			FRMotorAngle += FRStepSize;
		}
		else
		{
			FRDiagPulseHighCCW++;
			FRMotorAngle -= FRStepSize;
		}
		delay( FRStepSpeedMilliSec/2);
		/* lower field rotation motor pulse */
		BiDirOutNibbleValue = UnusedFRPPortLines + FRDir;
		BiDirOutNibble();
		FRStepsDone++;
		FRDiagPulseLow++;
		SetHandpadOKFlag();
		/* other movement functions that retain control until user stops the movement use MoveMs()
		which calls NewSidT() */
		NewSidT();
		HPEventGetEquat();
		CheckLX200Events();
		SequentialTaskController();
	}
	if( KeyStroke)
		getch();
	FRMotorAngle = FieldR;
}

/* for moving the focus motor while tracking is off */
void ProcessHPEventsFocusMethod_Pulse_Dir( void)
{
	int msdelay;

	SetFocusControlLines();
	/* set focus motor direction */
	if( InitiatedType == HandpadInitiated && Buttons & CCWKey ||
	InitiatedType == LX200Initiated && LX200_Focus_Cmd == FocusIn ||
	InitiatedType == MouseInitiated && (MouseMoveResult == MouseMovedLeft || MouseMoveResult == MouseMovedDown))
		ReverseFocusDir();
	if( ReverseFocusMotor)
		ReverseFocusDir();
	/* set focus motor speed */
	if( InitiatedType == HandpadInitiated && MsSpeed ||
	InitiatedType == LX200Initiated && LX200_Focus_Speed_Cmd == FocusSetSlow ||
	InitiatedType == MouseInitiated && (MouseMoveResult == MouseMovedUp || MouseMoveResult == MouseMovedDown))
	{
		FocusFastDisplayFlag = False;
		msdelay = 1000 / (FocusSlowStepsSec * 2);
	}
	else
	{
		FocusFastDisplayFlag = True;
		msdelay = 1000 / (FocusFastStepsSec * 2);
	}
	if( msdelay < 1)
		msdelay = 1;
	/* get state of port so as to not disturb unused parallel port pins */
	GetUnusedPPortLines();
	/* setup handpad vars */
	InitHandpad = Handpad;
	HandpadOKFlag = Yes;
	/* keep control here */
	while( HandpadOKFlag &&	!KeyStroke &&
	(InitiatedType != LX200Initiated || InitiatedType == LX200Initiated && LX200_Focus_Cmd != FocusQuit) &&
	(InitiatedType != MouseInitiated || InitiatedType == MouseInitiated && !MouseLeftButtonReleaseCount()))
	{
		InsideProcessHPEventsFocusMethod_Pulse_Dir = Yes;
		delay( msdelay);
		if( FocusDir)
			DisplayBiDirOut |= DisplayFocusOutBit;
		else
			DisplayBiDirOut |= DisplayFocusInBit;
		/* raise focus motor pulse */
		BiDirOutNibbleValue = UnusedFocusPPortLines + FocusPulseBit + FocusDir;
		BiDirOutNibble();
		if( (FocusDir && !ReverseFocusMotor) || (!FocusDir && ReverseFocusMotor))
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
		/* other movement functions that retain control until user stops the movement use MoveMs()
		which calls NewSidT() */
		NewSidT();
		HPEventGetEquat();
		CheckLX200Events();
		SequentialTaskController();
	}
	if( KeyStroke)
		getch();
	InsideProcessHPEventsFocusMethod_Pulse_Dir = No;
}

/* raise/lower the bidrectional parallel port pins */
void ProcessHPEventsHandpadAux( void)
{
	InitiatedType = HandpadInitiated;
	switch( Buttons)
	{
		case UpKey:
			AuxControl = Aux1;
			HandpadAuxButtonPressed = Yes;
			break;
		case DownKey:
			AuxControl = Aux14;
			HandpadAuxButtonPressed = Yes;
			break;
		case CWKey:
			AuxControl = Aux16;
			HandpadAuxButtonPressed = Yes;
			break;
		case CCWKey:
			AuxControl = Aux17;
			HandpadAuxButtonPressed = Yes;
			break;
		default:
			AuxControl = AuxOff;
			HandpadAuxButtonPressed = No;
	}
	AuxControlBiDirPPort();
}

/* raise/lower the bidrectional parallel port pins */
void AuxControlBiDirPPort( void)
{
	BiDirInNibbleValue = BiDirInNibble();
	switch( AuxControl)
	{
		case Aux1:
			/* parallel port pin #1 */
			BiDirOutNibbleValue = (BiDirInNibbleValue & PPortPins16_17) + PPortPin1;
			DisplayBiDirOut |= DisplayAux1Bit;
			break;
		case Aux14:
			/* parallel port pin #14 */
			BiDirOutNibbleValue = (BiDirInNibbleValue & PPortPins16_17) + PPortPin14;
			DisplayBiDirOut |= DisplayAux14Bit;
			break;
		case Aux16:
			/* parallel port pin #16 */
			BiDirOutNibbleValue = (BiDirInNibbleValue & PPortPins1_14) + PPortPin16;
			DisplayBiDirOut |= DisplayAux16Bit;
			break;
		case Aux17:
			/* parallel port pin #17 */
			BiDirOutNibbleValue = (BiDirInNibbleValue & PPortPins1_14) + PPortPin17;
			DisplayBiDirOut |= DisplayAux17Bit;
			break;
		case AuxOff:
			BiDirInNibbleValue = Off;
	}
	BiDirOutNibble();
}

void ProcessHPEventsFocus_OnOff_16_17( void)
{
	InitiatedType = HandpadInitiated;
	if( Buttons & CCWKey)
	{
		FocusControl = FocusMinus;
		HandpadFocusButtonPressed = Yes;
	}
	else
		if( Buttons & CWKey)
		{
			FocusControl = FocusPlus;
			HandpadFocusButtonPressed = Yes;
		}
		else
		{
			FocusControl = FocusStop;
			HandpadFocusButtonPressed = No;
		}

	FocusControl_OnOff_16_17();
}

void FocusControl_OnOff_16_17( void)
{
	BiDirInNibbleValue = BiDirInNibble();
	switch( FocusControl)
	{
		case FocusPlus:
			if( ReverseFocusMotor)
				FocusPulseBit = PPortPin17;
			else
				FocusPulseBit = PPortPin16;
			break;
		case FocusMinus:
			if( ReverseFocusMotor)
				FocusPulseBit = PPortPin16;
			else
				FocusPulseBit = PPortPin17;
			break;
		case FocusStop:
			FocusPulseBit = Off;
	}
	if( FocusPulseBit == PPortPin16)
		DisplayBiDirOut |= DisplayFocusOutBit;
	else
		if( FocusPulseBit == PPortPin17)
			DisplayBiDirOut |= DisplayFocusInBit;
	BiDirOutNibbleValue = (BiDirInNibbleValue & PPortPins1_14) + FocusPulseBit;
	BiDirOutNibble();
	FocusFastDisplayFlag = True;
}

void ProcessHPEventsFocus_OnOff_16_17_Slow1_14( void)
{
	if( !MsSpeed)
		ProcessHPEventsFocus_OnOff_16_17();
	else
	{
		InitiatedType = HandpadInitiated;
		if( Buttons & CCWKey)
		{
			FocusControl = FocusMinus;
			HandpadFocusButtonPressed = Yes;
		}
		else
			if( Buttons & CWKey)
			{
				FocusControl = FocusPlus;
				HandpadFocusButtonPressed = Yes;
			}
			else
			{
				FocusControl = FocusStop;
				HandpadFocusButtonPressed = No;
			}

		FocusControl_OnOff_16_17_Slow1_14();
	}
}

void FocusControl_OnOff_16_17_Slow1_14( void)
{
	BiDirInNibbleValue = BiDirInNibble();
	switch( FocusControl)
	{
		case FocusPlus:
			if( ReverseFocusMotor)
				FocusPulseBit = PPortPin14;
			else
				FocusPulseBit = PPortPin1;
			break;
		case FocusMinus:
			if( ReverseFocusMotor)
				FocusPulseBit = PPortPin1;
			else
				FocusPulseBit = PPortPin14;
			break;
		case FocusStop:
			FocusPulseBit = Off;
	}
	if( FocusPulseBit == PPortPin1)
		DisplayBiDirOut |= DisplayFocusOutBit;
	else
		if( FocusPulseBit == PPortPin14)
			DisplayBiDirOut |= DisplayFocusInBit;
	BiDirOutNibbleValue = (BiDirInNibbleValue & PPortPins16_17) + FocusPulseBit;
	BiDirOutNibble();
	FocusFastDisplayFlag = False;
}

/* add accum guide, drift to current position */
void AddAccumGuideDriftToCurrentPosition( void)
{
	SetCurrentAltazToAccumMs();
	HPEventGetEquat();
	SetEncoderAZandEncoderOffset();
}

Flag CheckStartGuideForWriteNSave( void)
{
	if( PECFlag)
	{
	  if( GuideArrayFlag == No)
		{
			InitSaveGuideArray();
			HandpadButtonTimer = StartHandpadTimer;
			return True;
		}
	}
	else
		PressKeyToContMsg("PEC must be enabled before guiding can be saved");
	return False;
}

Flag CheckReadyToSaveGuide( void)
{
	if (GuideArrayFlag == WritingToGuideArray)
	{
		HandpadButtonTimer = StartHandpadTimer;
		GuideArrayFlag = ReadyToSaveGuideArray;
		return True;
	}
	return False;
}


Flag CheckSaveGuideArray( void)
{
	if( GuideArrayFlag == ReadyToSaveGuideArray)
	{
		SaveGuideArray();
		return True;
	}
	return False;
}

void ProcessHPEventsMoveMotors( void)
{
	/* align motors to fullstep in case halfstepping chosen */
	AlignMs_f_ptr();
	SetCurrentAltazToAccumMs();
	/* if microstep switch selection on */
	if( MsSpeed)
	{
		StepsToMove.A = MsTick.A;
		StepsToMove.Z = MsTick.Z;
	}
	/* else halfstep */
	else
		StepsToMove.A = StepsToMove.Z = MaxHs;

	Steps.A = Steps.Z = 0;
	switch( Buttons)
	{
		case UpKey:
			Steps.A = StepsToMove.A;
			Dir.A = CW;
			break;
		case DownKey:
			Steps.A = StepsToMove.A;
			Dir.A = CCW;
			break;
		case CCWKey:
			Steps.Z = StepsToMove.Z;
			Dir.Z = CCW;
			break;
		case CWKey:
			Steps.Z = StepsToMove.Z;
			Dir.Z = CW;
	}
	if( MsSpeed)
		/* keeps control until Ms mvmt ends */
		AltazMoveMs();
	else
	{
		SlewStartedFromHandpad = True;
		KBEventMoveHs();
		SlewStartedFromHandpad = False;
		PauseUntilNewSidTime();
		HPEventGetEquat();
	}
}

void ProcessHPEventsModeSwitch( void)
{
	switch( HandpadFlag)
	{
		case HandpadOff:
			CheckMiscEvents();
			break;
		case InitAutoOn:
			if( (Buttons & LeftKey || Buttons & RightKey) && !HandpadButtonTimer)
			{
				HandpadButtonTimer = StartHandpadTimer;
				/* use Current altaz and In equat coordinates to init with */
				Current.Ra = In.Ra;
				Current.Dec = In.Dec;
				strcpy( WhyInit, WHY_INIT_HANDPAD);
				if( Three.Init)
					/* replace previous closest init with new init */
					KBEventInitMatrix( FindClosestInit());
				else
					if( Two.Init)
						KBEventInitMatrix( 3);
					else
						if( One.Init)
							KBEventInitMatrix( 2);
						else
							KBEventInitMatrix( 1);
			}
			else
				CheckMiscEvents();
			break;
		case Init1On:
			if( (Buttons & LeftKey || Buttons & RightKey) && !HandpadButtonTimer)
			{
				HandpadButtonTimer = StartHandpadTimer;
				/* use Current altaz and In equat coordinates to init with */
				Current.Ra = In.Ra;
				Current.Dec = In.Dec;
				strcpy( WhyInit, WHY_INIT_HANDPAD);
				KBEventInitMatrix( 1);
			}
			else
				CheckMiscEvents();
			break;
		case Init2On:
			if( (Buttons & LeftKey || Buttons & RightKey) && !HandpadButtonTimer)
			{
				if( One.Init)
				{
					HandpadButtonTimer = StartHandpadTimer;
					/* use Current altaz and In equat coordinates to init with */
					Current.Ra = In.Ra;
					Current.Dec = In.Dec;
					strcpy( WhyInit, WHY_INIT_HANDPAD);
					KBEventInitMatrix( 2);
				}
				else
					PressKeyToContMsg( "Must init 1 first");
			}
			else
				CheckMiscEvents();
			break;
		case Init3On:
			if( (Buttons & LeftKey || Buttons & RightKey) && !HandpadButtonTimer)
			{
				if( Two.Init)
				{
					HandpadButtonTimer = StartHandpadTimer;
					/* use Current altaz and In equat coordinates to init with */
					Current.Ra = In.Ra;
					Current.Dec = In.Dec;
					strcpy( WhyInit, WHY_INIT_HANDPAD);
					KBEventInitMatrix( 3);
				}
				else
					PressKeyToContMsg( "Must init 2 first");
			}
			else
				CheckMiscEvents();
			break;
		case HandpadPolarAlign:
			if( Buttons & LeftKey)
				HPPolarAlignFlag = Yes;
			CheckMiscEvents();
			break;
		case AnalyzeOn:
			if( (Buttons & LeftKey || Buttons & RightKey) && !HandpadButtonTimer)
			{
				if( Two.Init)
				{
					HandpadButtonTimer = StartHandpadTimer;
					WriteAnalysisFile();
				}
				else
					PressKeyToContMsg( "Must init 2 first");
			}
			else
				CheckMiscEvents();
			break;
		case GuideOn:
			CheckSaveGuideArray();
			if( (Buttons & LeftKey && Two.Init) && !HandpadButtonTimer)
			{
				if( !GuideFlag)
				{
					HandpadButtonTimer = StartHandpadTimer;
					if( HPUpdateDriftFlag)
						StartDriftT = Current.SidT;
					InitGuide();
					WriteHandpadStatus();
				}
				else
					CheckStartGuideForWriteNSave();
			}
			else
				if( (Buttons & RightKey) && !HandpadButtonTimer)
				{
					if( CheckReadyToSaveGuide())
						;
					else
						if( GuideFlag)
						{
							HandpadButtonTimer = StartHandpadTimer;
							if( HPUpdateDriftFlag)
							{
								SetEndDriftT();
								UpdateDriftCalculatedFromGuide();
							}
							StopGuide();
							if( HPUpdateDriftFlag)
								DisplayDrift();
							WriteRemoveAccumGuide();
							WriteHandpadStatus();
						}
				}
				else
					CheckMiscEvents();
			break;
		case GuideStayOn:
		case GuideStayRotateOn:
			CheckSaveGuideArray();
			if( (Buttons & LeftKey && Two.Init) && !HandpadButtonTimer)
			{
				if( !GuideFlag)
				{
					HandpadButtonTimer = StartHandpadTimer;
					if( HPUpdateDriftFlag)
						StartDriftT = Current.SidT;
					InitGuide();
					WriteHandpadStatus();
				}
				else
					CheckStartGuideForWriteNSave();
			}
			else
				if( (Buttons & RightKey) && !HandpadButtonTimer)
				{
					if( CheckReadyToSaveGuide())
						;
					else
						if( GuideFlag)
						{
							HandpadButtonTimer = StartHandpadTimer;
                     // following 2 procedures will cause scope to stay put after guiding finished
							AlignMs_f_ptr();
							AddAccumGuideDriftToCurrentPosition();
							if( HPUpdateDriftFlag)
							{
								SetEndDriftT();
								UpdateDriftCalculatedFromGuide();
							}
							StopGuide();
							// AccumMs = new altaz that includes accum guide=0, drift
							SetAccumMsToCurrentAltaz();
							if( HPUpdateDriftFlag)
								DisplayDrift();
							WriteRemoveAccumGuide();
							WriteHandpadStatus();
						}
				}
				else
					CheckMiscEvents();
			break;
		case GuideDragOn:
			/* adds in drag drift from config.dat, then removes it */
			/* if drift to be updated, then follow this algorithm (remember to work only with altaz
			drift as driftdrag does all 4 axes): if current drift = 10, and dragdrift = 20, then drift
			will be 30, at end of guide cycle, drift calculated from guide will = -20, take the
			difference between dragdrift and calculated drift (here 20-20=0) and add it to the drift of
			30, then remove dragdrift of 20 for final current drift of 10; if drift should really be 5,
			then drift calculated from guide would be -25, amount to subtract would be -5, and after
			subtracting dragdrift of 20, result would be 5 */
			CheckSaveGuideArray();
			if( (Buttons & LeftKey && Two.Init) && !HandpadButtonTimer)
			{
				if( !GuideFlag)
				{
					HandpadButtonTimer = StartHandpadTimer;
					InitGuide();
					AddDragToDrift();
					DisplayDrift();
					WriteHandpadStatus();
				}
				else
					CheckStartGuideForWriteNSave();
			}
			else
				if( (Buttons & RightKey) && !HandpadButtonTimer)
				{
					if( CheckReadyToSaveGuide())
						;
					else
						if( GuideFlag)
						{
							HandpadButtonTimer = StartHandpadTimer;
							AlignMs_f_ptr();
							AddAccumGuideDriftToCurrentPosition();
							StopGuide();
							// AccumMs = new altaz that includes accum guide=0, drift
							SetAccumMsToCurrentAltaz();
							RemoveDragFromDrift();
							DisplayDrift();
							WriteHandpadStatus();
						}
				}
				else
					CheckMiscEvents();
			break;
		case GrandTour:
			/* sets flag, then continues tracking, flag set to no in ProcessGrandTour() */
			if( Two.Init && (Buttons & LeftKey || Buttons & RightKey))
				GrandTourFlag = Yes;
			CheckMiscEvents();
			break;
		case ScrollTour:
		case ScrollAutoTour:
			/* sets flag, then continues tracking if possible; flag set to no in
			Move_Update_Handpad_Subr() called from ProcessScroll() */
			if( Buttons & LeftKey)
				ScrollFlag = Yes;
			CheckMiscEvents();
			break;
		case RecordOn:
			if( Two.Init && (Buttons & LeftKey || Buttons & RightKey)
			&& !RecordEquatTimer)
			{
				RecordEquatTimer = StartHandpadTimer;
				RecordEquat();
			}
			CheckMiscEvents();
			break;
		case ToggleTrack:
			if( Buttons & LeftKey)
			{
				TrackFlag = On;
				WriteTrackStatus();
			}
			else
				if( Buttons & RightKey)
				{
					TrackOff();
					WriteTrackStatus();
				}
			CheckMiscEvents();
			break;
		case HandpadFR:
			if( Buttons & LeftKey)
				if( FocusMethod == FocusMethod_Pulse_1_14)
					PressKeyToContMsg( "Focus method precludes field rotation motor use");
				else
				{
					HandpadFRMotorControlOnFlag = On;
					WriteHandpadStatus();
				}
			else
				if( Buttons & RightKey)
				{
					HandpadFRMotorControlOnFlag = Off;
					WriteHandpadStatus();
				}
			CheckMiscEvents();
			break;
		case HandpadFocus:
			if( Buttons & LeftKey)
			{
				HandpadFocusMotorControlOnFlag = On;
				WriteHandpadStatus();
			}
			else
				if( Buttons & RightKey)
				{
					HandpadFocusMotorControlOnFlag = Off;
					WriteHandpadStatus();
				}
			CheckMiscEvents();
			break;
		case HandpadAux:
			if( Buttons & LeftKey)
			{
			/* turned off in ProcessHPEvents() */
				HandpadAuxControlOnFlag = On;
				WriteHandpadStatus();
			}
			CheckMiscEvents();
		default:
			CheckMiscEvents();
	}
}

void ProcessHPEvents( void)
{
	/* if a directional button is pressed, and the handpad mode is changed away from HandpadAux,
	call ProcessHPEventsHandpadAux() in order to set the bidirectional port pins properly */
	if( HandpadAuxButtonPressed && HandpadFlag != HandpadAux)
		ProcessHPEventsHandpadAux();
	/* similarly for focus */
	if( HandpadFocusButtonPressed && HandpadFlag != HandpadFocus)
		if( FocusMethod == FocusMethod_OnOff_16_17)
			ProcessHPEventsFocus_OnOff_16_17();
		else
			if( FocusMethod == FocusMethod_OnOff_16_17_Slow1_14)
				ProcessHPEventsFocus_OnOff_16_17_Slow1_14();

	/* set default so that upon entering handpad mode to operate the field rotation motor, buttons
	will initially control the A and Z motors, not the field rotation motor */
	if( HandpadFRMotorControlOnFlag && HandpadFlag != HandpadFR)
		HandpadFRMotorControlOnFlag = Off;
	/* similarly for focus */
	if( HandpadFocusMotorControlOnFlag && HandpadFlag != HandpadFocus)
		HandpadFocusMotorControlOnFlag = Off;

	/* to prevent too rapid init'ing */
	if( HandpadButtonTimer)
		HandpadButtonTimer--;
	/* to prevent too rapid recording of equat position */
	if( RecordEquatTimer)
		RecordEquatTimer--;

	/* if PEC on and auto sync of PEC on, then check alt PEC sync signal */
	if( PECFlag && AutoAltPECSyncOnFlag)
	{
		CheckAutoAltPECDeBounce();
		if( AutoAltPECDeBounce < 2)
			CheckAltPECSynch();
	}

	/* if PEC on and auto sync of PEC on, then check az PEC sync signal */
	if( PECFlag && AutoAzPECSyncOnFlag)
	{
		CheckAutoAzPECDeBounce();
		if( AutoAzPECDeBounce < 2)
		CheckAzPECSynch();
	}

	/* check for handpad event... */
	ReadHandpad_f_ptr();

	/* if handpad mode is set to auxiliary output, then process auxiliary handpad control, whether a
	button is pressed or not */
	if( HandpadFlag == HandpadAux && HandpadAuxControlOnFlag)
	{
		if( Buttons & RightKey)
		{
			HandpadAuxControlOnFlag = Off;
			WriteHandpadStatus();
		}
		else
			ProcessHPEventsHandpadAux();
		CheckMiscEvents();
	}
	else
		/* similarly for focus mode */
		if( HandpadFlag == HandpadFocus && HandpadFocusMotorControlOnFlag
		&& (FocusMethod == FocusMethod_OnOff_16_17 || FocusMethod == FocusMethod_OnOff_16_17_Slow1_14))
		{
			if( Buttons & RightKey)
			{
				HandpadFocusMotorControlOnFlag = Off;
				WriteHandpadStatus();
			}
			else
				if( FocusMethod == FocusMethod_OnOff_16_17)
					ProcessHPEventsFocus_OnOff_16_17();
				else
					if( FocusMethod == FocusMethod_OnOff_16_17_Slow1_14)
						ProcessHPEventsFocus_OnOff_16_17_Slow1_14();
			CheckMiscEvents();
		}
		else
		{
			/* if direction button pressed: */
			if( Buttons & UpKey || Buttons & DownKey || Buttons & CCWKey || Buttons & CWKey)
			{
				/* if guide on, then use guiding speed */
				if( GuideFlag)
				{
					ProcessHPEventsGuide();
					if( TrackByRateFlag)
						TrackByRate();
					else
						MoveToCurrentRaDec();
				}
				else
					if( FRMotorOn && FRStepSize > 0 && HandpadFRMotorControlOnFlag &&
					(Buttons & CCWKey || Buttons & CWKey))
						/* if tracking, let MoveMs() handle the FR motor movement */
						if( TrackFlag)
							CheckMiscEvents();
						else
							/* keeps control until button released or keyboard hit so do not attach more
							processing such as CheckMiscEvents() */
							ProcessHPEventsFR();
					else
						if( (FocusMethod == FocusMethod_Pulse_1_14 || FocusMethod == FocusMethod_Pulse_16_17)
						&& HandpadFocusMotorControlOnFlag && (Buttons & CCWKey || Buttons & CWKey))
						{
							InitiatedType = HandpadInitiated;
							if( TrackFlag)
								CheckMiscEvents();
							else
								ProcessHPEventsFocusMethod_Pulse_Dir();
						}
						else
							/* else either microstep or halfstep move: keeps control until microstep or
							halfstep movement ends */
							ProcessHPEventsMoveMotors();
			}
			/* else no valid direction buttons so look at mode switch and follow its commands; many
			commands	and no command at all end with CheckMiscEvents(); */
			else
				ProcessHPEventsModeSwitch();
		}
}

void WriteAnalysisFile( void)
{
	struct Position Temp;

	Output = fopen( AnalysisFile, "a");
	if( Output == NULL)
		BadExit( strcat( "Could not open for append ", AnalysisFile));

	Temp = Current;
	/* save input equatorial, current altaz, and current sidereal time */
	Current.Ra = In.Ra;
	Current.Dec = In.Dec;
	FWritePosition( Output, &Current, No);
	fprintf( Output, " PMCarcmin %3.3f %3.3f\n", PMC.A*RadToArcmin, PMC.Z*RadToArcmin);
	fclose( Output);
	Current = Temp;
}

