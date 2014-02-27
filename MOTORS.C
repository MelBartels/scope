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
#include <alloc.h>
#include "header.h"

/* Steppers */

/* each of the 4 windings of a motor is controlled directly through the parallel port by Pulse Width
Modulation, thereby giving individual current/voltage control over each winding;

move in overvoltage, halfstep mode for fastest speed, highest torque motion:
	homebuilt stepper motor drive circuit with diode + zener suppression + 4X overdrive voltage with
	smooth ramping up to	10,000 hs/sec with minimal torque loading;

move in microstep mode for smooth, fine motion;

microstepping algorithm:
	for first full step (angle of 0 deg to 90 deg):
	start winding A with max voltage and winding B with 0 voltage;
	gradually ramp winding B voltage up to max voltage and ramp down winding A voltage to 0:
	electromagnetic force is felt by	the inverse square of the distance, so to position the rotor at
	distance 'a' from winding A and	distance 'b' from winding B, with full voltage for winding A,
	the voltage for winding B will be sqr( a/b);
	can skip microsteps for faster speed, and, switch to halfstepping for fastest speed;
	experiment shows that theoretical cos/sin voltage values need to be modified slightly to take
	into account timing idiosyncrasies of the PC and the finite on/off timing of the opto-isolators
	and drive transistors;

microstep accuracy:
	from stepper motor theory and manuals:
	common absolute accuracy is 5' or ~1/22 full step (1 full step = 1.8 degrees),
	shaft deflection directly proportional to torque loading when positioned inbetween windings;
	conclusion: if torque loading < 10%, 10 microsteps/fullstep OK,
	at minimum, stepper should be accurate at quarter steps;
	more steps mean smoother motion, but not necessarily more accurate motion;
	software can correct for periodic errors in microsteps over the sequence of the 4 windings;

Halfstepping:
Halfstep                                        Winding:
																Bit Sum
								A   B   C   D
1                       1   0   0   0           1
2                       1   1   0   0           3
3                       0   1   0   0           2
4                       0   1   1   0           6
5                       0   0   1   0           4
6                       0   0   1   1           12
7                       0   0   0   1           8
8                       1   0   0   1           9

Module algorithms:
	1. use microsteps for tracking & guiding, and halfsteps for slewing,
	2. use PWM (Pulse Width Modulation) to control motor winding voltages,
	3. keep CPU usage (recalculations, writing to screen, etc) to mimimum while microstepping to
	keep motor rotation smooth, minimum PC requirements are 286, with co-processor for conversion
	routines helping,
	4. use pre-calculated Byte arrays for outportb() values for faster speed,
	5. speed of microstepping set by:
		a. MaxIncrMsPerPWM, which says max increment of microsteps per PWM (so that you can skip
		microsteps in an effort to rotate faster), when increment of microsteps per PWM >=
		MsHsToggleIncrMsPerPWMPWM, then microstepping will utilize halfstepping, one halfstep per PWM
		(max halfstepping speed = # of PWMs per timer tick * timer ticks per second);
		when IncrMs < 1, a microstep will occur over multiple PWM cycles,
		b. # of PWM per bios clock tick (bios clock tick occurs 18.2x a second)	is approximated by
		PWMRepsTick; optionally, no matter the value of PWMRepsTick, microstepping PWM will stop when
		program installed timer interrupt sets a flag,
	6. microstep tracking timing synchronized by program installed timer interrupt routine; the last
	PWM in each bios clock tick will be truncated,
	7. for faster PCs, repeat each microstepping outportb() MsDelayX times and run through a pause
	loop MsPause times,
	8. microstepping routine starts by moving the first microstep immediately;
	9. start half stepping with a holding time to precisely align rotor,
	10. when finished with halfstepping or microstepping, end on fullstep alignment with braking time
	to dampen shaft oscillation,
	11.speed of halfstepping determined by var HsTimerFlag, where 1 means to hook IRQ 8 interrupt,
	and 0 means to disable IRQs and time halfsteps by a delay loop of (MaxDelay - MinDelay)*HsDelayX,
	12.IRQ 8 timer is set to mode 4 to time the steps; mode 4 outputs a single	pulse at the end of a
	countdown period; the counter does not start until it is loaded by software; the period range is
	about 1 us to 54 ms for count	values of 1 to 65536. 65536 (count in one hour) is the familiar
	18.2 times a second of the standard timer; thus the MinDelay and MaxDelay are specified in about
	1 us increments; this range should cover most stepper requirements; the 8253 timer chip uses IRQ
	0, which is remapped to IRQ+8 by 8259 pic, and it is this IRQ8 that is hooked (Timer 0 is
	connected to IRQ0 of the 8259 PIC which in turn calls IRQ 08, which in turn calls INT 1C, or, can
	calla user supplied interrupt routine);
	13. from http://www.qzx.com/pc-gpe/pit.txt
	The PIT chip runs at a freqency of 1234DDh (1193181) Hz, and normally the BIOS timer interrupt
	handler is called for every 10000h (65536) cycles of this clock. If clock reprogrammed, keep a
	running total of the number of clock ticks which have occurred. For every interrupt generated
	add the reprogrammed cycles per tick to the total. When total >=  10000h (65536) call the old
	interrupt timer routine that is normally called every timer tick;
	14. 6v Steppers start to stall at about 250 us with a 12 volt supply; the interrupt routine
	should not take longer to execute than the shortest stepper pulse; so far testing has shown that
	a 286/16Mhz can easily keep up even with a MinDelay of only 50 us; suggested by Dale Eason who
	provided sample code,
	15.interrupt timer speed set by HsRepsIx + MinDelay, where HsRepsIx at slowest speed = MaxDelay -
	MinDelay, and at max speed = 0,
	16.in parallel while halfstep slewing, control kept in MoveHs(), where aborts are checked for and
	displays updated,
	17.linearily ramp motor speed during halfstepping to protect worm and gear speed reducer and to
	give fastest slewing speed possible,
	18.for long, open loop slews, set steps to move to MAXLONG - 1 and bring motor to ramped stop by
	setting HsMoveOKFlag,
	19.reset pc time based on CMOS clock when finished with halfstepping,
	20.module keeps track of accumulated steps made,
	21.controls 2 motors called 'A' and 'Z',
	22.slewing moves both motors initially then finishes with the remaining motor's leftover steps,
	23.use parallel port to interface to motors since the parallel port has 8 bits of output;

	24.Notes on 5-phase steppers:
	An option is provided for driving 5-phase motors by using pins 16 and 17 for the fifth phases of
	the A and Z motors respectively. To use this option, set MotorWindings to 5 in CONFIG.DATA. Note
	that the choice of 4 or 5 phases has no effect on the parallel port outputs for the field
	rotation motor.
	InvertOutput works, but ReverseA/Z motor directions will fail.
	Phase5 values stored for Z motor same as A motor, that is, they are not phase shifted 5 windings.
		This is unlike the 4 windings strategy, where the Z motor values are phase shifted 4 windings.
		Phase5 shifting for 5 windings fails because it shifts past the end of the 8 bit byte values
		used to store the PWM values.
	Each call to Phase5 consequently takes the Z motor values and * 32 in order to shift on the fly.
	The following changes in design or usage apply in the 5-phase case:
	1. Driver circuitry must use H-bridge design.  Refer to "Jones on Stepping Motors" at
	http://www.cs.uiowa.edu/~jones/step/ or design data for the SGS Thompson L298 Dual Full Bridge
	Driver at http://www.st.com/stonline/books/pdf/docs/1773.pdf
	2. Interpretation of the 0's in the microstepping table above and	in variables such as HsOut
	changes from "off" to "grounded" (see Jones).
	3. The bit layouts of variables such as HsOut change to accomodate the fifth phase.
	4. WriteAZ not only sends data on phases 1 - 4 to PPortAddrOutByte, but also sends phase 5 data
	to	FGMotorPort.  Writes to FGMotorPort are done to prevent AZ writes from affecting FR data and
	vice-versa. */

