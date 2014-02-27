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

void InitMouseControl( void)
{
	static Flag MouseControlMsgWrittenFlag;

	if( strcmpi( EncoderString, "Mouse") == 0)
		UseMouseFlag = No;
	if( UseMouseFlag)
	{
		MouseRightTimer = 0;
		MouseLeftTimer = 0;
		MouseLeftClickCount = MouseRightClickCount = 0;
		SlewStartedFromMouse = False;
		MouseModeAuxPinActive = False;
		MouseMode = MouseModeMenu;
		InitMouse();
		DisplayMouse();
		if( !MouseControlMsgWrittenFlag && DisplayOpeningMsgs)
		{
			printf( "\nUsing Mouse Control");
			MouseControlMsgWrittenFlag = True;
		}
	}
}

void CloseMouseControl( void)
{
	if( UseMouseFlag)
	{
		ResetMouse();
		HideMouse();
	}
}

/* return yes if mouse event causes processing > timer tick */
Flag ProcessMouseEvent( void)
{
	static int HoldMouseLeftClickCount;

	/* if aux mouse motion active, turn it off if mode switched away from mouse aux, or if button
	released, otherwise, return No to prevent further mouse processing while mouse aux motion on */
	if( MouseModeAuxPinActive)
		if( MouseMode != MouseModeAux || MouseLeftButtonReleaseCount())
		{
			AuxControl = AuxOff;
			MouseModeAuxPinActive = No;
			AuxControlBiDirPPort();
		}

	/* handle right button event */
	if( MouseRightButtonPressCount())
	{
		MouseRightClickCount++;
		MouseRightTimer = 1;
		MouseLeftTimer = 0;
	}
	if( MouseRightTimer > 0)
		MouseRightTimer++;
	if( MouseRightTimer > 12)
	{
		MouseRightTimer = 0;
		MouseMode = MouseRightClickCount - 1;
		if( MouseMode > MaxMouseMode)
			MouseMode = MaxMouseMode - 1;
		WriteMouseMode();
		MouseRightClickCount = 0;
	}

	/* handle left button event based on MouseMode */
	if( MouseMode == MouseModeMenu)
	{
		if (MouseLeftButtonPressCount())
			return ProcessMouseEventMenu();
	}
	else
	{
		InitiatedType = NotInitiated;
		MouseLeftClickCount += MouseLeftButtonPressCount();
		if( MouseLeftClickCount != HoldMouseLeftClickCount)
		{
			HoldMouseLeftClickCount = MouseLeftClickCount;
			MouseLeftTimer = 1;
			MouseRightTimer = 0;
		}
		if( MouseLeftTimer)
			MouseLeftTimer++;
		if( MouseLeftTimer > 12)
			if( MouseLeftClickCount < 5 && MouseLeftButtonDown)
			{
				MouseMoveResult = MouseLeftClickCount - 1;
				if( MouseMode == MouseModeMs || MouseMode == MouseModeHs)
				{
					ProcessMouseModeHsMs();
					return Yes;
				}
				else
					if( MouseMode == MouseModeAux)
						ProcessMouseModeAux();
					else
						if( MouseMode == MouseModeFocus)
							return ProcessMouseModeFocusByMethod();
			}
			else
				MouseLeftTimer = MouseLeftClickCount = HoldMouseLeftClickCount = 0;
	}
	return No;
}

Flag ProcessMouseEventMenu( void)
{
	int MenuRow, MenuCol;

	/* if mouse click is on menu category line */
	if( MouseYText == MenuCatsY+1 && MouseXText>=MenuStartX && MouseXText<=MenuEndX)
	{
		/* scan until mouse X exceeds start of a menu category X: selected menu category X will then
		be previous menu category */
		for( Ix = 0; Ix < MaxMenuCats && MouseXText > MenuCatX[Ix]; Ix++)
			;
		Ix--;
		NewMenuCat = Ix;
		/* set sub menu item to upper left most item */
		NewMenuSub = 0;
		UpdateMenuCatSub();
		return Yes;
	}
	else
		/* if mouse click is on a sub menu item */
		if( MouseYText>MenuSubMenuY && MouseYText<MenuSubMenuY+MaxMenuDisplayRows+1
		&& MouseXText>=MenuStartX && MouseXText<=MenuEndX)
		{
			MenuRow = MouseYText - MenuSubMenuY - 1;
			for( MenuCol = 0; MenuCol < MaxMenuSubs && MouseXText > MenuSubX[MenuCol]; MenuCol++)
				;
			MenuCol--;
			NewMenuSub = MenuCol * MaxMenuDisplayRows + MenuRow;
			/* if mouse click on empty sub menu item, ignore */
			if( NewMenuSub < MenuArray[CurrentMenuCat].NumSubTitles)
			{
				UpdateMenuCatSub();
				Response = MenuArray[CurrentMenuCat].SubTitles[CurrentMenuSub].MenuItem;
				ProcessMenuSelection();
				return Yes;
			}
		}
	return No;
}

