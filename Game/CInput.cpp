/*==================================================================================================

CInput.cpp

Holds information about player input

==================================================================================================*/

#include "CInput.h"
#include "Game/CCamera.h"
#include "ECS/ECS.h"

unsigned int CInput::m_toggleState[e_inputToggleCount];
unsigned int CInput::m_toggleStateLastFrame[e_inputToggleCount];

void CInput::OnMouseMove(int relx, int rely)
{
	ECS::OnMouseMove((float)relx, (float)rely);
}

void CInput::Update ()
{
	memcpy(m_toggleStateLastFrame, m_toggleState, sizeof(m_toggleState));

	ECS::SetInputState(
	#define INPUT_TOGGLE(name, resetOnKeyUp) InputToggleOn(e_inputToggle##name),
	#include "InputToggleList.h"
	NULL);
}