void InitMotors( void)
{
	/* save original timer interrupt vector: both halfstep and microstep routines install new
	interrupt routines */
	ClockVect = getvect( Timer_Int);

	if( MotorControlMethod != MotorControl_PWM_PCB && MotorControlMethod != MotorControl_PulseDir)
		MotorControlMethod = MotorControl_PWM_PCB;

	if( DisplayOpeningMsgs)
		switch (MotorControlMethod)
		{
			case MotorControl_PWM_PCB:
				printf( "\nusing motor control method of pulse width modulation via PCB controller");
				break;
			case MotorControl_PulseDir:
				printf( "\nusing motor control method of pulse and direction control lines");
				break;
			default:
				BadExit( "\nunknown motor control method");
		}

	if( DisplayOpeningMsgs)
   	if( MotorWindings == 5)
			printf( "\nconfigured for 5 phase stepper motors, using parallel port pins 16, 17");

	switch( MotorControlMethod)
	{
		case MotorControl_PWM_PCB:
			MoveHs_f_ptr = MoveHs;
			DoOneHs_f_ptr = DoOneHs;
			Hold_f_ptr = HoldPWM;
			MoveMs_f_ptr = MoveMs;
			SteppersOff_f_ptr = SteppersOffPWM;
			AlignMs_f_ptr = AlignMs;
			DisplayMsValues_f_ptr = DisplayMsValuesPWM;
			HoldPWMIsOnFlag = No;
			StartPWMTickHandler();
			PWMTickHandlerInstalledFlag = True;
			break;
		case MotorControl_PulseDir:
			A_Pulse_State = Z_Pulse_State = Off;
			A_PowerOnState = Z_PowerOnState = Off;
			MoveHs_f_ptr = MoveHs;
			DoOneHs_f_ptr = DoOneHsPulseDir;
			Hold_f_ptr = HoldPulseDir;
			MoveMs_f_ptr = MoveMsPulseDir;
			SteppersOff_f_ptr = SteppersOffPulseDir;
			AlignMs_f_ptr = NULL_FUNCTION;
			DisplayMsValues_f_ptr = DisplayMsValuesPulseDir;
			StartPWMTickHandler();
			PWMTickHandlerInstalledFlag = True;
			MsAlignFlag = Yes;
			MaxIncrMsPerPWM = 1;
			MsHsToggleIncrMsPerPWM = 1;
			Ms = 2;
			UsePWMZFlag = No;
			MaxPWM = 1;
			for( Ix = 0; Ix < MaxMs; Ix++)
				PWM[Ix].A = PWM[Ix].Z = PWMZ[Ix].A = PWMZ[Ix].Z = 0;
			break;
	}

	if( KeepAlivePPortPin)
	{
		OutputKeepAlive_f_ptr = OutputKeepAlive;
		switch( KeepAlivePPortPin)
		{
			case 1:
				KeepAlivePPortValue = PPortPin1;
				KeepAlivePPortPinMask = PPortPin14 + PPortPin16 + PPortPin17;
				break;
			case 14:
				KeepAlivePPortValue = PPortPin14;
				KeepAlivePPortPinMask = PPortPin1 + PPortPin16 + PPortPin17;
				break;
			case 16:
				KeepAlivePPortValue = PPortPin16;
				KeepAlivePPortPinMask = PPortPin1 + PPortPin14 + PPortPin17;
				break;
			case 17:
				KeepAlivePPortValue = PPortPin17;
				KeepAlivePPortPinMask = PPortPin1 + PPortPin14 + PPortPin16;
				break;
			default:
				BadExit( "Bad KeepAlivePPortPin value: must be 1, 14, 16, or 17");
		}
	}
	else
	{
		OutputKeepAlive_f_ptr = NULL_FUNCTION;
	}

	if( AltFullStepSizeArcsec <= 0)
		BadExit( "AltFullStepSizeArcsec must be positive value");
	if( AzFullStepSizeArcsec <= 0)
		BadExit( "AzFullStepSizeArcsec must be positive value");
	if( MinDelay >= MaxDelay)
		BadExit( "MinDelay >= MaxDelay in Steppers module");
	if( HsDelayX < 1)
		HsDelayX = 1;
	if( HsRampX < 1)
		HsRampX = 1;
	if( MsDelayX < 1)
		MsDelayX = 1;
	if( Ms > MaxMs)
		BadExit( "Ms value too large");
	if( MotorWindings != 4 && MotorWindings !=5)
		BadExit( "MotorWindings must be 4 or 5");
	if( !MaxPWMFoundFlag)
	{
		MaxPWM = PWM[0].A;
		if( DisplayOpeningMsgs)
			printf( "\nno MaxPWM value found in %s, setting MaxPWM to PWM[0].A", ConfigFile);
	}
	for( Ix = 0; Ix < Ms; Ix++)
		if( PWM[Ix].A > MaxPWM)
			BadExit( "No PWM[]. value can exceed MaxPWM");

	/* setup PWMRepsTickArray */
	TotalPWMRepsTickArray = 0;
	for( Ix = 0; Ix < SizeofPWMRepsTickArray; Ix++)
	{
		PWMRepsTickArray[Ix] = PWMRepsTick;
		TotalPWMRepsTickArray += PWMRepsTick;
	}
	PWMRepsTickArrayIx = 0;
	AvgPWMRepsTick = PWMRepsTick;

	CalcVarsRelatingToMs();

	EnableMotor.A = EnableMotor.Z = Yes;

	StartMsPowerDownCount = MsPowerDownSec*18.2;
	MsPowerDownCount.A = MsPowerDownCount.Z = StartMsPowerDownCount;

	HsOverVoltageSet = False;
	TrackingViaHs = No;
	SlewState = SlewDone;
	AbortState = NoAbort;
	MsZeroSoundOn = No;
	MoveHsUnderway = No;

	AltLowLimit = AltLowLimitDeg*DegToRad;
	AltHighLimit = AltHighLimitDeg*DegToRad;
	if( AltLowLimit == AltHighLimit)
		AltLimitFlag = No;
	else
		AltLimitFlag = Yes;
	AzLowLimit = AzLowLimitDeg*DegToRad;
	AzHighLimit = AzHighLimitDeg*DegToRad;
	if( AzLowLimit == AzHighLimit)
		AzLimitFlag = No;
	else
		AzLimitFlag = Yes;

	CalcVarsRelatingToStepSizes();

	/* 'A' motor lower nibble, 'Z' motor upper nibble */
	if( InvertOutput)
		if( MotorWindings == 4)
		{
			MotorOff.A = 0x0F;
			MotorOff.Z = 0xF0;
		}
		else
		/* phase5 motors strategy uses same values for Z as for A, since 5 bits are needed, and
			shifting the Z bits results in the 2 high bits being dropped */
		{
			MotorOff.A = 0x1F;
			MotorOff.Z = 0x1F;
		}
	else
		MotorOff.Z = MotorOff.A = 0;

	SteppersOff_f_ptr();

	Dir.A = Dir.Z = CW;

	HsIx.A = HsIx.Z = 0;
	MsIx.A = MsIx.Z = 0;
	Steps.A = Steps.Z = 0;
	IncrHs.A = IncrHs.Z = 0;
	IncrMs.A = IncrMs.Z = 0;

	CreateHsArrays();
	InitHsArrays();

	CreateMsArrays();
	InitMsArrays();
}

void CalcVarsRelatingToMs( void)
{
	MsPerHs = Ms/2;
	MsInWindings = Ms * MotorWindings;
}

/* following functions need to be called if changing fullstep size:
	CalcVarsRelatingToStepSizes(); (calls CalcVarsRelatingToPWMRepsTick(); & InitMsTickVars())
	CalcBacklashVars();
	CalcHsMsgSteps();
	SetAccumMsToCurrentAltaz();
	WriteMsArcsecSec();

	following need to be called if changing PWMRepsTick:
	CalcVarsRelatingToPWMRepsTick();
	CalcSectoredFRDrive(); */

void CalcVarsRelatingToStepSizes( void)
{
	/* microstep size in arc seconds */
	MicroStepSizeArcsec.A = AltFullStepSizeArcsec / Ms;
	MicroStepSizeArcsec.Z = AzFullStepSizeArcsec / Ms;

	/* length of one microstep in Radians = MsRad. */
	MsRad.A = MicroStepSizeArcsec.A * ArcsecToRad;
	MsRad.Z = MicroStepSizeArcsec.Z * ArcsecToRad;

	/* length of one halfstep in Radians = HsRad. */
	HsRad.A = MsRad.A*MsPerHs;
	HsRad.Z = MsRad.Z*MsPerHs;

	CalcVarsRelatingToPWMRepsTick();

	/* set vars for microstepping velocity based on arc seconds per seconds */
	InitMsTickVars( MsArcsecSec);
}

void CalcVarsRelatingToPWMRepsTick( void)
{
	/* max Ms distance possible in one bios clock tick (microstepping will halfstep if necessary) */
	MaxMsDistanceRadTick.A = PWMRepsTick * MaxIncrMsPerPWM * MsRad.A;
	MaxMsDistanceRadTick.Z = PWMRepsTick * MaxIncrMsPerPWM * MsRad.Z;

	/* max Ms distance to track */
	MaxMsDistanceRadTrack.A = 9 * MaxMsDistanceRadTick.A;
	MaxMsDistanceRadTrack.Z = 9 * MaxMsDistanceRadTick.Z;

	/* set maximum microstepping speed */
	if( MicroStepSizeArcsec.A < MicroStepSizeArcsec.Z)
		MaxMsSpeed = PWMRepsTick * MaxIncrMsPerPWM * ClockTicksSec * MicroStepSizeArcsec.A;
	else
		MaxMsSpeed = PWMRepsTick * MaxIncrMsPerPWM * ClockTicksSec * MicroStepSizeArcsec.Z;
}

void StartPWMTickHandler( void)
{
	/* set new interrupt handler; old is restored at program close in StopPWMTickHandler();
	this handler turns off the steppers and signals to MoveMs() to immediately end the PWMs;
	the user choosen optional halfstep timer interrupt upon concluding sets the timer interrupt back
	to the microstep interrupt handler */
	disable();
	setvect( Timer_Int, PWMTickHandler);
	enable();
	PWMTickHandlerInstalledFlag = True;
	PWMUnderwayFlag = No;
	PWMTickHandlerCalledFlag = False;
}

void StopPWMTickHandler( void)
{
	disable();
	/* restore interrupt vector set in InitMotors() */
	setvect( Timer_Int, ClockVect);
	enable();
	PWMTickHandlerInstalledFlag = False;
	PWMUnderwayFlag = No;
	PWMTickHandlerCalledFlag = False;
}

void CreateHsArrays( void)
{
	/* 4 phase motors: MaxHsIx = 7 */
	MaxHsIx = MotorWindings * 2 - 1;
	HsOut = (struct AZByte far*) farmalloc( (MaxHsIx + 1) * sizeof( struct AZByte));
	if( HsOut == NULL)
		BadExit( "Problem with malloc of HsOut in steppers module");

	/* set MaxHsRepsIx */
	MaxHsRepsIx = MaxDelay - MinDelay;
	/* build HsReps array */
	HsReps = (long far*) farmalloc( (MaxHsRepsIx + 1) * sizeof( long));
	if( HsReps == NULL)
		BadExit( "Problem with malloc of HsReps in steppers module");
}

