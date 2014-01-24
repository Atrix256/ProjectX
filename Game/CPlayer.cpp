/*==================================================================================================

CPlayer.cpp

Holds info about the player and also handles player input

==================================================================================================*/

#include "CPlayer.h"
#include "CInput.h"
#include "CCamera.h"
#include "CGame.h"

//--------------------------------------------------------------------------------------------------
CPlayer::CPlayer()
{
	m_crouched = false;
	m_onGround = false;

	m_playerHeight = 0.0f;

	m_position[0] = 0.0f;
	m_position[1] = 0.0f;
	m_position[2] = 0.0f;

	m_velocity[0] = 0.0f;
	m_velocity[1] = 0.0f;
	m_velocity[2] = 0.0f;
}

//--------------------------------------------------------------------------------------------------
void CPlayer::Init ()
{
	m_playerHeight = CGame::GameData().m_StandingHeight;
}

//--------------------------------------------------------------------------------------------------
void CPlayer::Update (float elapsed)
{
	const float moveAmount = m_crouched ? 1.5f : 5.0f;
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

	if (CInput::InputToggleOn(CInput::e_inputToggleJump))
	{
		CInput::SetInputToggle(CInput::e_inputToggleJump, false, false);
		if (m_onGround)
			m_velocity[1] = 0.1f;
	}

	m_position += m_velocity;

	float currentGroundHeight = CCamera::Get().CurrentGroundHeight();

	if (m_position[1] > currentGroundHeight)
	{
		m_onGround = false;
		m_velocity[1] -= 0.3f * elapsed;
	}
	else
	{
		m_position[1] = currentGroundHeight;
		m_onGround = true;
		m_velocity[1] = 0.0f;
	}

	// handle toggling crouch
	if (CInput::InputToggleOn(CInput::e_inputToggleCrouch) != m_crouched)
		m_crouched = !m_crouched;

	const SData_GameData& gameData = CGame::GameData();
	if (m_crouched && m_playerHeight > gameData.m_CrouchingHeight)
	{
		m_playerHeight -= elapsed * gameData.m_CrouchSpeed;
		if (m_playerHeight < gameData.m_CrouchingHeight)
			m_playerHeight = gameData.m_CrouchingHeight;
	}
	else if (!m_crouched && m_playerHeight < gameData.m_StandingHeight)
	{
		m_playerHeight += elapsed * gameData.m_CrouchSpeed;
		if (m_playerHeight > gameData.m_StandingHeight)
			m_playerHeight = gameData.m_StandingHeight;
	}

	float3 playerHeight = {0.0f, m_playerHeight, 0.0f};
	CCamera::Get().SetPosition(m_position + playerHeight);

	// right now movement is not fleshed out very well, so the camera does sector traversal and
	// collision detection.  So, get the position in case the camera changed it.
	CCamera::Get().GetPosition(m_position);
	m_position -= playerHeight;
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
void CPlayer::SetFacing(float X, float Y, float Z)
{
	CCamera::Get().SetFacing(X, Y, Z);
}