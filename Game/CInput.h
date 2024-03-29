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
		#define INPUT_TOGGLE(name, resetOnKeyUp) e_inputToggle##name,
		#include "InputToggleList.h"

		e_inputToggleCount
	};

	CInput()
	{
		for (unsigned int index = 0; index < e_inputToggleCount; ++index)
		{
			m_toggleState[index] = 0;
			m_toggleStateLastFrame[index] = 0;
		}
	}

	static void SetInputToggle(EInputToggles inputToggle, bool enable, bool fromInput = true)
	{
		if (fromInput && !enable)
		{
			switch(inputToggle)
			{
				#define INPUT_TOGGLE(name, resetOnKeyUp) case e_inputToggle##name: if(!resetOnKeyUp) return;break;
				#include "InputToggleList.h"
			}
		}

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

	static void Update ();

private:
	static unsigned int m_toggleState[e_inputToggleCount];
	static unsigned int m_toggleStateLastFrame[e_inputToggleCount];
};