void InitHsArrays( void)
{
	int IxA;
	Byte B;
	long MinMaxRatio;
	double DblIx, Sshape, Ramp, Ramp2;

	/* fill 'A' motor array with halfstep values */
	/* bit layout of HsOut:
	bit				0		1		2		3		4		5 		6		7		8		9
	bit value		1		2		4		8		16		32		64		128	256	512
						---	---	---	---	---	---	---	---	---	---
	4-phase case:
	A motor phase	1		2		3		4		0		0		0		0		0		0
	Z motor phase	0		0		0		0		1		2		3		4		0		0
	5-phase case:
	A motor phase	1		2		3		4		5		0		0		0 		0		0
	Z motor phase	0		0		0		0		0		1		2		3		4		5 */

	HsOut[0].A = 1;
	HsOut[1].A = 3;
	HsOut[2].A = 2;
	HsOut[3].A = 6;
	HsOut[4].A = 4;
	HsOut[5].A = 12;
	HsOut[6].A = 8;
	HsOut[7].A = 9;

	if( MotorWindings == 5)
	{
		HsOut[7].A = 24;
		HsOut[8].A = 16;
		HsOut[9].A = 17;
	}

	/* fill 'Z' motor values */
	for( IxA = 0; IxA <= MaxHsIx; IxA++)
		if( MotorWindings == 4)
			HsOut[IxA].Z = HsOut[IxA].A << MotorWindings;
		else
			HsOut[IxA].Z = HsOut[IxA].A;

	/* swap leads 1 and 3 (bits 1 and 4) to reverse motor direction */
	if( ReverseAMotor && MotorWindings == 4)
		for( IxA = 0; IxA <= MaxHsIx; IxA++)
		{
			B = HsOut[IxA].A & 0x5;
			HsOut[IxA].A &= 0xA;
			if( B == 0x1)
				B = 0x4;
			else
				if( B == 0x4)
					B = 0x1;
			HsOut[IxA].A += B;
		}
	if( ReverseZMotor && MotorWindings == 4)
		for( IxA = 0; IxA <= MaxHsIx; IxA++)
		{
			B = HsOut[IxA].Z & 0x50;
			HsOut[IxA].Z &= 0xA0;
			if( B == 0x10)
				B = 0x40;
			else
				if( B == 0x40)
					B = 0x10;
			HsOut[IxA].Z += B;
		}

	/* invert array */
	if( InvertOutput)
		for( IxA = 0; IxA <= MaxHsIx; IxA++)
		{
			HsOut[IxA].A = MotorOff.A - HsOut[IxA].A;
			HsOut[IxA].Z = MotorOff.Z - HsOut[IxA].Z;
		}

	/* since MoveHs() rounds up to nearest full step, make it max - 1 */
	MaxHs = MAXLONG - 1;
	/* HsReps[] contains # of halfsteps per speed or Delay setting: to allow for slews of maximum
	length, set HsReps[0] to highest possible value */
	HsReps[0] = MaxHs;
	MinMaxRatio = (long) MaxDelay / (long) (MinDelay);
	for( HsRepsIx = 1; HsRepsIx <= MaxHsRepsIx; HsRepsIx++)
	{
		if( HsRampStyle == 0)
		/* simple ramp */
			HsReps[HsRepsIx] = (long) HsRampX * (long) MaxDelay / (long) (HsRepsIx + MinDelay);
		else
		{
			/* S ramp: make a ramp ranging between MinMaxRatio and 1:
			code it as 1 + (a ramp between (MinMaxRatio-1) and 0);
			ramp is 'S' shaped curve, derived from sin() function, multiplied by the Ramp so that the
			faster time scale at the conclusion of the ramp is taken into account; Ramp2 further slows
			down the final quarter of the ramp */
			DblIx = (double)(MaxHsRepsIx-HsRepsIx)/MaxHsRepsIx;
			Sshape = fabs( sin( (DblIx-.5)*M_PI));
			Ramp = DblIx;
			Ramp2 = 1 + (DblIx - 3/4) * 6;
			if( Ramp2 < 1)
				Ramp2 = 1;
			HsReps[HsRepsIx] = (long) HsRampX * (1.5 + (MinMaxRatio-1) * Sshape * Ramp * Ramp2);
		}
	}
	/* Output = fopen( "HSREPS.TXT", "w");
	if( Output == NULL)
		BadExit( "Could not open HSREPS.TXT ");
	for( HsRepsIx = 1; HsRepsIx <= MaxHsRepsIx; HsRepsIx++)
		fprintf( Output, "%ld\n", HsReps[HsRepsIx]);
	fclose( Output); */
}

void FreeHsArrays( void)
{
	free( HsReps);
	free( HsOut);
}

void CreateMsArrays( void)
{
	HsIxToMsIxConvFactor = MaxPWM*MsPerHs;
	/* set MaxMsIx;
	# of microstep waveforms = # of microsteps (Ms) * # of windings (4);
	# of elements in microstep array = # of microstep waveforms * # of pulse width modulations per
	microstep voltage (MaxPWM);
	if MsIx is index to MsPWM, then microstep # (from 0 to Ms-1) = (MsIx/MaxPWM)%Ms,
	and winding # (from 0 to MotorWinding-1) = (MsIx/MaxPWM)/Ms */
	MaxMsIx = Ms*MotorWindings*MaxPWM - 1;
	MsMotorOffIx = MaxMsIx + 1;

	/* build microstep arrays, tagging onto the end room for a microstep of motor off commands */
	MsPWM = (struct AZByte far*) farmalloc( (MaxMsIx + 1 + MaxPWM) * sizeof( struct AZByte));
	if( MsPWM == NULL)
		BadExit( "Problem with malloc of MsPWM in steppers module ");
}

/* cleanup PWM[]: no negative values and no PWM[] greater than MaxPWM */
void CleanupPWM( struct AZInt* PWM)
{
	int IxA;

	for( IxA = 0; IxA < MaxMs; IxA++)
	{
		if( PWM[IxA].A < 0)
			PWM[IxA].A = 0;
		else
			if( PWM[IxA].A > MaxPWM)
				PWM[IxA].A = MaxPWM;
		if( PWM[IxA].Z < 0)
			PWM[IxA].Z = 0;
		else
			if( PWM[IxA].Z > MaxPWM)
				PWM[IxA].Z = MaxPWM;
	}
}

void InitMsArrays( void)
{
	int Winding;
	int MicroStep;
	int WindingA, WindingB;
	int IxA, IxB;
	Byte B;
	int loopc;

	/* fill in PWM[] and PWMZ[] arrays:
	PWM[].A is A winding of A motor, PWM[].Z is B winding of A motor,
	PWMZ[].A is A winding of Z	motor, PWMZ[].Z is B winding of Z motor*/

	/* if instead of PWM[] A, config file using PWM[] A : Z values, then ReadConfig() loads A values
	into PWM[].Z, else generate A values from PWM[Ms-MicroStep] */
	if( !UseComplexPWMFlag)
	{
		/* PWM[0] has no contribution from the complimentary winding */
		PWM[0].Z = 0;
		/* the winding B microstep that corresponds to the winding A microstep of Ms is
		Ms - MicroStep, ie, if the total # of microsteps = 10
		microstep     winding A Winding B
		0             PWM[0]  n/a
		1             PWM[1]  PWM[9]
		2             PWM[2]  PWM[8]
		3             PWM[3]  PWM[7]
		4             PWM[4]  PWM[6]
		5             PWM[5]  PWM[5]
		6             PWM[6]  PWM[4]
		7             PWM[7]  PWM[3]
		8             PWM[8]  PWM[2]
		9             PWM[9]  PWM[1] */
		for( Ix = 1; Ix < Ms; Ix++)
			PWM[Ix].Z = PWM[Ms-Ix].A;
	}

	/* if config file doesn't have explicit PWMZ[] values for Z motor, then copy A motor values;
	if complex PWMZ values, then UsePWMZFlag will be True */
	if( UsePWMZFlag)
		if( UseComplexPWMZFlag)
			;
		else
		{
			PWMZ[0].Z = 0;
			for( Ix = 1; Ix < Ms; Ix++)
				PWMZ[Ix].Z = PWMZ[Ms-Ix].A;
		}
	else
		for( Ix = 0; Ix < Ms; Ix++)
		{
			PWMZ[Ix].A = PWM[Ix].A;
			PWMZ[Ix].Z = PWM[Ix].Z;
		}

	CleanupPWM( PWM);
	CleanupPWM( PWMZ);

	/* zero out arrays */
	for( IxA = 0; IxA <= MaxMsIx; IxA++)
		MsPWM[IxA].Z = MsPWM[IxA].A = 0;

	/* start winding A at bit #1 and winding B at bit #2, winding A will start at the highest
	microstep voltage and ramp	down to zero volts while winding B will start at zero volts and ramp
	up */
	WindingA = 1;
	WindingB = 2;

	/* start base index of microstep array at the beginning,	index is incremented by MaxPWM (# of
	pulse width modulations per microstep voltage) */
	IxA = 0;

	/* microstep array has MotorWindings number of windings to build */
	for( Winding = 1; Winding <= MotorWindings; Winding++)
	{
		/* each winding has Ms number of microsteps to build */
		for( MicroStep = 0; MicroStep < Ms; MicroStep++)
		{
			/* 'A' motor winding A: turn it on for PWM[MicroStep] # of times, multiplied by a
			compensating factor */
			loopc = PWM[MicroStep].A * PWM_A_Comp[Winding-1]/1.;
			for( IxB = 0; IxB < loopc; IxB++)
				MsPWM[IxA+IxB].A += WindingA;

			/* 'Z' motor winding A: right shift the values MotorWindings # of bits so that the Z
			motor bits occupy the next grouping of parallel port output pins after the A motor bits */
			loopc = PWMZ[MicroStep].A * PWM_Z_Comp[Winding-1]/1.;
			for( IxB = 0; IxB < loopc; IxB++)
				if( MotorWindings == 4)
					MsPWM[IxA+IxB].Z += WindingA << 4;
				else
					MsPWM[IxA+IxB].Z += WindingA;

			/* 'A' motor winding B: */
			loopc = PWM[MicroStep].Z * PWM_A_Comp[Winding-1]/1.;
			for( IxB = 0; IxB < loopc; IxB++)
				MsPWM[IxA+IxB].A += WindingB;

			/* 'Z' motor winding B: */
			loopc = PWMZ[MicroStep].Z * PWM_Z_Comp[Winding-1]/1.;
			for( IxB = 0; IxB < loopc; IxB++)
				if( MotorWindings == 4)
					MsPWM[IxA+IxB].Z += WindingB << 4;
				else
					MsPWM[IxA+IxB].Z += WindingB;

			/* increment to start of next microstep voltage */
			IxA += MaxPWM;
		}
		/* goto next windings */
		WindingA *= 2;
		WindingB *= 2;
		/* max winding bit # for 4 windings = 8, for 5 windings = 16:
		1<<3 = 8 and 1<<4 = 16 (1 bit shifted to the left 4 times */
		if( WindingB > (1<<(MotorWindings-1)))
			WindingB = 1;
	}

	/* swap leads 1 and 3 (bits 1 and 4) to reverse motor direction */
	if( ReverseAMotor && MotorWindings == 4)
		for( IxA = 0; IxA <= MaxMsIx; IxA++)
		{
			B = MsPWM[IxA].A & 0x5;
			MsPWM[IxA].A &= 0xA;
			if( B == 0x1)
				B = 0x4;
			else
				if( B == 0x4)
					B = 0x1;
			MsPWM[IxA].A += B;
		}
	if( ReverseZMotor && MotorWindings == 4)
		for( IxA = 0; IxA <= MaxMsIx; IxA++)
		{
			B = MsPWM[IxA].Z & 0x50;
			MsPWM[IxA].Z &= 0xA0;
			if( B == 0x10)
				B = 0x40;
			else
				if( B == 0x40)
					B = 0x10;
			MsPWM[IxA].Z += B;
		}

	if( InvertOutput)
		for( IxA = 0; IxA <= MaxMsIx; IxA++)
		{
			MsPWM[IxA].A = MotorOff.A - MsPWM[IxA].A;
			MsPWM[IxA].Z = MotorOff.Z - MsPWM[IxA].Z;
		}

	/* do the filler microstep containing motor off bytes */
	for( IxA = 0; IxA < MaxPWM; IxA++)
	{
		MsPWM[MsMotorOffIx + IxA].A = MotorOff.A;
		MsPWM[MsMotorOffIx + IxA].Z = MotorOff.Z;
	}

	// WritePWMValues();
}

