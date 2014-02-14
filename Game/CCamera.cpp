/*==================================================================================================

CCamera.cpp

This is the camera singleton class.  Manipulate this object to change the view of the world.

==================================================================================================*/

#include "Platform\CDirectx.h"
#include "CCamera.h"
#include "Game/MatrixMath.h"

// singleton
CCamera CCamera::s_camera;

void CCamera::AttemptMove (const float3 &delta)
{
	//move the camera
	SCamera &camera = SSharedDataRootHostToKernel::Camera();
	camera.m_pos += delta;

	//get our sectors
	const CWorld &world = CDirectX::GetWorld();
	unsigned int numSectors = 0;
	const SSector* sectors = world.GetSectors(numSectors);

	// get our portals
	unsigned int numPortals = 0;
	const SPortal* portals = world.GetPortals(numPortals);

	// clamp the position against the walls, or traverse portals,
	// as is appropriate, until we are done
	while (true)
	{
		// bail if invalid sector
		if (camera.m_sector >= numSectors)
			return;

		const SSector& sector = sectors[camera.m_sector];

		// find out which wall we are most outside of, if any
		float maxDistOutside = 0.0f;
		int maxDistOutsideAxis = 0;
		for (int index = 0; index < 3; ++index)
		{
			float distOutside = abs(camera.m_pos[index]) - sector.m_halfDims[index];
			if (index == 0 || distOutside > maxDistOutsideAxis)
			{
				maxDistOutside = distOutside;
				maxDistOutsideAxis = index;
			}
		}

		// if we are not outside of a wall, bail out
		if (maxDistOutside <= 0.0f)
			return;

		// if we are outside of a wall, and there's no portal there, clamp our position to the sector and bail out
		int maxDistOutsidePlane = maxDistOutsideAxis * 2;
		if (camera.m_pos[maxDistOutsideAxis] < 0)
			maxDistOutsidePlane++;

		unsigned int portalIndex = sector.m_planes[maxDistOutsidePlane].m_portalIndex;
		if (portalIndex >= numPortals)
		{
			for (int index = 0; index < 3; ++index)
			{
				if (camera.m_pos[index] < -sector.m_halfDims[index])
					camera.m_pos[index] = -sector.m_halfDims[index];
				else if (camera.m_pos[index] > sector.m_halfDims[index])
					camera.m_pos[index] = sector.m_halfDims[index];
			}

			return;
		}

		const SPortal &portal = portals[portalIndex];
		camera.m_sector = portal.m_sector;
		float3 outPos;
		TransformPointByMatrix(outPos, camera.m_pos, portal.m_xaxis, portal.m_yaxis, portal.m_zaxis, portal.m_waxis);
		camera.m_pos = outPos;

		TransformFacing(portal.m_xaxis, portal.m_yaxis, portal.m_zaxis);
	}
}

float CCamera::CurrentGroundHeight () const {
	const SCamera &camera = SSharedDataRootHostToKernel::CameraConst();
	const CWorld &world = CDirectX::GetWorld();
	unsigned int numSectors = 0;
	const SSector* sectors = world.GetSectors(numSectors);
	if (camera.m_sector < numSectors)
		return -sectors[camera.m_sector].m_halfDims[1];

	return 0.0f;
}

void CCamera::TransformFacing (const cl_float4& xAxis, const cl_float4& yAxis, const cl_float4& zAxis)
{
	// transform the basis vectors
	SCamera &camera = SSharedDataRootHostToKernel::Camera();
	TransformVectorByMatrix(camera.m_fwd , xAxis, yAxis, zAxis);
	SetFacing(camera.m_fwd[0],camera.m_fwd[1], camera.m_fwd[2]);
}