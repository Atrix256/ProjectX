/*==================================================================================================

clrt.cl

The kernel code 

==================================================================================================*/

#include "Shared/SSharedDataRoot.h"
#include "Shared/SharedGeometry.h"

#define c_maxRayBounces 6

const sampler_t g_textureSampler = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_REPEAT | CLK_FILTER_NEAREST; 

struct SCollisionInfo
{
	TObjectId			m_objectHit;
	bool 				m_fromInside;
	float3				m_intersectionPoint;
	float				m_intersectionTime;
	float3				m_surfaceNormal;
	float3				m_surfaceU;
	float3				m_surfaceV;
	float2				m_textureCoordinates;
	unsigned int		m_materialIndex;
	unsigned int		m_portalIndex;
};

bool RayIntersectSphere (__constant const struct SSphere *sphere, struct SCollisionInfo *info, const float3 rayPos, const float3 rayDir, const TObjectId ignorePrimitiveId)
{
	if (ignorePrimitiveId == sphere->m_objectId)
		return false;

	// get the vector from the center of this circle to where the ray begins.
	float3 m = rayPos - sphere->m_positionAndRadius.xyz;

    // get the dot product of the above vector and the ray's vector
	float b = dot(m, rayDir);

	float c = dot(m, m) - sphere->m_positionAndRadius.w * sphere->m_positionAndRadius.w;

	//exit if r's origin outside s (c > 0) and r pointing away from s (b > 0)
	if(c > 0.0 && b > 0.0)
		return false;

	//calculate discriminant
	float discr = b * b - c;

	//a negative discriminant corresponds to ray missing sphere
	if(discr < 0.0)
		return false;

	//not inside til proven otherwise
	bool fromInside = false;

	//ray now found to intersect sphere, compute smallest t value of intersection
	float collisionTime = -b - sqrt(discr);

	//if t is negative, ray started inside sphere so clamp t to zero and remember that we hit from the inside
	if(collisionTime < 0.0)
	{
		collisionTime = -b + sqrt(discr);
		fromInside = true;
	}

	//enforce max distance
	if(collisionTime > info->m_intersectionTime)
		return false;

	// set all the info params since we are garaunteed a hit at this point
	info->m_fromInside = fromInside;
	info->m_materialIndex = sphere->m_materialIndex;
	info->m_portalIndex = sphere->m_portalIndex;

	//compute the point of intersection
	info->m_intersectionPoint = rayPos + rayDir * collisionTime;
	info->m_intersectionTime = collisionTime;

	// calculate the normal
	info->m_surfaceNormal = info->m_intersectionPoint - sphere->m_positionAndRadius.xyz;
	info->m_surfaceNormal = normalize(info->m_surfaceNormal);

	// calculate U and V
	float3 up = {0, 1, 0};
	info->m_surfaceU = normalize(cross(up, info->m_surfaceNormal));
	info->m_surfaceV = normalize(cross(info->m_surfaceU, info->m_surfaceNormal));

	// texture coordinates are just the angular part of spherical coordiantes of normal
	info->m_textureCoordinates.x = atan2(info->m_surfaceNormal.y, info->m_surfaceNormal.x);
	info->m_textureCoordinates.y = acos(info->m_surfaceNormal.z );
	info->m_textureCoordinates *= sphere->m_textureScale;

	// we found a hit!
	info->m_objectHit = sphere->m_objectId;
	return true;
}