void WritePWMValues( void)
{
	int IxA;

	Output = fopen( "pwm.txt", "w");
	if( Output == NULL)
		BadExit( "Could not open pwm.txt");
	for( IxA = 0; IxA <= MaxMsIx + MaxPWM; IxA++)
	{
		if( !(IxA % MaxPWM))
			fprintf( Output, "\n%4d: ", IxA);
		if( InvertOutput)
			fprintf( Output, "%4d/%4d ", MotorOff.A - MsPWM[IxA].A, MotorOff.Z - MsPWM[IxA].Z);
		else
			fprintf( Output, "%4d/%4d ", MsPWM[IxA].A, MsPWM[IxA].Z);
	}
	fclose( Output);
	printf( "\n\nBuilt new pwm.txt file, containing microstepping values.\n\n");
	_exit( 0);
}

void FreeMsArrays( void)
{
	farfree( MsPWM);
}

void CloseSteppers( void)
{
	FreeHsArrays();
	FreeMsArrays();
	if( PWMTickHandlerInstalledFlag)
		StopPWMTickHandler();
	if( SteppersOff_f_ptr)
		SteppersOff_f_ptr();
}

/* change MsTick.A and MsTick.Z based on MsArcsecSec */
void InitMsTickVars( const int MsArcsecSec)
{
	MsTick.A = MsArcsecSec / (MicroStepSizeArcsec.A * ClockTicksSec);
	MsTick.Z = MsArcsecSec / (MicroStepSizeArcsec.Z * ClockTicksSec);
}

void WritePhase5Data( int AZData)
{
	int Phase5Data, Phase1To4Data;

	/* shift phase 5 alt and az into proper bit pattern for parallel port:
	bit '16' is phase 5 for alt, and should end up in bit '4';
	bit '512' is phase 5 for az, and should end up in bit '8';
	BiDirOutNibble() takes care of inverting pins */
	Phase5Data = ((AZData & 16) >> 2) + ((AZData & 512) >> 6);
	/* Write phase 5 data, protecting field rotation data from being overwritten */
	GetUnusedPPortLines();
	BiDirOutNibbleValue = UnusedFRPPortLines + Phase5Data;
	BiDirOutNibble();
	/* write just the 4 phase pattern for alt and shift 4 phase pattern for az down 1
	(32+64+128+256=480) */
	Phase1To4Data = ( AZData & 15) + (( AZData & 480) >> 1);
	outportb( PPortAddrOutByte, Phase1To4Data);
}

void SteppersOffPWM( void)
{
	if( MotorWindings == 5)
		WritePhase5Data( MotorOff.A + MotorOff.Z * 32);
	else
		outportb( PPortAddrOutByte, MotorOff.A + MotorOff.Z);
}

void SteppersOffPulseDir( void)
{
	Byte Out = 0;

	A_PowerOnState = Off;
	if( (!ReverseAMotor && Dir.A == CW) || (ReverseAMotor && Dir.A == CCW))
		Out += A_Dir;

	Z_PowerOnState = Off;
	if( (!ReverseZMotor && Dir.Z == CW) || (ReverseZMotor && Dir.Z == CCW))
		Out += Z_Dir;

	if( InvertOutput)
		Out = ~Out;
	outportb( PPortAddrOutByte, Out);
}

/* holds motors position by energizing windings for HoldReps * HsDelayX * MaxDelay repetitions of
port call: use to align motors and brake motors;
Hold_f_ptr, which is set to HoldPWM() for PWM stepper control, is called at start of and end of MoveHs(),
which is where HoldMotor.A,Z are set */
void HoldPWM( void)
{
	struct AZByte Out;
	int RepCount, HsDelayXCount, DelayCount;

	/* to prevent PWMTickHandler() interrupt routine from turning off steppers */
	HoldPWMIsOnFlag = Yes;
	if( HoldMotor.A)
		Out.A = HsOut[HsIx.A].A;
	else
		Out.A = MotorOff.A;
	if( HoldMotor.Z)
		Out.Z = HsOut[HsIx.Z].Z;
	else
		Out.Z = MotorOff.Z;
	for( RepCount = 0; RepCount < HoldReps; RepCount++)
		for( HsDelayXCount = 0; HsDelayXCount < HsDelayX; HsDelayXCount++)
			for( DelayCount = 0; DelayCount < MaxDelay; DelayCount++)
				if( MotorWindings == 5)
					WritePhase5Data( Out.A + Out.Z * 32);
				else
					outportb( PPortAddrOutByte, Out.A + Out.Z);
	HoldPWMIsOnFlag = No;
}

void HoldPulseDir( void)
{
	int RepCount, HsDelayXCount, DelayCount;

	for( RepCount = 0; RepCount < HoldReps; RepCount++)
		for( HsDelayXCount = 0; HsDelayXCount < HsDelayX; HsDelayXCount++)
			for( DelayCount = 0; DelayCount < MaxDelay; DelayCount++)
				SteppersOffPulseDir();
}

/* pauses until next clock tick */
void PauseUntilNewSidTime( void)
{
	NewSidT();
	while( !NewSidT())
		;
}

void OutputKeepAlive( void)
{
	static Flag PinState;

	BiDirInNibbleValue = BiDirInNibble();
	if( PinState)
		BiDirOutNibbleValue = (BiDirInNibbleValue & KeepAlivePPortPinMask);
	else
		BiDirOutNibbleValue = (BiDirInNibbleValue & KeepAlivePPortPinMask) + KeepAlivePPortValue;
	BiDirOutNibble();
	PinState = !PinState;
}

void SetBacklashDirA( int Dir)
{
	int Ix;

	for( Ix = 0; Ix < LastBacklashDirSize; Ix++)
		LastBacklashDir[Ix].A = Dir;
}

void SetBacklashDirZ( int Dir)
{
	int Ix;

	for( Ix = 0; Ix < LastBacklashDirSize; Ix++)
		LastBacklashDir[Ix].Z = Dir;
}

/* as motor moves CW, ActualBacklash is reduced to 0, as motor moves CCW, ActualBacklash is increased
to Backlash */
void AddBacklashA( long Ms)
{
	if( Dir.A == CCW)
		Ms = -Ms;
	ActualBacklashMs.A -= Ms;
	if( ActualBacklashMs.A < 0)
		ActualBacklashMs.A = 0;
	if( ActualBacklashMs.A > BacklashMs.A)
		ActualBacklashMs.A = BacklashMs.A;
}

void AddBacklashZ( long Ms)
{
	if( Dir.Z == CCW)
		Ms = -Ms;
	ActualBacklashMs.Z -= Ms;
	if( ActualBacklashMs.Z < 0)
		ActualBacklashMs.Z = 0;
	if( ActualBacklashMs.Z > BacklashMs.Z)
		ActualBacklashMs.Z = BacklashMs.Z;
}

void CalcActualBacklashRad( void)
{
	ActualBacklashRad.A = ActualBacklashMs.A * MsRad.A;
	ActualBacklashRad.Z = ActualBacklashMs.Z * MsRad.Z;
}

/* fullstep broken into two halves: one to raise the pulse line high, the other to lower the line */
void DoOneHsPulseDir( void)
{
	Byte Out = 0;

	A_PowerOnState = Z_PowerOnState = Off;

	if( (!ReverseAMotor && Dir.A == CW) || (ReverseAMotor && Dir.A == CCW))
		Out += A_Dir;
	if( (!ReverseZMotor && Dir.Z == CW) || (ReverseZMotor && Dir.Z == CCW))
		Out += Z_Dir;

	if( Steps.A)
	{
		Steps.A--;
		if( A_Pulse_State)
			A_Pulse_State = Off;
		else
		{
			A_Pulse_State = On;
			Out += A_Pulse;
		}
		if( Dir.A == CW)
			AccumMs.A++;
		else
			AccumMs.A--;
		AddBacklashA( 1);
		A_PowerOnState = A_Power_On;
		Out += A_PowerOnState;
	}
	if( Steps.Z)
	{
		Steps.Z--;
		if( Z_Pulse_State)
			Z_Pulse_State = Off;
		else
		{
			Z_Pulse_State = On;
			Out += Z_Pulse;
		}
		if( Dir.Z == CW)
			AccumMs.Z++;
		else
			AccumMs.Z--;
		AddBacklashZ( 1);
		Z_PowerOnState = Z_Power_On;
		Out += Z_PowerOnState;
	}

	if( InvertOutput)
		Out = ~Out;
	outportb( PPortAddrOutByte, Out);

	OutputKeepAlive_f_ptr();
}

