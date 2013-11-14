/*==================================================================================================

CPlayer.cpp

Holds info about the player and also handles player input

==================================================================================================*/

#include "CPlayer.h"
#include "CInput.h"
#include "CCamera.h"

//--------------------------------------------------------------------------------------------------
void CPlayer::Update (float elapsed)
{
	const float moveAmount = 5.0f;
	float3 moveDelta = {0.0f, 0.0f, 0.0f};

	if (CInput::InputToggleOn(CInput::e_inputToggleWalkForward))
		moveDelta += CCamera::Get().Forward2D() * moveAmount * elapsed;

	if (CInput::InputToggleOn(CInput::e_inputToggleWalkBack))
		moveDelta -= CCamera::Get().Forward2D() * moveAmount * elapsed;

	if (CInput::InputToggleOn(CInput::e_inputToggleWalkLeft))
		moveDelta += CCamera::Get().Left() * moveAmount * elapsed;

	if (CInput::InputToggleOn(CInput::e_inputToggleWalkRight))
		moveDelta -= CCamera::Get().Left() * moveAmount * elapsed;

	m_position += moveDelta;
	CCamera::Get().SetPosition(m_position);

	if (CInput::InputToggleActivated(CInput::e_inputToggleJump))
	{
		CCamera::Get().MoveLeft(1.0);
	}
}

//--------------------------------------------------------------------------------------------------
void CPlayer::SetPosition(float X, float Y, float Z)
{
	m_position[0] = X;
	m_position[1] = Y;
	m_position[2] = Z;
	CCamera::Get().SetPosition(m_position);
}

//--------------------------------------------------------------------------------------------------
void CPlayer::SetFacing(float angle)
{
	CCamera::Get().SetYaw(angle);
}