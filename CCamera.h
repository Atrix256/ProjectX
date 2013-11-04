#include "float3.h"
#include "SCamera.h"

class CCamera
{
public:
	CCamera ()
	{
		m_cameraData.m_pos[0] = 0.0f;
		m_cameraData.m_pos[1] = 0.0f;
		m_cameraData.m_pos[2] = -6.0f;

		m_cameraData.m_fwd[0] = 0.0f;
		m_cameraData.m_fwd[1] = 0.0f;
		m_cameraData.m_fwd[2] = 1.0f;

		m_cameraData.m_up[0] = 0.0f;
		m_cameraData.m_up[1] = 1.0f;
		m_cameraData.m_up[2] = 0.0f;

		m_cameraData.m_left[0] = -1.0f;
		m_cameraData.m_left[1] =  0.0f;
		m_cameraData.m_left[2] =  0.0f;

		m_viewDistance = 6.0f;
		m_viewWidth = 6.0f;

		m_cameraAngleX = 1.57f;
		m_cameraAngleY = 0.0f;

		UpdateCameraFacing();
	}

	// singleton access
	static CCamera& Get () { return s_camera; }

	// camera data access
	SCamera& GetCameraData () { return m_cameraData; }

	float ViewDistance () const { return m_viewDistance; }
	float ViewWidth () const { return m_viewWidth; }

	void MoveForward (float amount)
	{
		m_cameraData.m_pos = m_cameraData.m_pos + m_cameraData.m_fwd * amount;
	}

	void MoveForward2D (float amount)
	{
		float3 fwd2D = m_cameraData.m_fwd;
		fwd2D[1] = 0.0f;
		fwd2D = normalize(fwd2D);
		m_cameraData.m_pos = m_cameraData.m_pos + fwd2D * amount;
	}

	void MoveLeft (float amount)
	{
		m_cameraData.m_pos = m_cameraData.m_pos + m_cameraData.m_left * amount;
	}

	void YawRight (float amount)
	{
		m_cameraAngleX += amount;
		UpdateCameraFacing();
	}

	void YawUp (float amount)
	{
		m_cameraAngleY += amount;

		if (m_cameraAngleY > 1.5f)
			m_cameraAngleY = 1.5f;
		else if (m_cameraAngleY < -1.5f)
			m_cameraAngleY = -1.5f;

		UpdateCameraFacing();
	}

	void UpdateCameraFacing ()
	{
		m_cameraData.m_fwd[0] = cos(m_cameraAngleX) * cos(m_cameraAngleY);
		m_cameraData.m_fwd[1] = sin(m_cameraAngleY);
		m_cameraData.m_fwd[2] = sin(m_cameraAngleX) * cos(m_cameraAngleY);

		float3 tempUp = {0.0f, 1.0f, 0.0f};
		m_cameraData.m_left = normalize(cross(m_cameraData.m_fwd, tempUp));

		m_cameraData.m_up = normalize(cross(m_cameraData.m_left, m_cameraData.m_fwd));
	}

private:
	// camera data in shared camera data format
	SCamera m_cameraData;
	float m_viewDistance;
	float m_viewWidth;

	float m_cameraAngleX;
	float m_cameraAngleY;

	// singleton
	static CCamera s_camera;
};