void DoOneHs( void)
{
	struct AZByte Out;

	if( Steps.A)
	{
		if( Dir.A == CW)
		{
			if( ++HsIx.A > MaxHsIx)
				HsIx.A = 0;
			AccumMs.A += MsPerHs;
		}
		else
		{
			if( --HsIx.A < 0)
				HsIx.A = MaxHsIx;
			AccumMs.A -= MsPerHs;
		}
		Steps.A--;
		AddBacklashA( MsPerHs);
		Out.A = HsOut[HsIx.A].A;
	}
	else
		Out.A = MotorOff.A;
	MsIx.A = HsIx.A * HsIxToMsIxConvFactor;

	if( Steps.Z)
	{
		if( Dir.Z == CW)
		{
			if( ++HsIx.Z > MaxHsIx)
				HsIx.Z = 0;
			AccumMs.Z += MsPerHs;
		}
		else
		{
			if( --HsIx.Z < 0)
				HsIx.Z = MaxHsIx;
			AccumMs.Z -= MsPerHs;
		}
		Steps.Z--;
		AddBacklashZ( MsPerHs);
		Out.Z = HsOut[HsIx.Z].Z;
	}
	else
		Out.Z = MotorOff.Z;
	MsIx.Z = HsIx.Z * HsIxToMsIxConvFactor;

	if( MotorWindings == 5)
		WritePhase5Data( Out.A + Out.Z * 32);
	else
		outportb( PPortAddrOutByte, Out.A + Out.Z);

	OutputKeepAlive_f_ptr();
}

void DoHsDelayLoop( void)
{
	int HsDelayXCount, DelayCount;

	/* repetitions of delay counts */
	for( HsDelayXCount = 0; HsDelayXCount < HsDelayX; HsDelayXCount++)
		for( DelayCount = 0; DelayCount < HsRepsIx + MinDelay; DelayCount++)
			;
}

void SetTickDelay( void)
{
	if( AbortState)
	{
		SlewState = RampDown;
		if( !HsRepsIx)
			/* immediate ramp down from max slew speed */
			SameSpeedHs = HsReps[HsRepsIx];
	}
	switch( SlewState)
	{
		case RampUp:
			if( --RampHs == 0)
				SlewState = RampDown;
			if( --SameSpeedHs == 0)
				SameSpeedHs = HsReps[--HsRepsIx];
			if( HsRepsIx == 0)
				SlewState = MaxSlewUp;
			RampUpSteps++;
			break;
		case MaxSlewUp:
			if( --RampHs)
				 --SameSpeedHs;
			else if( HsRepsIx == 0)
				SlewState = MaxSlewDown;
			else
				SlewState = RampDown;
			MaxSlewUpSteps++;
			break;
		case MaxSlewDown:
			if( SameSpeedHs++ == HsReps[0])
			{
				HsRepsIx = 1;
				SameSpeedHs = 0;
				SlewState = RampDown;
			}
			MaxSlewDownSteps++;
			break;
		case RampDown:
			if( SameSpeedHs++ == HsReps[HsRepsIx])
				if( ++HsRepsIx > MaxHsRepsIx)
				{
					SlewState = SlewDone;
					break;
				}
				else
					SameSpeedHs = 1;
			RampDownSteps++;
	}
	Timer = HsRepsIx + MinDelay;
	if( Timer == 0)
		Timer = 1;

	/* timer_count can be used to calculate the time elapsed:
	elapsed time in seconds = (double)timer_count/TIMER_FREQ */
	timer_count += Timer;

	/* set 8253 chip: Timer 0, binary counter, rate generator (mode 2); load low byte, then high byte
	(for more, see notes above) */
	outportb( Timer_Control, 0x38);
	/* low byte */
	outportb( Timer_0, 0xFF & Timer);
	/* high byte */
	outportb( Timer_0, 0xFF & (Timer >> 8));
	/* do not do any floating point code from this point, as it locks up Toshiba laptop 486SX */

	if( Timer < HsOverVoltageControl)
	{
		if( !HsOverVoltageSet)
		{
			SetPPortLine17High();
			HsOverVoltageSet = True;
		}
	}
	else
		if( HsOverVoltageSet)
		{
			SetPPortLine17Low();
			HsOverVoltageSet = False;
		}
}

void interrupt SlewTickHandler( void)
{
	if( SlewState != SlewDone)
	{
		DoOneHs_f_ptr();
		SetTickDelay();
	}
	/* enable PIC EOI (end of interrupt) */
	outportb( PIC_EOI_ADDR, EOI);
}

void RoundNearestFullstep( void)
{
	/* end on fullstep */
	if( Steps.A%2)
		Steps.A--;
	if( Steps.Z%2)
		Steps.Z--;
	/* steps can never be < 0 */
	if( Steps.A < 0)
		Steps.A = 0;
	if( Steps.Z < 0)
		Steps.Z = 0;
}

void SetPPortLine17High( void)
{
	BiDirInNibbleValue = BiDirInNibble();
	HsOverVoltageUnusedBiDirPPortLines = BiDirInNibbleValue & PPortPins1_14_16;
	BiDirOutNibbleValue = PPortPin17 + HsOverVoltageUnusedBiDirPPortLines;
	BiDirOutNibble();
}

void SetPPortLine17Low( void)
{
	BiDirInNibbleValue = BiDirInNibble();
	HsOverVoltageUnusedBiDirPPortLines = BiDirInNibbleValue & PPortPins1_14_16;
	BiDirOutNibbleValue = HsOverVoltageUnusedBiDirPPortLines;
	BiDirOutNibble();
}

/* MoveHs_f_ptr is set to MoveHs() or other functions in InitMotors(); MoveHs_f_ptr is called by:
	1.HPEventMoveHs() which is called by:
		a.KBEventMoveHs() which is called by:
			1.SetDirDistanceStepsThenMove() which is called by:
				a.MoveToCurrentRaDec() which is called by:
					1.Move_Update_Handpad_Subr() which is called by:
						a.ProcessScroll()
					2.ProcessLX200_Motor_Cmd()
					3.CheckMiscEvents()
					4.ProcessHPEvents()
				b.ProcessMenuMoveAltaz()
				c.ProcessMenuMoveHome()
				d.ProcessScroll()
			2.ProcessScroll()
			3.ProcessLX200_Motor_Cmd()
			4.ProcessHPEventsMoveMotors() which is called by:
				a.Move_Update_Handpad_Subr()
				b.ProcessHandpadEvents()
			5.ProcessMenuMoveHs()
			6.ProcessMenuMoveGEMFlip()
			7.ProcessMenuMoveZeroPEC()
			8.ProcessMouseModeHsMs()
*/

/* keeps control until motors come to a stop after ramping down; moves to nearest full step */
void MoveHs( void)
{
	MoveHsUnderway = Yes;
	CheckLX200Events();
	InitHandpad = Handpad;
	HandpadOKFlag = Yes;
	AbortState = No;
	/* for delay loop timing of halfsteps, all interrupts disabled and encoders disabled;
	for interrupt timer timing of halfsteps, both ports remain enabled but encoders will not be
	queried */
	DisplayEncodersNotAvailable();
	if( HsTimerFlag)
	{
		/* setup for time keeping during slew */
		timer_count = 0;
		ElapsedHSSec = 0;
		NewSidT();
		StartSidTHS = Current.SidT;
		ETask = HTask = LTask = 0;
	}
	else
	{
		CloseSerial( EncoderComPort);
		CloseSerial( LX200ComPort);
	}

	while( !AbortState && (Steps.A || Steps.Z))
	{
		RoundNearestFullstep();

		/*
			HsRec[HsRecIx].A = Steps.A;
			HsRec[HsRecIx].Z = Steps.Z;
			HsRecIx++;
			if( HsRecIx >= HsRecSize)
				HsRecIx = 0;
		*/

		/* hold motor(s) position for brief time to align */
		if( Steps.A)
			HoldMotor.A = On;
		else
			HoldMotor.A = Off;
		if( Steps.Z)
			HoldMotor.Z = On;
		else
			HoldMotor.Z = Off;
		Hold_f_ptr();

		/* do smaller # of steps 1st, unless a motor has 0 steps to do: flags set for Hold() based from
		total Steps.A,Z depend on this mvmt algorithm to lock the appropriate motor */
		if( Steps.A == 0)
			MvmtHs = Steps.Z;
		else if( Steps.Z == 0)
			MvmtHs = Steps.A;
		else if( Steps.A < Steps.Z)
			MvmtHs = Steps.A;
		else
			MvmtHs = Steps.Z;
		if( MvmtHs%2)
			BadExit( "Bad MvmtHs - odd value - in MoveHs()");

		RampHs = MvmtHs/2;
		if( RampHs < 0)
			BadExit( "Bad RampHs - negative value - in MoveHs()");

		if( HsTimerFlag)
			CheckLX200Events();
		if( HsTimerFlag)
			MoveHsUsingIRQTimer();
		else
		{
			MoveHsUsingDelayLoop();
			if( !HandpadOKFlag)
				AbortState = HandpadAbort;
		}
		if( HsTimerFlag)
			CheckLX200Events();
		/* hold motor(s) position for brief time to brake */
		Hold_f_ptr();
		HoldMotor.A = HoldMotor.Z = Off;
	}
	if( HsTimerFlag)
		CheckLX200Events();
	SteppersOff_f_ptr();
	if( HsTimerFlag)
		CheckLX200Events();
	// for critical instant LX200 response, make sure that SetDOSToCMOS_RTC_f_ptr = SetDOSToCMOS_RTC_ViaBios
	SetDOSToCMOS_RTC_f_ptr();
	if( HsTimerFlag)
		CheckLX200Events();
	else
	{
		InitSerial( EncoderComPort, EncoderBaudRate, Parity, DataBits, StopBits);
		InitSerial( LX200ComPort, LX200BaudRate, Parity, DataBits, StopBits);
	}
	CalcActualBacklashRad();
	CheckLX200Events();
	PauseUntilNewSidTime();
	CheckLX200Events();
	MoveHsUnderway = No;
}

/* this function installs a new clock interrupt handler, SlewTickHandler(), then stays in a loop
that handles limits and calls SequentialTaskController() and checks for SlewDone, when SlewDone,
restores the original clock interrupt handler;
each time SlewTickHandler() is called via the clock interrupt, it does a single halfstep, then calls
SetTickDelay();
SetTickDelay(() sets the delay interval for calling the clock interrupt handler again, and also sets
the SlewState (when finished sets it to SlewDone) */

