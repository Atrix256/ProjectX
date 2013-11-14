/*==================================================================================================

CInput.h

Holds information about player input

==================================================================================================*/

#pragma once

#include "Platform/Assert.h"
#include <string.h>

class CInput
{
public:
	enum EInputToggles
	{
		e_inputToggleWalkForward,
		e_inputToggleWalkLeft,
		e_inputToggleWalkRight,
		e_inputToggleWalkBack,

		e_inputToggleJump,
		e_inputToggleCrouch,

		e_inputToggleCount,
	};

	CInput()
	{
		for (unsigned int index = 0; index < e_inputToggleCount; ++index)
		{
			m_toggleState[index] = 0;
			m_toggleStateLastFrame[index] = 0;
		}
	}

	static void SetInputToggle(EInputToggles inputToggle, bool enable)
	{
		Assert_(inputToggle >= 0 && inputToggle < e_inputToggleCount);
		m_toggleState[inputToggle] = enable;

		// key repeat makes this not work right, fix later (this is in the todo.txt file)
		/*
		if (enable)
		{
			++m_toggleState[inputToggle];
		}
		else
		{
			Assert_(m_toggleState[inputToggle] > 0);
			--m_toggleState[inputToggle];
		}
		*/
	}

	static void OnMouseMove(int relx, int rely);

	// is the toggle On?
	static bool InputToggleOn(EInputToggles inputToggle) { return m_toggleState[inputToggle] > 0; }

	// was the toggle activated this frame?
	static bool InputToggleActivated(EInputToggles inputToggle) { return m_toggleState[inputToggle] != 0 && m_toggleStateLastFrame[inputToggle] == 0; }

	// was the toggle deactivated this frame?
	static bool InputToggleDeactivated(EInputToggles inputToggle) { return m_toggleState[inputToggle] == 0 && m_toggleStateLastFrame[inputToggle] != 0; }

	static void Update ()
	{
		memcpy(m_toggleStateLastFrame, m_toggleState, sizeof(m_toggleState));
	}

private:
	static unsigned int m_toggleState[e_inputToggleCount];
	static unsigned int m_toggleStateLastFrame[e_inputToggleCount];
};