bool RayIntersectAABox (__constant const struct SAABox *box, struct SCollisionInfo *info, const float3 rayPos, const float3 rayDir, const TObjectId ignorePrimitiveId)
{
	if (ignorePrimitiveId == box->m_objectId)
		return false;
	
	float rayMinTime = 0.0;
	float rayMaxTime = FLT_MAX;
	
	//enforce a max distance
	rayMaxTime = info->m_intersectionTime;

	// find the intersection of the intersection times of each axis to see if / where the
	// ray hits.
	for(int axis = 0; axis < 3; ++axis)
	{
		//calculate the min and max of the box on this axis
		float axisMin = ((__constant float*)&box->m_position)[axis] - ((__constant float*)&box->m_scale)[axis] * 0.5;
		float axisMax = axisMin + ((__constant float*)&box->m_scale)[axis];

		//if the ray is paralel with this axis
		if(fabs(((float*)&rayDir)[axis]) < 0.0001)
		{
			//if the ray isn't in the box, bail out we know there's no intersection
			if(((float*)&rayPos)[axis] < axisMin || ((float*)&rayPos)[axis] > axisMax)
				return false;
		}
		else
		{
			//figure out the intersection times of the ray with the 2 values of this axis
			float axisMinTime = (axisMin - ((float*)&rayPos)[axis]) / ((float*)&rayDir)[axis];
			float axisMaxTime = (axisMax - ((float*)&rayPos)[axis]) / ((float*)&rayDir)[axis];

			//make sure min < max
			if(axisMinTime > axisMaxTime)
			{
				float temp = axisMinTime;
				axisMinTime = axisMaxTime;
				axisMaxTime = temp;
			}

			//union this time slice with our running total time slice
			if(axisMinTime > rayMinTime)
				rayMinTime = axisMinTime;

			if(axisMaxTime < rayMaxTime)
				rayMaxTime = axisMaxTime;

			//if our time slice shrinks to below zero of a time window, we don't intersect
			if(rayMinTime > rayMaxTime)
				return false;
		}
	}
	
	//if we got here, we do intersect, return our collision info
	info->m_fromInside = (rayMinTime == 0.0);
	if(info->m_fromInside)
		info->m_intersectionTime = rayMaxTime;
	else
		info->m_intersectionTime = rayMinTime;
	info->m_materialIndex = box->m_materialIndex;
	info->m_portalIndex = box->m_portalIndex;
	
	info->m_intersectionPoint = rayPos + rayDir * info->m_intersectionTime;

	// figure out the surface normal by figuring out which axis we are closest to
	float closestDist = FLT_MAX;
	int closestAxis = 0;
	for(int axis = 0; axis < 3; ++axis)
	{
		float distFromPos= fabs(((__constant float*)&box->m_position)[axis] - ((float*)&info->m_intersectionPoint)[axis]);
		float distFromEdge = fabs(distFromPos - (((__constant float*)&box->m_scale)[axis] * 0.5f));

		if(distFromEdge < closestDist)
		{
			closestAxis = axis;
			closestDist = distFromEdge;
		}
	}

	float multiplier = 1.0f;
	info->m_surfaceNormal = (float3)( 0.0f, 0.0f, 0.0f);
	if(((float*)&info->m_intersectionPoint)[closestAxis] < ((__constant float*)&box->m_position)[closestAxis])
	{
		multiplier = -1.0f;
		((float*)&info->m_surfaceNormal)[closestAxis] = -1.0;
	}
	else
	{
		((float*)&info->m_surfaceNormal)[closestAxis] =  1.0;
	}

	// texture coordinates 
	float3 uaxis = {0.0,0.0,0.0};
	float3 vaxis = {0.0,0.0,0.0};
	
	if (closestAxis == 0)
	{
		uaxis.z = multiplier;
		vaxis.y = -1.0f;
	}
	else if (closestAxis == 1)
	{
		uaxis.z = multiplier * -1.0f;
		vaxis.x = -1.0f;
	}
	else
	{
		uaxis.x = multiplier * -1.0f;
		vaxis.y = -1.0f;
	}

	info->m_surfaceU = uaxis;
	info->m_surfaceV = vaxis;
	
	float3 relPoint = info->m_intersectionPoint - box->m_position;
	info->m_textureCoordinates.x = dot(relPoint, uaxis);
	info->m_textureCoordinates.y = dot(relPoint, vaxis);
	info->m_textureCoordinates *= box->m_textureScale;

	// we found a hit!
	info->m_objectHit = box->m_objectId;
	return true;	
}