void ProcessMouseModeHsMs( void)
{
	struct AZLongV HoldSteps;

	if( MouseMode == MouseModeMs)
	{
		/* see ProcessHPEventsMoveMotors() */
		StepsToMove.A = MsTick.A;
		StepsToMove.Z = MsTick.Z;
	}
	else
		if( MouseMode == MouseModeHs)
			StepsToMove.A = StepsToMove.Z = MaxHs;

	Steps.A = Steps.Z = 0;
	switch( MouseMoveResult)
	{
		case MouseMovedUp:
			Steps.A = StepsToMove.A;
			Dir.A = CW;
			break;
		case MouseMovedDown:
			Steps.A = StepsToMove.A;
			Dir.A = CCW;
			break;
		case MouseMovedLeft:
			Steps.Z = StepsToMove.Z;
			Dir.Z = CCW;
			break;
		case MouseMovedRight:
			Steps.Z = StepsToMove.Z;
			Dir.Z = CW;
	}
	if( MouseMode == MouseModeMs)
	{
		HoldSteps = Steps;
		while( !MouseLeftButtonReleaseCount())
		{
			HPEventMoveMs();
			HPEventGetEquat();
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
		if( MouseMode == MouseModeHs)
		{
			SlewStartedFromMouse = True;
			KBEventMoveHs();
			SlewStartedFromMouse = False;
			PauseUntilNewSidTime();
			HPEventGetEquat();
		}
}

/* see ProcessHPEventsHandpadAux() */
void ProcessMouseModeAux( void)
{
	InitiatedType = MouseInitiated;
	switch( MouseMoveResult)
	{
		case MouseMovedUp:
			AuxControl = Aux1;
			MouseModeAuxPinActive = Yes;
			break;
		case MouseMovedDown:
			AuxControl = Aux14;
			MouseModeAuxPinActive = Yes;
			break;
		case MouseMovedRight:
			AuxControl = Aux16;
			MouseModeAuxPinActive = Yes;
			break;
		case MouseMovedLeft:
			AuxControl = Aux17;
			MouseModeAuxPinActive = Yes;
	}
	AuxControlBiDirPPort();
}

Flag ProcessMouseModeFocusByMethod( void)
{
	InitiatedType = MouseInitiated;

	if( MouseMoveResult == MouseMovedLeft)
		FocusControl = FocusMinus;
	else
		if( MouseMoveResult == MouseMovedRight)
			FocusControl = FocusPlus;
		else
			FocusControl = FocusStop;

	if( FocusMethod == FocusMethod_OnOff_16_17)
		ProcessMouseFocus_OnOff_16_17();
	else
		if( FocusMethod == FocusMethod_OnOff_16_17_Slow1_14)
			ProcessMouseFocus_OnOff_16_17_Slow1_14();
		else
			if( FocusMethod == FocusMethod_Pulse_1_14 || FocusMethod == FocusMethod_Pulse_16_17)
				/* if tracking on, control of focus motor handled by MoveMs() */
				if( TrackFlag)
					return No;
				else
					if( MouseMoveResult == MouseMovedLeft || MouseMoveResult == MouseMovedRight ||
					MouseMoveResult == MouseMovedUp || MouseMoveResult == MouseMovedDown)
					{
						if( !InsideProcessHPEventsFocusMethod_Pulse_Dir)
							ProcessHPEventsFocusMethod_Pulse_Dir();
						return Yes;
					}
	return No;
}

void ProcessMouseFocus_OnOff_16_17( void)
{
	FocusControl_OnOff_16_17();
}

void ProcessMouseFocus_OnOff_16_17_Slow1_14( void)
{
	if( MouseMoveResult == MouseMovedLeft || MouseMoveResult == MouseMovedRight)
		ProcessMouseFocus_OnOff_16_17();
	else
	{
		if( MouseMoveResult == MouseMovedDown)
			HandpadFocusButtonPressed = Yes;
		else
			if( MouseMoveResult == MouseMovedUp)
				HandpadFocusButtonPressed = Yes;
			else
				HandpadFocusButtonPressed = No;

		FocusControl_OnOff_16_17_Slow1_14();
	}
}

void WriteMouseMode( void)
{
	TextAttr = DisplayText;
	VidMemXY = DisplayXY[DisplayMouseMode];
	switch( MouseMode)
	{
		case MouseModeMenu:
			sprintf( StrBuf, "menu ");
			break;
		case MouseModeMs:
			sprintf( StrBuf, "ms   ");
			break;
		case MouseModeHs:
			sprintf( StrBuf, "HS   ");
			break;
		case MouseModeAux:
			sprintf( StrBuf, "aux  ");
			break;
		case MouseModeFocus:
			sprintf( StrBuf, "focus");
	}
	WriteStrBufToScreen_f_ptr();
}