void MoveHsUsingIRQTimer( void)
{
	int IxL;

	/* initialize for SetTickDelay() */
	HsRepsIx = MaxHsRepsIx;
	SameSpeedHs = HsReps[HsRepsIx];
	SlewState = RampUp;
	RampUpSteps = MaxSlewUpSteps = MaxSlewDownSteps = RampDownSteps = 0;
	/* set new interrupt handler */
	disable();
	/* ClockVect = getvect( Timer_Int); set once in InitMotors() */
	setvect( Timer_Int, SlewTickHandler);
	/* enable() occurs after the while loop evaluation, to prevent the SlewState from being changed
	by the timer interrupt handler routine during the while loop evaluation, suggested by Egon Lenc */
	while( SlewState != SlewDone)
	{
		enable();

		if( !UseAutoPECSynch101213Flag)
		{
			/* allow slew to continue afterall if original handpad button is re-pressed: this avoids
			coming to a complete stop and ramping up again if the handpad commanded slew is deemed to fall
			short */
			if( SlewStartedFromHandpad && !HandpadOKFlag && RampHs)
			{
				ReadHandpad_f_ptr();
				if( Handpad == InitHandpad)
				{
					disable();
					HandpadOKFlag = Yes;
					AbortState = NoAbort;
					if( HsRepsIx == 0)
						SlewState = MaxSlewUp;
					else
						SlewState = RampUp;
					SameSpeedHs = HsReps[HsRepsIx];
					enable();
				}
			}

			SetHandpadOKFlag();
			if( !HandpadOKFlag)
			{
				disable();
				AbortState = HandpadAbort;
				enable();
			}
		}

		if( UseMouseFlag && SlewStartedFromMouse && MouseLeftButtonReleaseCount())
		{
			disable();
			AbortState = MouseAbort;
			enable();
		}
		if( KeyStroke)
		{
			disable();
			AbortState = KeyStrokeAbort;
			enable();
			getch();
		}

		if( AltLimitFlag || AzLimitFlag)
		{
			/* calculate distance for ramp down using current speed setting (HsReps[HsRepsIx]) */
			RampSteps = 0;
			IxL = HsRepsIx;
			if( IxL == 0)
				IxL = 1;
			while( IxL <= MaxHsRepsIx)
			{
				RampSteps += HsReps[IxL];
				IxL++;
			}
			if( Steps.A)
				RampRad.A = (double) RampSteps * HsRad.A;
			else
				RampRad.A = 0;
			if( Steps.Z)
				RampRad.Z = (double) RampSteps * HsRad.Z;
			else
				RampRad.Z = 0;
		}
		if( AltLimitFlag)
		{
			if( Current.Alt < (AltLowLimit+RampRad.A) && Dir.A == CCW)
			{
				disable();
				AbortState = AltLowLimitAbort;
				enable();
			}
			else if( Current.Alt > (AltHighLimit-RampRad.A) && Dir.A == CW)
			{
				disable();
				AbortState = AltHighLimitAbort;
				enable();
			}
		}
		if( AzLimitFlag)
		{
			if( Current.Az < (AzLowLimit+RampRad.Z) && Dir.Z == CCW)
			{
				disable();
				AbortState = AzLowLimitAbort;
				enable();
			}
			else if( Current.Az > (AzHighLimit-RampRad.Z) && Dir.Z == CW)
			{
				disable();
				AbortState = AzHighLimitAbort;
				enable();
			}
		}

		SequentialTaskController();
		CheckLX200Events();

		/* this disable() stays in effect through the while loop evaluation (but not the code in the
		while loop) */
		disable();
		if( SlewState == SlewDone)
		{
			/* return 8253 chip to normal operation of 65536u countdown, or about 18.2 Hertz, the
			standard bios clock tick frequency;
			the 8259 timer chip can be programmed to generate interrupts at a different rate;
			standard rate is ~18.2 x's per sec: Divisor > 1 will X the rate accordingly;
			from Tech Section,Computer Shopper,Oct,'92,807-
			Count = 65536/Divisor; Low = Count%256; High = Count/256;
			outportb( 0x43,0x36); outportb( 0x40,Low); outportb( 0x40,High); */
			outportb( Timer_Control, 0x36);
			outportb( Timer_0, 0);
			outportb( Timer_0, 0);
			/* restore interrupt vector */
			if( PWMTickHandlerInstalledFlag)
				setvect( Timer_Int, PWMTickHandler);
			else
				setvect( Timer_Int, ClockVect);
		}
	}
	enable();
}

void MoveHsUsingDelayLoop( void)
{
	if( HsOverVoltageControl)
	{
		SetPPortLine17High();
		HsOverVoltageSet = True;
	}
	/* set for max delay giving slowest speed (index is decremented in while loop before being used
	for 1st time) */
	HsRepsIx = MaxHsRepsIx + 1;
	/* if steps to move exceed InterruptHs, then disable interrupts for smoothest timing pulses to
	allow faster speeds */
	if( RampHs > InterruptHs)
	{
		InterruptFlag = Yes;
		disable();
	}
	/* ramp up */
	/* a goto label */
	RAMPUP:
	while( HandpadOKFlag && RampHs)
	{
		SameSpeedHs = HsReps[--HsRepsIx];
		while( HandpadOKFlag && RampHs && SameSpeedHs)
		{
			DoOneHs_f_ptr();
			DoHsDelayLoop();
			if( !UseAutoPECSynch101213Flag)
				SetHandpadOKFlag();
			RampHs--;
			SameSpeedHs--;
		}
	}
	/* ramp down */
	while( HsRepsIx <= MaxHsRepsIx)
	{
		while( SameSpeedHs < HsReps[HsRepsIx])
		{
			/* if move cancelled while moving at max speed, then immediate ramp down */
			if( !HandpadOKFlag && HsRepsIx == 0)
				SameSpeedHs = HsReps[HsRepsIx] - 1;
			DoOneHs_f_ptr();
			DoHsDelayLoop();

			/* allow slew to continue afterall if original handpad button is re-pressed: this avoids
			coming to a complete stop and ramping up again if the handpad commanded slew is deemed to
			fall short:
			/* SetHandpadOKFlag(); is replaced by */
			if( !UseAutoPECSynch101213Flag)
				if( SlewStartedFromHandpad && !HandpadOKFlag && RampHs)
				{
					ReadHandpad_f_ptr();
					if( Handpad == InitHandpad)
					{
						HandpadOKFlag = Yes;
						HsRepsIx++;
						goto RAMPUP;
					}
				}

			RampHs--;
			SameSpeedHs++;
		}
		SameSpeedHs = 0;
		HsRepsIx++;
	}
	if( InterruptFlag)
		enable();
	if( HsOverVoltageSet)
	{
		SetPPortLine17Low();
		HsOverVoltageSet = False;
	}
}

/* new timer interrupt routine: insures that motors are turned off at beginning of timer interrupt -
this prevents a motor 'on' spike during the time that the timer interrupt takes to process (this
time can be 1/2 millisecond in a 386/20 machine; also used to signal end of time period so that
pulse width modulations can be stopped immediately */

void interrupt PWMTickHandler( void)
{
	PWMTickHandlerCalledFlag = True;
	/* don't call SteppersOff() if control method is pulse/dir - this would incorrectly turn off the
	motor power control lines, and, don't call SteppersOff() if HoldPWM() is underway */
	if( PWMUnderwayFlag && (MotorControlMethod == MotorControl_PWM_PCB && !HoldPWMIsOnFlag))
		SteppersOff_f_ptr();

	OutputKeepAlive_f_ptr();

	ClockVect();
}