bool RayIntersectPlane (__constant const struct SPlane *plane, struct SCollisionInfo *info, const float3 rayPos, const float3 rayDir, const TObjectId ignorePrimitiveId)
{
	if (ignorePrimitiveId == plane->m_objectId)
		return false;

	float denom = dot(plane->m_equation.xyz, rayDir);

	// ray is paralel.  Could do >= for "back face culling".  BFC would be a nice feature
	// except for objects that want transparency.
	if (denom == 0)
		return false;

	float num = -(dot(plane->m_equation.xyz, rayPos) + plane->m_equation.w);

	//t = - (n·org +D) / (n·dir)
	float collisionTime = num / denom;

	if (collisionTime < 0)
		return false;

	// enforce max distance
	if(collisionTime > info->m_intersectionTime)
		return false;

	// test our unscaled UV coordinates by the dims to make sure we hit the quad
	float3 intersectionPoint = rayPos + rayDir * collisionTime;

	float3 surfaceU = plane->m_UAxis;
	float3 surfaceV = normalize(cross(plane->m_UAxis, plane->m_equation.xyz));

	float2 textureCoordinates;
	textureCoordinates.x = dot(intersectionPoint, surfaceU);
	textureCoordinates.y = dot(intersectionPoint, surfaceV);

	if (textureCoordinates.x < plane->m_dims.x
	 || textureCoordinates.y < plane->m_dims.y
	 || textureCoordinates.x > plane->m_dims.z
	 || textureCoordinates.y > plane->m_dims.w)
		return false;

	// set all the info params since we are garaunteed a hit at this point
	info->m_materialIndex = plane->m_materialIndex;
	info->m_portalIndex = plane->m_portalIndex;

	// see if we are inside or not (in the negative half space)
	info->m_fromInside = denom > 0;

	//compute the point of intersection
	info->m_intersectionPoint = intersectionPoint;
	info->m_intersectionTime = collisionTime;

	// calculate the normal
	info->m_surfaceNormal = plane->m_equation.xyz;

	// calculate U and V
	info->m_surfaceU = surfaceU;
	info->m_surfaceV = surfaceV;

	// scaled texture coordinates
	info->m_textureCoordinates = textureCoordinates * plane->m_textureScale;

	// we found a hit!
	info->m_objectHit = plane->m_objectId;
	return true;	
}

bool RayIntersectSector (__constant const struct SSector *sector, struct SCollisionInfo *info, const float3 rayPos, const float3 rayDir, const TObjectId ignorePrimitiveId)
{
	float closestHitTime = info->m_intersectionTime;
	int closestHitPlaneIndex = SSECTOR_NUMPLANES;
	float3 closestHitSurfaceNormal;

	// test X axis slab if the ray isn't paralel with the x axis
	if (rayDir.x != 0.0f)
	{
		float denom = 1.0f / rayDir.x;

		float num1 = -rayPos.x + sector->m_halfDims.x;
		float num2 = -rayPos.x - sector->m_halfDims.x;

		float time1 = num1 * denom;
		float time2 = num2 * denom;

		if (time1 >= time2)
		{
			if (time1 > 0.0f && time1 < closestHitTime)
			{
				closestHitSurfaceNormal = (float3)(-1.0f, 0.0f, 0.0f);
				closestHitPlaneIndex = 0;
				closestHitTime = time1;
			}
		}
		else if (time2 > 0.0f && time2 < closestHitTime)
		{
			closestHitSurfaceNormal = (float3)(1.0f, 0.0f, 0.0f);
			closestHitPlaneIndex = 1;
			closestHitTime = time2;
		}
	}

	// test Y axis slab if the ray isn't paralel with the y axis
	if (rayDir.y != 0.0f)
	{
		float denom = 1.0f / rayDir.y;

		float num1 = -rayPos.y + sector->m_halfDims.y;
		float num2 = -rayPos.y - sector->m_halfDims.y;

		float time1 = num1 * denom;
		float time2 = num2 * denom;

		if (time1 >= time2)
		{
			if (time1 > 0.0f && time1 < closestHitTime)
			{
				closestHitSurfaceNormal = (float3)(0.0f, -1.0f, 0.0f);
				closestHitPlaneIndex = 2;
				closestHitTime = time1;
			}
		}
		else if (time2 > 0.0f && time2 < closestHitTime)
		{
			closestHitSurfaceNormal = (float3)(0.0f, 1.0f, 0.0f);
			closestHitPlaneIndex = 3;
			closestHitTime = time2;
		}
	}

	// test Z axis slab if the ray isn't paralel with the z axis
	if (rayDir.z != 0.0f)
	{
		float denom = 1.0f / rayDir.z;

		float num1 = -rayPos.z + sector->m_halfDims.z;
		float num2 = -rayPos.z - sector->m_halfDims.z;

		float time1 = num1 * denom;
		float time2 = num2 * denom;

		if (time1 >= time2)
		{
			if (time1 > 0.0f && time1 < closestHitTime)
			{
				closestHitSurfaceNormal = (float3)(0.0f, 0.0f, -1.0f);
				closestHitPlaneIndex = 4;
				closestHitTime = time1;
			}
		}
		else if (time2 > 0.0f && time2 < closestHitTime)
		{
			closestHitSurfaceNormal = (float3)(0.0f, 0.0f, 1.0f);
			closestHitPlaneIndex = 5;
			closestHitTime = time2;
		}
	}

	// if no planes hit, bail out
	if (closestHitPlaneIndex == SSECTOR_NUMPLANES)
		return false;

	// else we hit a sector wall, so set and calculate our collision info data
	info->m_intersectionTime = closestHitTime;

	//compute the point of intersection
	info->m_intersectionPoint = rayPos + rayDir * closestHitTime;

	// set the normal
	info->m_surfaceNormal = closestHitSurfaceNormal;

	// calculate U and V
	info->m_surfaceU = sector->m_planes[closestHitPlaneIndex].m_UAxis;
	info->m_surfaceV = normalize(cross(info->m_surfaceU, info->m_surfaceNormal));

	// unscaled texture coordinates
	info->m_textureCoordinates.x = dot(info->m_intersectionPoint, info->m_surfaceU);
	info->m_textureCoordinates.y = dot(info->m_intersectionPoint, info->m_surfaceV);
	
	// for sector planes, only set the portal index if the ray is in the portal window
	// this makes for more efficient portals when you can put the portal on a sector wall
	if (sector->m_planes[closestHitPlaneIndex].m_portalIndex != -1
	 && info->m_textureCoordinates.x >= sector->m_planes[closestHitPlaneIndex].m_portalWindow.x
	 && info->m_textureCoordinates.y >= sector->m_planes[closestHitPlaneIndex].m_portalWindow.y
	 && info->m_textureCoordinates.x <= sector->m_planes[closestHitPlaneIndex].m_portalWindow.z
	 && info->m_textureCoordinates.y <= sector->m_planes[closestHitPlaneIndex].m_portalWindow.w)
	{
		info->m_portalIndex = sector->m_planes[closestHitPlaneIndex].m_portalIndex;
	}
	else
	{
		info->m_portalIndex = -1;
	}

	// scale the texture coordinates
	info->m_textureCoordinates *= sector->m_planes[closestHitPlaneIndex].m_textureScale;

	info->m_fromInside = false;
	info->m_materialIndex = sector->m_planes[closestHitPlaneIndex].m_materialIndex;

	// we found a hit!
	info->m_objectHit = sector->m_planes[closestHitPlaneIndex].m_objectId;
	return true;
}

