/*==================================================================================================

CInput.cpp

Holds information about player input

==================================================================================================*/

#include "CInput.h"
#include "Game/CCamera.h"

unsigned int CInput::m_toggleState[e_inputToggleCount];
unsigned int CInput::m_toggleStateLastFrame[e_inputToggleCount];

void CInput::OnMouseMove(int relx, int rely)
{
	const float m_mouseSensitivity = 0.01f;
	CCamera::Get().YawRight(-m_mouseSensitivity * (float)relx);
	CCamera::Get().PitchUp(-m_mouseSensitivity * (float)rely);
}