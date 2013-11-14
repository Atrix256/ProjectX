/*==================================================================================================

CPlayer.cpp

Holds info about the player and also handles player input

==================================================================================================*/

#include "CPlayer.h"
#include "CInput.h"
#include "CCamera.h"

//--------------------------------------------------------------------------------------------------
CPlayer::CPlayer()
{
	m_onGround = false;

	m_position[0] = 0.0f;
	m_position[1] = 0.0f;
	m_position[2] = 0.0f;

	m_velocity[0] = 0.0f;
	m_velocity[1] = 0.0f;
	m_velocity[2] = 0.0f;
}

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

	if (m_onGround && CInput::InputToggleActivated(CInput::e_inputToggleJump))
		m_velocity[1] = 0.1f;

	m_position += m_velocity;

	if (m_position[1] > 0.0f)
	{
		m_onGround = false;
		m_velocity[1] -= 0.3f * elapsed;
	}
	else
	{
		m_onGround = true;
		m_velocity[1] = 0.0f;
	}

	CCamera::Get().SetPosition(m_position);
}

//--------------------------------------------------------------------------------------------------
void CPlayer::SetPosition(float X, float Y, float Z)
{
	m_onGround = false;
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