inline float3 reflect(float3 V, float3 N)
{
	return V - 2.0f * dot( V, N ) * N;
}

inline float3 refract(float3 V, float3 N, float refrIndex)
{
	float cosI = -dot( N, V );
	float cosT2 = 1.0f - refrIndex * refrIndex * (1.0f - cosI * cosI);
	return (refrIndex * V) + (refrIndex * cosI - sqrt( cosT2 )) * N;
}

bool PointCanSeePoint(
	const float3 startPos,
	const float3 targetPos,
	const TObjectId ignorePrimitiveId,
	int numSpheres,
	__constant struct SSphere *spheres,
	int numBoxes,
	__constant struct SAABox *boxes,
	int numPlanes,
	__constant struct SPlane *planes
)
{
	// see if we can hit the target point from the starting point
	struct SCollisionInfo collisionInfo = 
	{
		c_invalidObjectId,
		false,
		{ 0.0f, 0.0f, 0.0f },
		FLT_MAX,
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f },
		0,
		0,
	};
	
	float3 rayDir = targetPos - startPos;
	collisionInfo.m_intersectionTime = length(rayDir);
	rayDir = normalize(rayDir);

	for (int sphereIndex = 0; sphereIndex < numSpheres; ++sphereIndex) {
		if (spheres[sphereIndex].m_castsShadows
		 && RayIntersectSphere(&spheres[sphereIndex], &collisionInfo, startPos, rayDir, ignorePrimitiveId))
			return false;
	}

	for (int boxIndex = 0; boxIndex < numBoxes; ++boxIndex) {
		if (boxes[boxIndex].m_castsShadows
		 && RayIntersectAABox(&boxes[boxIndex], &collisionInfo, startPos, rayDir, ignorePrimitiveId))
			return false;
	}

	for (int planeIndex = 0; planeIndex < numPlanes; ++planeIndex) {
		if (planes[planeIndex].m_castsShadows
		 && RayIntersectPlane(&planes[planeIndex], &collisionInfo, startPos, rayDir, ignorePrimitiveId))
			return false;
	}

	// if no hit, set pixel to ambient light and bail out
	return true;
}