/* speed of microstepping set by IncrMs, if IncrMs left at zero, function sets IncrMs to spread
Steps over 1 bios clock tick */
void MoveMs( void)
{
	int IxA, IxB, IxC;
	struct AZInt HsMsIx;


	SetPulseFRPulseFocusInMoveMs();

	/* to signal new timer interrupt that pulse width modulations are underway */
	PWMUnderwayFlag = Yes;
	/* so as to detect call of new timer interrupt */
	PWMTickHandlerCalledFlag = No;
	MsAlignFlag = No;
	ActualPWMRepsTick = 0;
	StepsMoved.A = StepsMoved.Z = 0;
	/* steps can never be < 0 */
	if( Steps.A < 0)
		Steps.A = 0;
	if( Steps.Z < 0)
		Steps.Z = 0;

	MsPowerDown = 0;

	/* allow power down of the motor */
	if( EnableMotor.A)
		if( Steps.A==0)
			if( MsPowerDownCount.A)
				MsPowerDownCount.A--;
			else
				MsPowerDown = 1;
		else
			MsPowerDownCount.A = StartMsPowerDownCount;
	else
	{
		MsPowerDown = 1;
		MsPowerDownCount.A = 0;
		Steps.A = 0;
	}

	if( EnableMotor.Z)
		if( Steps.Z==0)
			if( MsPowerDownCount.Z)
				MsPowerDownCount.Z--;
			else
				MsPowerDown += 2;
		else
			MsPowerDownCount.Z = StartMsPowerDownCount;
	else
	{
		MsPowerDown += 2;
		MsPowerDownCount.Z = 0;
		Steps.Z = 0;
	}

	/* speed of microstepping set by IncrMs (# of microsteps per PWM):
	when < 1, multiple PWMs occur for a microstep, when > 1 (max = 5), microsteps are skipped over */
	if( IncrMs.A == 0 && IncrMs.Z == 0)
	{
		/* if PWMRepsTick from config.dat > ActualPWMRepsTick, then IncrMs. will be too small
		possibly resulting in steps not being moved */
		IncrMs.A = ((double) Steps.A) / (double) PWMRepsTick;
		IncrMs.Z = ((double) Steps.Z) / (double) PWMRepsTick;
	}

	/* check against user defined speed limit */
	if( IncrMs.A > MaxIncrMsPerPWM)
		IncrMs.A = MaxIncrMsPerPWM;
	if( IncrMs.Z > MaxIncrMsPerPWM)
		IncrMs.Z = MaxIncrMsPerPWM;

	/* max speed is halfstepping */
	if( IncrMs.A > MsPerHs)
		IncrMs.A = MsPerHs;
	/* switch to halfstepping when step size exceeds user defined threshold */
	if( IncrMs.A > MsHsToggleIncrMsPerPWM)
		GoByHs.A = True;
	else
		GoByHs.A = False;

	if( IncrMs.Z > MsPerHs)
		IncrMs.Z = MsPerHs;
	if( IncrMs.Z > MsHsToggleIncrMsPerPWM)
		GoByHs.Z = True;
	else
		GoByHs.Z = False;

	/* make first step happen immediately */
	if( Steps.A)
		MsRunningTotal.A = 1;
	else
		MsRunningTotal.A = 0;
	if( Steps.Z)
		MsRunningTotal.Z = 1;
	else
		MsRunningTotal.Z = 0;

	do
	{
		if( Steps.A)
		{
			/* MsRunningTotal is a double */
			MsRunningTotal.A += IncrMs.A;
			/* MsToDo is an int */
			MsToDo.A = MsRunningTotal.A;
			/* fractional part remains in MsRunningTotal */
			MsRunningTotal.A -= MsToDo.A;
			Steps.A -= MsToDo.A;
			StepsMoved.A += MsToDo.A;
			if( Dir.A == CW)
			{
				MsIx.A += MaxPWM*MsToDo.A;
				if( MsIx.A > MaxMsIx)
					MsIx.A -= MaxMsIx + 1;
				AccumMs.A += MsToDo.A;
			}
			else
			{
				MsIx.A -= MaxPWM*MsToDo.A;
				if( MsIx.A < 0)
					MsIx.A += MaxMsIx + 1;
				AccumMs.A -= MsToDo.A;
			}
			AddBacklashA( MsToDo.A);
		}
		if( Steps.Z)
		{
			MsRunningTotal.Z += IncrMs.Z;
			MsToDo.Z = MsRunningTotal.Z;
			MsRunningTotal.Z -= MsToDo.Z;
			Steps.Z -= MsToDo.Z;
			StepsMoved.Z += MsToDo.Z;
			if( Dir.Z == CW)
			{
				MsIx.Z += MaxPWM*MsToDo.Z;
				if( MsIx.Z > MaxMsIx)
					MsIx.Z -= MaxMsIx + 1;
				AccumMs.Z += MsToDo.Z;
			}
			else
			{
				MsIx.Z -= MaxPWM*MsToDo.Z;
				if( MsIx.Z < 0)
					MsIx.Z += MaxMsIx + 1;
				AccumMs.Z -= MsToDo.Z;
			}
			AddBacklashZ( MsToDo.Z);
		}

		/* if halfstepping, find microstep index that corresponds to nearest halfstep index */
		if( GoByHs.A)
		{
			/* add 1/4 winding's worth so as to end up at nearest halfstep */
			HsMsIx.A = MsIx.A + MaxPWM * Ms/4;
			/* subtract microsteps above the halfstep mark */
			HsMsIx.A -= HsMsIx.A % (MaxPWM * MsPerHs);
			if( HsMsIx.A > MaxMsIx)
				HsMsIx.A = 0;
		}
		if( GoByHs.Z)
		{
			HsMsIx.Z = MsIx.Z + MaxPWM * Ms/4;
			HsMsIx.Z -= HsMsIx.Z % (MaxPWM * MsPerHs);
			if( HsMsIx.Z > MaxMsIx)
				HsMsIx.Z = 0;
		}
		/* microstep by sending a PWM[] that generates voltages to motors;
		repeat outportb() MsDelayX times;
		a motor may need to be held between windings so don't power down that motor unless it is
		centered on a winding and stationary for a period of time;
		if necessary, halfstep */
		if( MotorWindings == 5)
		{
			switch( MsPowerDown)
			{
				case 0:
					if( GoByHs.A && GoByHs.Z)
						for( IxA = 0; !PWMTickHandlerCalledFlag && IxA < MaxPWM; IxA++)
							for( IxB = 0; IxB < MsDelayX; IxB++)
								WritePhase5Data( MsPWM[HsMsIx.A].A + MsPWM[HsMsIx.Z].Z * 32);
					else
						if( GoByHs.A)
							for( IxA = 0; !PWMTickHandlerCalledFlag && IxA < MaxPWM; IxA++)
								for( IxB = 0; IxB < MsDelayX; IxB++)
									WritePhase5Data( MsPWM[HsMsIx.A].A + MsPWM[MsIx.Z+IxA].Z * 32);
						else
							if( GoByHs.Z)
								for( IxA = 0; !PWMTickHandlerCalledFlag && IxA < MaxPWM; IxA++)
									for( IxB = 0; IxB < MsDelayX; IxB++)
										WritePhase5Data( MsPWM[MsIx.A+IxA].A + MsPWM[HsMsIx.Z].Z * 32);
							else
								for( IxA = 0; !PWMTickHandlerCalledFlag && IxA < MaxPWM; IxA++)
									for( IxB = 0; IxB < MsDelayX; IxB++)
										WritePhase5Data( MsPWM[MsIx.A+IxA].A + MsPWM[MsIx.Z+IxA].Z * 32);
					break;
				case 1:
					if( GoByHs.Z)
						for( IxA = 0; !PWMTickHandlerCalledFlag && IxA < MaxPWM; IxA++)
							for( IxB = 0; IxB < MsDelayX; IxB++)
								WritePhase5Data( MsPWM[MsMotorOffIx+IxA].A + MsPWM[HsMsIx.Z].Z * 32);
					else
						for( IxA = 0; !PWMTickHandlerCalledFlag && IxA < MaxPWM; IxA++)
							for( IxB = 0; IxB < MsDelayX; IxB++)
								WritePhase5Data( MsPWM[MsMotorOffIx+IxA].A + MsPWM[MsIx.Z+IxA].Z * 32);
					break;
				case 2:
					if( GoByHs.A)
						for( IxA = 0; !PWMTickHandlerCalledFlag && IxA < MaxPWM; IxA++)
							for( IxB = 0; IxB < MsDelayX; IxB++)
								WritePhase5Data( MsPWM[HsMsIx.A].A + MsPWM[MsMotorOffIx+IxA].Z * 32);
					else
						for( IxA = 0; !PWMTickHandlerCalledFlag && IxA < MaxPWM; IxA++)
							for( IxB = 0; IxB < MsDelayX; IxB++)
								WritePhase5Data( MsPWM[MsIx.A+IxA].A + MsPWM[MsMotorOffIx+IxA].Z * 32);
					break;
				case 3:
					for( IxA = 0; !PWMTickHandlerCalledFlag && IxA < MaxPWM; IxA++)
						for( IxB = 0; IxB < MsDelayX; IxB++)
							WritePhase5Data( MsPWM[MsMotorOffIx+IxA].A + MsPWM[MsMotorOffIx+IxA].Z * 32);
			}
		}
		else
		{
			switch( MsPowerDown)
			{
				case 0:
					if( GoByHs.A && GoByHs.Z)
						for( IxA = 0; !PWMTickHandlerCalledFlag && IxA < MaxPWM; IxA++)
							for( IxB = 0; IxB < MsDelayX; IxB++)
								outportb( PPortAddrOutByte, MsPWM[HsMsIx.A].A + MsPWM[HsMsIx.Z].Z);
					else
						if( GoByHs.A)
							for( IxA = 0; !PWMTickHandlerCalledFlag && IxA < MaxPWM; IxA++)
								for( IxB = 0; IxB < MsDelayX; IxB++)
									outportb( PPortAddrOutByte, MsPWM[HsMsIx.A].A + MsPWM[MsIx.Z+IxA].Z);
						else
							if( GoByHs.Z)
								for( IxA = 0; !PWMTickHandlerCalledFlag && IxA < MaxPWM; IxA++)
									for( IxB = 0; IxB < MsDelayX; IxB++)
										outportb( PPortAddrOutByte, MsPWM[MsIx.A+IxA].A + MsPWM[HsMsIx.Z].Z);
							else
								for( IxA = 0; !PWMTickHandlerCalledFlag && IxA < MaxPWM; IxA++)
									for( IxB = 0; IxB < MsDelayX; IxB++)
										outportb( PPortAddrOutByte, MsPWM[MsIx.A+IxA].A + MsPWM[MsIx.Z+IxA].Z);
					break;
				case 1:
					if( GoByHs.Z)
						for( IxA = 0; !PWMTickHandlerCalledFlag && IxA < MaxPWM; IxA++)
							for( IxB = 0; IxB < MsDelayX; IxB++)
								outportb( PPortAddrOutByte, MsPWM[MsMotorOffIx+IxA].A + MsPWM[HsMsIx.Z].Z);
					else
						for( IxA = 0; !PWMTickHandlerCalledFlag && IxA < MaxPWM; IxA++)
							for( IxB = 0; IxB < MsDelayX; IxB++)
								outportb( PPortAddrOutByte, MsPWM[MsMotorOffIx+IxA].A + MsPWM[MsIx.Z+IxA].Z);
					break;
				case 2:
					if( GoByHs.A)
						for( IxA = 0; !PWMTickHandlerCalledFlag && IxA < MaxPWM; IxA++)
							for( IxB = 0; IxB < MsDelayX; IxB++)
								outportb( PPortAddrOutByte, MsPWM[HsMsIx.A].A + MsPWM[MsMotorOffIx+IxA].Z);
					else
						for( IxA = 0; !PWMTickHandlerCalledFlag && IxA < MaxPWM; IxA++)
							for( IxB = 0; IxB < MsDelayX; IxB++)
								outportb( PPortAddrOutByte, MsPWM[MsIx.A+IxA].A + MsPWM[MsMotorOffIx+IxA].Z);
					break;
				case 3:
					for( IxA = 0; !PWMTickHandlerCalledFlag && IxA < MaxPWM; IxA++)
						for( IxB = 0; IxB < MsDelayX; IxB++)
							outportb( PPortAddrOutByte, MsPWM[MsMotorOffIx+IxA].A + MsPWM[MsMotorOffIx+IxA].Z);
			}
		}
		IxC = 0;
		do
		{
			SteppersOff_f_ptr();
		/* pause for MsPause counts */
		}while( !PWMTickHandlerCalledFlag && IxC++ <= MsPause);

		/* one microstepping voltage waveform finished */
		ActualPWMRepsTick++;

		if( PulseFR || PulseFocus)
			PulseFRFocusPerPWM();

	} while( !NewSidT());

	/* set halfstep indexes */
	HsIx.A = MsIx.A / HsIxToMsIxConvFactor;
	HsIx.Z = MsIx.Z / HsIxToMsIxConvFactor;
	IncrMs.A = IncrMs.Z = 0;
	CalcActualBacklashRad();

	UpdatePWMAverage();
	PWMUnderwayFlag = False;
	DisplayMsValues_f_ptr();
}

