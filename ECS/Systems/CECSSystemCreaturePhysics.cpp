/*==================================================================================================

CECSSystemCreaturePhysics.cpp

The code for "Systems", which are part of the ECS Entity-Component-Systems model.

==================================================================================================*/

#include "ECS/Systems.h"
#include "ECS/Components.h"
#include "ECS/ECS.h"

//--------------------------------------------------------------------------------------------------
void CECSSystemCreaturePhysics::UpdateEntity (
	float elapsedSeconds,
	CECSComponentBearings &bearings,
	CECSComponentPhysics &physics
)
{
	// get our sector information
	const CSharedArray<SSector> &sectorsArray = ECS::GetWorldData();
	unsigned int sectorsCount = sectorsArray.Count();

	// if entity is in an invalid sector, bail out
	if (bearings.m_sector >= sectorsCount)
		return;

	// get our specific sector
	const SSector &sector = sectorsArray.DataConst()[bearings.m_sector];

	// move, keeping within the AABB
	for (int index = 0; index < 3; ++index)
	{
		// if moving positively, calculate collision time to the positive wall
		float collisionTime;
		if (physics.m_positionDelta[index] >= 0.0f)
			collisionTime = (sector.m_halfDims[index] - physics.m_cylinderHalfDims[index] - bearings.m_position[index]) / physics.m_positionDelta[index];
		// else moving negatively, so calculate collision time to the negative wall
		else
			collisionTime = (-sector.m_halfDims[index] + physics.m_cylinderHalfDims[index] - bearings.m_position[index]) / physics.m_positionDelta[index];

		// move up to 1.0 on each axis independantly, to make the cylinder slide along the AABB
		bearings.m_position[index] += physics.m_positionDelta[index] * min(collisionTime, 1.0f);
	}
}