void ApplyPointLight (
	float3 *pixelColor,
	const struct SCollisionInfo *collisionInfo,
	__constant const struct SMaterial *material,
	__constant const struct SPointLight *light,
	const float reflectionAmount,
	const float3 rayDir,
	int numSpheres,
	__constant struct SSphere *spheres,
	int numBoxes,
	__constant struct SAABox *boxes,
	int numPlanes,
	__constant struct SPlane *planes,
	float3 diffuseColor
)
{
	if (!PointCanSeePoint(
		collisionInfo->m_intersectionPoint,
		light->m_position,
		collisionInfo->m_objectHit,
		numSpheres,
		spheres,
		numBoxes,
		boxes,
		numPlanes,
		planes)
	)
		return;

	// diffuse
	float3 hitToLight = normalize(light->m_position - collisionInfo->m_intersectionPoint);
	float dp = dot(collisionInfo->m_surfaceNormal, hitToLight);
	if(dp > 0.0)
		*pixelColor += diffuseColor * dp * light->m_color * reflectionAmount;
				
	// specular
	float3 reflection = reflect(hitToLight, collisionInfo->m_surfaceNormal);
	dp = dot(rayDir, reflection);
	if (dp > 0.0)
		*pixelColor += material->m_specularColorAndPower.xyz * pow(dp, material->m_specularColorAndPower.w) * light->m_color * reflectionAmount;
}