void UpdatePWMAverage( void)
{
	TotalPWMRepsTickArray -= PWMRepsTickArray[PWMRepsTickArrayIx];
	PWMRepsTickArray[PWMRepsTickArrayIx] = ActualPWMRepsTick;
	PWMRepsTickArrayIx++;
	PWMRepsTickArrayIx &= (SizeofPWMRepsTickArray-1);
	TotalPWMRepsTickArray += ActualPWMRepsTick;
	AvgPWMRepsTick = TotalPWMRepsTickArray / SizeofPWMRepsTickArray;
	if( AvgPWMRepsTick < 9)
		AvgPWMRepsTick = 9;
	if( HoldAvgPWMRepsTick != AvgPWMRepsTick)
	{
		if( AvgPWMRepsTickOnFlag)
		{
			PWMRepsTick = AvgPWMRepsTick;
			CalcVarsRelatingToPWMRepsTick();
		}
		HoldAvgPWMRepsTick = AvgPWMRepsTick;
	}
}

void DisplayMsValuesPWM( void)
{
	struct AZInt W;
	struct AZInt M;
	static int HoldPWMReps, HoldAvgPWMRepsTick;
	static Flag SoundOn;

	TextAttr = DisplayText;

	/* write PWMRepsTick */
	if( HoldPWMReps != ActualPWMRepsTick)
	{
		VidMemXY = DisplayXY[DisplayPWMReps];
		sprintf( StrBuf, "%3d", ActualPWMRepsTick);
		WriteStrBufToScreen_f_ptr();
		HoldPWMReps = ActualPWMRepsTick;
	}
	if( HoldAvgPWMRepsTick != AvgPWMRepsTick)
	{
		VidMemXY.Y = DisplayXY[DisplayPWMReps].Y;
		VidMemXY.X = DisplayXY[DisplayPWMReps].X+3;
		sprintf( StrBuf, "/%3d", AvgPWMRepsTick);
		WriteStrBufToScreen_f_ptr();
		HoldAvgPWMRepsTick = AvgPWMRepsTick;
	}
	W.A = MsIx.A/MaxPWM/Ms;
	W.Z = MsIx.Z/MaxPWM/Ms;
	M.A = MsIx.A/MaxPWM%Ms;
	M.Z = MsIx.Z/MaxPWM%Ms;
	VidMemXY = DisplayXY[DisplayMsStatus];
	sprintf( StrBuf, "%3d %3d %c%02d %c%02d %1d%1d%1d",
	StepsMoved.A, StepsMoved.Z, 'a'+W.A, M.A, 'a'+W.Z, M.Z, GoByHs.A, GoByHs.Z, MsPowerDown);

	WriteStrBufToScreen_f_ptr();

	if( MsZeroSoundOn)
	{
		if( SoundOn)
		{
			SoundOn = No;
			nosound();
		}
		if( !M.A)
		{
			sound( 500);
			SoundOn = Yes;
		}
		if( !M.Z)
		{
			sound( 1000);
			SoundOn = Yes;
		}
	}
}

void DisplayMsValuesPulseDir( void)
{
	static struct AZInt CurrentHighlight;

	/* unhighlight current display */
	VidMemXY.Y = DisplayXY[DisplayMsStatus].Y;
	VidMemXY.X = DisplayXY[DisplayMsStatus].X + CurrentHighlight.A;
	Screen[VidMemXY.Y][VidMemXY.X].Attr = DisplayText;
	VidMemXY.X = DisplayXY[DisplayMsStatus].X + CurrentHighlight.Z + 10;
	Screen[VidMemXY.Y][VidMemXY.X].Attr = DisplayText;

	/* highlight new */
	CurrentHighlight.A = (int) (AccumMs.A % 10);
	CurrentHighlight.Z = (int) (AccumMs.Z % 10);
	VidMemXY.Y = DisplayXY[DisplayMsStatus].Y;
	VidMemXY.X = DisplayXY[DisplayMsStatus].X + CurrentHighlight.A;
	Screen[VidMemXY.Y][VidMemXY.X].Attr = SelectText;
	VidMemXY.X = DisplayXY[DisplayMsStatus].X + CurrentHighlight.Z + 10;
	Screen[VidMemXY.Y][VidMemXY.X].Attr = SelectText;
}

void DisplayMsValuesSerial( void)
{
	static struct AZLong HoldAccumMs;
	struct AZLong Diff;

	if( HoldAccumMs.A != AccumMs.A || HoldAccumMs.Z != AccumMs.Z)
	{
		Diff.A = AccumMs.A - HoldAccumMs.A;
		Diff.Z = AccumMs.Z - HoldAccumMs.Z;
		HoldAccumMs.A = AccumMs.A;
		HoldAccumMs.Z = AccumMs.Z;
		TextAttr = DisplayText;
		VidMemXY = DisplayXY[DisplayMsStatus];
		sprintf( StrBuf, "%7ld %7ld", Diff.A, Diff.Z);
		WriteStrBufToScreen_f_ptr();
	}
}

/* moves steppers to nearest full step */
void AlignMs( void)
{
	const int MaxTries = 4;
	int count = 0;

	if( !MsAlignFlag)
	{
		/* find microsteps to nearest fullstep */
		do
		{
			Steps.A = (MsIx.A/MaxPWM) % Ms;
			if( Steps.A && Dir.A == CW)
				Steps.A = Ms - Steps.A;
			Steps.Z = (MsIx.Z/MaxPWM) % Ms;
			if( Steps.Z && Dir.Z == CW)
				Steps.Z = Ms - Steps.Z;
			MoveMs_f_ptr();
			count++;
		}
		while( count < MaxTries && (Steps.A || Steps.Z));
		if( count < MaxTries)
			MsAlignFlag = Yes;
	}
}

void MoveMsPulseDir( void)
{
	int IxA, IxB, IxC;
	Byte Out;

	SetPulseFRPulseFocusInMoveMs();
	PWMUnderwayFlag = Yes;
	PWMTickHandlerCalledFlag = No;
	ActualPWMRepsTick = 0;
	StepsMoved.A = StepsMoved.Z = 0;
	if( Steps.A < 0)
		Steps.A = 0;
	if( Steps.Z < 0)
		Steps.Z = 0;

	MsPowerDown = 0;

	/* allow power down of the motor */
	if( EnableMotor.A)
		if( Steps.A==0)
			if( MsPowerDownCount.A)
				MsPowerDownCount.A--;
			else
				MsPowerDown = 1;
		else
			MsPowerDownCount.A = StartMsPowerDownCount;
	else
	{
		MsPowerDown = 1;
		MsPowerDownCount.A = 0;
		Steps.A = 0;
	}

	if( EnableMotor.Z)
		if( Steps.Z==0)
			if( MsPowerDownCount.Z)
				MsPowerDownCount.Z--;
			else
				MsPowerDown += 2;
		else
			MsPowerDownCount.Z = StartMsPowerDownCount;
	else
	{
		MsPowerDown += 2;
		MsPowerDownCount.Z = 0;
		Steps.Z = 0;
	}

	if( IncrMs.A == 0 && IncrMs.Z == 0)
	{
		IncrMs.A = ((double) Steps.A) / (double) PWMRepsTick;
		IncrMs.Z = ((double) Steps.Z) / (double) PWMRepsTick;
	}
	if( IncrMs.A > 1)
		IncrMs.A = 1;
	if( IncrMs.Z > 1)
		IncrMs.Z = 1;
	if( Steps.A)
		MsRunningTotal.A = 1;
	else
		MsRunningTotal.A = 0;
	if( Steps.Z)
		MsRunningTotal.Z = 1;
	else
		MsRunningTotal.Z = 0;
	do
	{
		Out = 0;
		if( MsPowerDown & 1)
			A_PowerOnState = Off;
		else
			A_PowerOnState = A_Power_On;
		Out += A_PowerOnState;
		if( (!ReverseAMotor && Dir.A == CW) || (ReverseAMotor && Dir.A == CCW))
			Out += A_Dir;
		if( Steps.A)
		{
			MsRunningTotal.A += IncrMs.A;
			MsToDo.A = MsRunningTotal.A;
			MsRunningTotal.A -= MsToDo.A;
			if( MsToDo.A)
			{
				Steps.A -= MsToDo.A;
				StepsMoved.A += MsToDo.A;
				if( Dir.A == CW)
					AccumMs.A += MsToDo.A;
				else
					AccumMs.A -= MsToDo.A;
				if( A_Pulse_State)
					A_Pulse_State = Off;
				else
				{
					A_Pulse_State = On;
					Out += A_Pulse;
				}
			}
		}
		if( MsPowerDown & 2)
			Z_PowerOnState = Off;
		else
			Z_PowerOnState = Z_Power_On;
		Out += Z_PowerOnState;
		if( (!ReverseZMotor && Dir.Z == CW) || (ReverseZMotor && Dir.Z == CCW))
			Out += Z_Dir;
		if( Steps.Z)
		{
			MsRunningTotal.Z += IncrMs.Z;
			MsToDo.Z = MsRunningTotal.Z;
			MsRunningTotal.Z -= MsToDo.Z;
			if( MsToDo.Z)
			{
				Steps.Z -= MsToDo.Z;
				StepsMoved.Z += MsToDo.Z;
				if( Dir.Z == CW)
					AccumMs.Z += MsToDo.Z;
				else
					AccumMs.Z -= MsToDo.Z;
				if( Z_Pulse_State)
					Z_Pulse_State = Off;
				else
				{
					Z_Pulse_State = On;
					Out += Z_Pulse;
				}
			}
		}
		if( InvertOutput)
			Out = ~Out;
		for( IxA = 0; !PWMTickHandlerCalledFlag && IxA < MaxPWM; IxA++)
			for( IxB = 0; IxB < MsDelayX; IxB++)
				outportb( PPortAddrOutByte, Out);
		IxC = 0;
		do
		{
			outportb( PPortAddrOutByte, Out);
		}while( !PWMTickHandlerCalledFlag && IxC++ <= MsPause);
		ActualPWMRepsTick++;
		if( PulseFR || PulseFocus)
			PulseFRFocusPerPWM();
	} while( !NewSidT());
	IncrMs.A = IncrMs.Z = 0;
	CalcActualBacklashRad();
	UpdatePWMAverage();
	PWMUnderwayFlag = False;
	DisplayMsValues_f_ptr();
}


