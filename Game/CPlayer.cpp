/*==================================================================================================

CPlayer.cpp

Holds info about the player and also handles player input

==================================================================================================*/

#include "CPlayer.h"
#include "CInput.h"
#include "CCamera.h"

static const float c_playerHeightCrouched = 1.5f;
static const float c_playerHeightStanding = 3.0f;

//--------------------------------------------------------------------------------------------------
CPlayer::CPlayer()
{
	m_crouched = false;
	m_onGround = false;

	m_playerHeight = c_playerHeightStanding;

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

	if (m_onGround && CInput::InputToggleActivated(CInput::e_inputToggleJump))
		m_velocity[1] = 0.1f;

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


	if (m_crouched && m_playerHeight > c_playerHeightCrouched)
	{
		m_playerHeight -= elapsed * 10.0f;
		if (m_playerHeight < c_playerHeightCrouched)
			m_playerHeight = c_playerHeightCrouched;
	}
	else if (!m_crouched && m_playerHeight < c_playerHeightStanding)
	{
		m_playerHeight += elapsed * 10.0f;
		if (m_playerHeight > c_playerHeightStanding)
			m_playerHeight = c_playerHeightStanding;
	}

	float3 playerHeight = {0.0f, m_playerHeight, 0.0f};
	CCamera::Get().SetPosition(m_position + playerHeight);
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