void TraceRay (
	__constant struct SSharedDataRoot *dataRoot,
	__read_only image3d_t tex3dIn,
	float3 rayPos,
	float3 rayDir,
	float3 *pixelColor,
	__constant struct SPointLight *lights,
	__constant struct SSphere *spheres,
	__constant struct SAABox *boxes,
	__constant struct SPlane *planes,
	__constant struct SSector *sectors,
	__constant struct SMaterial *materials,
	__constant struct SPortal *portals
)
{
	float3 ambientLight = dataRoot->m_world.m_ambientLight;
	TObjectId lastHitPrimitiveId = c_invalidObjectId;

	float colorMultiplier = 1.0f;

	float3 rayToCameraDir = rayDir;

	unsigned int currentSector = dataRoot->m_camera.m_sector;

	for(int index = 0; index < c_maxRayBounces; ++index)
	{
		struct SCollisionInfo collisionInfo = 
		{
			c_invalidObjectId,
			false,
			{ 0.0f, 0.0f, 0.0f },
			FLT_MAX,
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f },
			0,
			0,
		};

		for (int sphereIndex = 0; sphereIndex < dataRoot->m_world.m_numSpheres; ++sphereIndex)
			RayIntersectSphere(&spheres[sphereIndex], &collisionInfo, rayPos, rayDir, lastHitPrimitiveId);

		for (int boxIndex = 0; boxIndex < dataRoot->m_world.m_numBoxes; ++boxIndex)
			RayIntersectAABox(&boxes[boxIndex], &collisionInfo, rayPos, rayDir, lastHitPrimitiveId);

		for (int planeIndex = 0; planeIndex < dataRoot->m_world.m_numPlanes; ++planeIndex)
			RayIntersectPlane(&planes[planeIndex], &collisionInfo, rayPos, rayDir, lastHitPrimitiveId);

		// test the rays against the current world sector if it's valid
		if (currentSector < dataRoot->m_world.m_numSectors)
			RayIntersectSector(&sectors[currentSector], &collisionInfo, rayPos, rayDir, lastHitPrimitiveId);

		// if no hit, set pixel to ambient light and bail out
		if (collisionInfo.m_objectHit == c_invalidObjectId)
		{
			*pixelColor += ambientLight * colorMultiplier;
			return;
		}

		// if we hit a portal, change our sector, transform the ray and bail out of this loop.
		if (collisionInfo.m_portalIndex != -1)
		{
			// transform the collision point into sector space
			float3 transformedPoint;
			transformedPoint.x = collisionInfo.m_intersectionPoint.x * portals[collisionInfo.m_portalIndex].m_xaxis.x
				               + collisionInfo.m_intersectionPoint.y * portals[collisionInfo.m_portalIndex].m_yaxis.x
							   + collisionInfo.m_intersectionPoint.z * portals[collisionInfo.m_portalIndex].m_zaxis.x
							   +                                1.0f * portals[collisionInfo.m_portalIndex].m_waxis.x;

			transformedPoint.y = collisionInfo.m_intersectionPoint.x * portals[collisionInfo.m_portalIndex].m_xaxis.y
				               + collisionInfo.m_intersectionPoint.y * portals[collisionInfo.m_portalIndex].m_yaxis.y
							   + collisionInfo.m_intersectionPoint.z * portals[collisionInfo.m_portalIndex].m_zaxis.y
							   +                                1.0f * portals[collisionInfo.m_portalIndex].m_waxis.y;

			transformedPoint.z = collisionInfo.m_intersectionPoint.x * portals[collisionInfo.m_portalIndex].m_xaxis.z
				               + collisionInfo.m_intersectionPoint.y * portals[collisionInfo.m_portalIndex].m_yaxis.z
							   + collisionInfo.m_intersectionPoint.z * portals[collisionInfo.m_portalIndex].m_zaxis.z
							   +                                1.0f * portals[collisionInfo.m_portalIndex].m_waxis.z;

			// transform the ray direction into sector space
			float3 transformedDir;
			transformedDir.x = rayDir.x * portals[collisionInfo.m_portalIndex].m_xaxis.x
				             + rayDir.y * portals[collisionInfo.m_portalIndex].m_yaxis.x
							 + rayDir.z * portals[collisionInfo.m_portalIndex].m_zaxis.x;
						   //+     0.0f * portals[collisionInfo.m_portalIndex].m_waxis.x;

			transformedDir.y = rayDir.x * portals[collisionInfo.m_portalIndex].m_xaxis.y
				             + rayDir.y * portals[collisionInfo.m_portalIndex].m_yaxis.y
							 + rayDir.z * portals[collisionInfo.m_portalIndex].m_zaxis.y;
						   //+     0.0f * portals[collisionInfo.m_portalIndex].m_waxis.y;

			transformedDir.z = rayDir.x * portals[collisionInfo.m_portalIndex].m_xaxis.z
				             + rayDir.y * portals[collisionInfo.m_portalIndex].m_yaxis.z
							 + rayDir.z * portals[collisionInfo.m_portalIndex].m_zaxis.z;
						   //+     0.0f * portals[collisionInfo.m_portalIndex].m_waxis.z;

			rayPos = transformedPoint;
			rayDir = normalize(transformedDir);
			currentSector = portals[collisionInfo.m_portalIndex].m_sector;
			lastHitPrimitiveId = collisionInfo.m_objectHit;
			continue;
		}

		__constant const struct SMaterial *material = &materials[collisionInfo.m_materialIndex];

		if (collisionInfo.m_fromInside)
			collisionInfo.m_surfaceNormal *= -1.0f;

		// handle normal mapping if there is any
		if (material->m_normalTextureIndex >= 0)
		{
			float4 textureCoords = {collisionInfo.m_textureCoordinates.x, collisionInfo.m_textureCoordinates.y, material->m_normalTextureIndex, 0};
			float3 textureNormal = read_imagef(tex3dIn, g_textureSampler, textureCoords).xyz;

			textureNormal = normalize(textureNormal * 2.0 - 1.0);

			float3 adjustedNormal;
			adjustedNormal.x = textureNormal.x * collisionInfo.m_surfaceU.x + textureNormal.y * collisionInfo.m_surfaceV.x + textureNormal.z * collisionInfo.m_surfaceNormal.x;
			adjustedNormal.y = textureNormal.x * collisionInfo.m_surfaceU.y + textureNormal.y * collisionInfo.m_surfaceV.y + textureNormal.z * collisionInfo.m_surfaceNormal.y;
			adjustedNormal.z = textureNormal.x * collisionInfo.m_surfaceU.z + textureNormal.y * collisionInfo.m_surfaceV.z + textureNormal.z * collisionInfo.m_surfaceNormal.z;

			collisionInfo.m_surfaceNormal = normalize(adjustedNormal);
		}

		// get the diffuse color of the object we hit
		float3 diffuseColorBase = material->m_diffuseColor;
		if (material->m_diffuseTextureIndex >= 0)
		{
			float4 textureCoords = {collisionInfo.m_textureCoordinates.x, collisionInfo.m_textureCoordinates.y, material->m_diffuseTextureIndex, 0};
			diffuseColorBase *= read_imagef(tex3dIn, g_textureSampler, textureCoords).xyz;
		}

		// get the emissive color of the object we hit
		float3 emissiveColor = material->m_emissiveColor;
		if (material->m_emissiveTextureIndex >= 0)
		{
			float4 textureCoords = {collisionInfo.m_textureCoordinates.x, collisionInfo.m_textureCoordinates.y, material->m_emissiveTextureIndex, 0};
			emissiveColor *= read_imagef(tex3dIn, g_textureSampler, textureCoords).xyz;
		}

		// apply ambient lighting and emissive color
		float3 diffuseColor = diffuseColorBase * ambientLight + emissiveColor;

		// apply diffuse / specular from a point light
		for (int lightIndex = 0; lightIndex < dataRoot->m_world.m_numLights; ++lightIndex)
			ApplyPointLight(
				&diffuseColor,
				&collisionInfo,
				material,
				&lights[lightIndex],
				colorMultiplier,
				rayDir,
				dataRoot->m_world.m_numSpheres,
				spheres,
				dataRoot->m_world.m_numBoxes,
				boxes,
				dataRoot->m_world.m_numPlanes,
				planes,
				diffuseColorBase
			);

		// add the color in
		*pixelColor += diffuseColor * colorMultiplier;

		// if reflective, set up the reflected ray
		if (material->m_reflectionAmount > 0.0f)
		{
			rayPos = collisionInfo.m_intersectionPoint;
			rayDir = reflect(rayDir, collisionInfo.m_surfaceNormal);

			lastHitPrimitiveId = collisionInfo.m_objectHit;
			colorMultiplier *= material->m_reflectionAmount;
		}
		// if refractive, set up the refracted ray
		else if (material->m_refractionAmount > 0.0f)
		{				
			// if we are entering a refractive object, we can't ignore it since we need to go out the back
			// side possibly.  Since we can't ignore it, we need to push a little bit past the point of
			// intersection so we don't intersect it again.
			lastHitPrimitiveId = 0;				
			rayPos = collisionInfo.m_intersectionPoint + rayDir * 0.001f;
				
			rayDir = refract(rayToCameraDir, collisionInfo.m_surfaceNormal, material->m_refractionIndex);
				
			colorMultiplier *= material->m_refractionAmount;
		}
		// else we are done
		else
			return;
	}
}

