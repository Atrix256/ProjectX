/*==================================================================================================

CCamera.h

This is the camera singleton class.  Manipulate this object to change the view of the world.

==================================================================================================*/

#pragma once

#include "Platform/float3.h"
#include "KernelCode/Shared/SSharedDataRoot.h"

class CCamera
{
public:
	CCamera ()
	{
		SCamera &cameraShared = SSharedDataRootHostToKernel::Camera();

		cameraShared.m_sector = 0;

		cameraShared.m_pos[0] = 0.0f;
		cameraShared.m_pos[1] = 0.0f;
		cameraShared.m_pos[2] = 0.0f;

		cameraShared.m_fwd[0] = 0.0f;
		cameraShared.m_fwd[1] = 0.0f;
		cameraShared.m_fwd[2] = 1.0f;

		cameraShared.m_up[0] = 0.0f;
		cameraShared.m_up[1] = 1.0f;
		cameraShared.m_up[2] = 0.0f;

		cameraShared.m_left[0] = -1.0f;
		cameraShared.m_left[1] =  0.0f;
		cameraShared.m_left[2] =  0.0f;

		cameraShared.m_frameCount = 0;

		// width
		cameraShared.m_viewWidthHeightDistance[0] = 6.0f;

		// height (will be calculated later)
		cameraShared.m_viewWidthHeightDistance[1] = 1.0f;

		// distance
		cameraShared.m_viewWidthHeightDistance[2] = 6.0f;

		cameraShared.m_brightnessMultiplier = 1.0f;

		m_cameraAngleX = 0.0f;
		m_cameraAngleY = 0.0f;

		m_autoAdjustBrightness = false;

		UpdateCameraFacing();
	}

	// singleton access
	static CCamera& Get () { return s_camera; }

	void GetPosition(float3 &pos)
	{
		pos = SSharedDataRootHostToKernel::Camera().m_pos;
	}

	void SetPosition(const float3 &pos)
	{
		SSharedDataRootHostToKernel::Camera().m_pos = pos;
		float3 delta = {0,0,0};
		AttemptMove(delta);
	}

	float3 Forward () const
	{
		return SSharedDataRootHostToKernel::CameraConst().m_fwd;
	}

	float3 Forward2D () const
	{
		float3 fwd2D = SSharedDataRootHostToKernel::CameraConst().m_fwd;
		fwd2D[1] = 0.0f;
		fwd2D = normalize(fwd2D);
		return fwd2D;
	}
	
	float3 Left () const
	{
		return SSharedDataRootHostToKernel::CameraConst().m_left;
	}

	void MoveForward (float amount)
	{
		SCamera &cameraShared = SSharedDataRootHostToKernel::Camera();
		AttemptMove(cameraShared.m_fwd * amount);
	}

	void MoveForward2D (float amount)
	{
		SCamera &cameraShared = SSharedDataRootHostToKernel::Camera();
		float3 fwd2D = cameraShared.m_fwd;
		fwd2D[1] = 0.0f;
		fwd2D = normalize(fwd2D);
		AttemptMove(fwd2D * amount);
	}

	void MoveLeft (float amount)
	{
		SCamera &cameraShared = SSharedDataRootHostToKernel::Camera();
		AttemptMove(cameraShared.m_left * amount);
	}

	void YawRight (float amount)
	{
		m_cameraAngleX += amount;
		UpdateCameraFacing();
	}

	void PitchUp (float amount)
	{
		m_cameraAngleY += amount;

		if (m_cameraAngleY > 1.5f)
			m_cameraAngleY = 1.5f;
		else if (m_cameraAngleY < -1.5f)
			m_cameraAngleY = -1.5f;

		UpdateCameraFacing();
	}

	void SetYaw (float angle)
	{
		m_cameraAngleX = angle;
		UpdateCameraFacing();
	}

	void SetFacing(float X, float Y, float Z)
	{
		float mag = sqrt(X*X +Y*Y + Z*Z);
		m_cameraAngleX = atan2(Z, X);
		m_cameraAngleY = asin(Y/mag);
		UpdateCameraFacing();
	}

	void UpdateCameraFacing ()
	{
		SCamera &cameraShared = SSharedDataRootHostToKernel::Camera();
		cameraShared.m_fwd[0] = cos(m_cameraAngleX) * cos(m_cameraAngleY);
		cameraShared.m_fwd[1] = sin(m_cameraAngleY);
		cameraShared.m_fwd[2] = sin(m_cameraAngleX) * cos(m_cameraAngleY);

		float3 tempUp = {0.0f, 1.0f, 0.0f};
		cameraShared.m_left = normalize(cross(cameraShared.m_fwd, tempUp));

		cameraShared.m_up = normalize(cross(cameraShared.m_left, cameraShared.m_fwd));
	}

	void AttemptMove (const float3 &delta);

	float CurrentGroundHeight () const;

	void TransformFacing (const cl_float4& xAxis, const cl_float4& yAxis, const cl_float4& zAxis);

	void SetAutoAjustBrightness (bool autoAdjustBrightness) { m_autoAdjustBrightness = autoAdjustBrightness; }

	bool AutoAdjustBrightness () const { return m_autoAdjustBrightness; }

private:
	// spherical coordinates of the camera (just angular coordinates to describe a direction)
	float m_cameraAngleX;
	float m_cameraAngleY;

	bool m_autoAdjustBrightness;

	// singleton
	static CCamera s_camera;
};