__kernel void clrt (
	__write_only image2d_t texOut, 
	__read_only image3d_t tex3dIn,
	__constant struct SSharedDataRoot *dataRoot,
	__constant struct SPointLight *lights,
	__constant struct SSphere *spheres,
	__constant struct SAABox *boxes,
	__constant struct SPlane *planes,
	__constant struct SSector *sectors,
	__constant struct SMaterial *materials,
	__constant struct SPortal *portals
)
{
    const int2 dims = (int2)(get_image_width(texOut), get_image_height(texOut));
	const int2 coord = (int2)(get_global_id(0), get_global_id(1));

	#if SETTINGS_INTERLACED == 1
	if ((coord.y / 16) % 2 == dataRoot->m_camera.m_oddEven)
		return;
	#endif

    // in the case where, due to quantization into grids, we have
    // more threads than pixels, skip the threads which don't 
    // correspond to valid pixels
	if (coord.x >= dims.x || coord.y >= dims.y)
		return;

	// calculate the ray direction
	const float2 percent = (float2)(((float)coord.x / (float)dims.x) - 0.5f, ((float)coord.y / (float)dims.y) - 0.5f);
	float3 rayDir = normalize((dataRoot->m_camera.m_fwd * dataRoot->m_camera.m_viewWidthHeightDistance.z)
		- (dataRoot->m_camera.m_left * percent.x * dataRoot->m_camera.m_viewWidthHeightDistance.x)
		- (dataRoot->m_camera.m_up * percent.y * dataRoot->m_camera.m_viewWidthHeightDistance.y));

	// trace the ray
	float3 color = (float3)(0);
	TraceRay(dataRoot, tex3dIn, dataRoot->m_camera.m_pos, rayDir, &color, lights, spheres, boxes, planes, sectors, materials, portals);
	write_imagef(texOut, coord, (float4)(color, 1.0)); 
}
