/*==================================================================================================

clrt.cl

The kernel code

==================================================================================================*/

#include "Shared/SSharedDataRoot.h"
#include "Shared/SharedGeometry.h"

#define c_maxRayBounces 6

struct SCollisionInfo
{
	TObjectId			m_objectHit;
	bool 				m_fromInside;
	float3				m_intersectionPoint;
	float				m_intersectionTime;
	float3				m_surfaceNormal;
	unsigned int		m_materialIndex;
};

bool RayIntersectSphere (__constant const struct SSphere *sphere, struct SCollisionInfo *info, const float3 rayPos, const float3 rayDir, const TObjectId ignorePrimitiveId)
{
	if (ignorePrimitiveId == sphere->m_objectId)
		return false;

	//get the vector from the center of this circle to where the ray begins.
	float3 m = rayPos - sphere->m_positionAndRadius.xyz;

    //get the dot product of the above vector and the ray's vector
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

	//compute the point of intersection
	info->m_intersectionPoint = rayPos + rayDir * collisionTime;
	info->m_intersectionTime = collisionTime;

	// calculate the normal
	info->m_surfaceNormal = info->m_intersectionPoint - sphere->m_positionAndRadius.xyz;
	info->m_surfaceNormal = normalize(info->m_surfaceNormal);

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
	
	info->m_intersectionPoint = rayPos + rayDir * info->m_intersectionTime;

	// figure out the surface normal by figuring out which axis we are closest to
	float closestDist = FLT_MAX;
	for(int axis = 0; axis < 3; ++axis)
	{
		float distFromPos= fabs(((__constant float*)&box->m_position)[axis] - ((float*)&info->m_intersectionPoint)[axis]);
		float distFromEdge = fabs(distFromPos - (((__constant float*)&box->m_scale)[axis] * 0.5f));

		if(distFromEdge < closestDist)
		{
			closestDist = distFromEdge;
			info->m_surfaceNormal = (float3)( 0.0f, 0.0f, 0.0f);
			if(((float*)&info->m_intersectionPoint)[axis] < ((__constant float*)&box->m_position)[axis])
				((float*)&info->m_surfaceNormal)[axis] = -1.0;
			else
				((float*)&info->m_surfaceNormal)[axis] =  1.0;
		}
	}

	// we found a hit!
	info->m_objectHit = box->m_objectId;
	return true;	
}

float3 reflect(float3 V, float3 N)
{
	return V - 2.0f * dot( V, N ) * N;
}

float3 refract(float3 V, float3 N, float refrIndex)
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
	__constant struct SAABox *boxes
)
{
	// see if we can hit the target point from the starting point
	struct SCollisionInfo collisionInfo = 
	{
		c_invalidObjectId,
		false,
		{ 0.0f, 0.0f, 0.0f},
		FLT_MAX,
		{ 0.0f, 0.0f, 0.0f },
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
	__constant struct SAABox *boxes
)
{
	if (!PointCanSeePoint(
		collisionInfo->m_intersectionPoint,
		light->m_position,
		collisionInfo->m_objectHit,
		numSpheres,
		spheres,
		numBoxes,
		boxes)
	)
		return;

	// diffuse
	float3 hitToLight = normalize(light->m_position - collisionInfo->m_intersectionPoint);
	float dp = dot(collisionInfo->m_surfaceNormal, hitToLight);
	if(dp > 0.0)
		*pixelColor += material->m_diffuseColorAndAmount.xyz * dp * light->m_color * material->m_diffuseColorAndAmount.w * reflectionAmount;
				
	// specular
	float3 reflection = reflect(hitToLight, collisionInfo->m_surfaceNormal);
	dp = dot(rayDir, reflection);
	if (dp > 0.0)
		*pixelColor += material->m_specularColorAndAmount.xyz * pow(dp, material->m_specularColorAndAmount.w) * light->m_color * reflectionAmount;
}

void TraceRay (
	__constant struct SSharedDataRoot *dataRoot,
	float3 rayPos,
	float3 rayDir,
	float3 *pixelColor,
	__constant struct SPointLight *lights,
	__constant struct SSphere *spheres,
	__constant struct SAABox *boxes,
	__constant struct SMaterial *materials
)
{
	float3 ambientLight = dataRoot->m_world.m_ambientLight;
	TObjectId lastHitPrimitiveId = c_invalidObjectId;

	float colorMultiplier = 1.0f;

	float3 rayToCameraDir = rayDir;

	for(int index = 0; index < c_maxRayBounces; ++index)
	{
		struct SCollisionInfo collisionInfo = 
		{
			c_invalidObjectId,
			false,
			{ 0.0f, 0.0f, 0.0f},
			FLT_MAX,
			{ 0.0f, 0.0f, 0.0f },
			0,
		};

		for (int sphereIndex = 0; sphereIndex < dataRoot->m_world.m_numSpheres; ++sphereIndex)
			RayIntersectSphere(&spheres[sphereIndex], &collisionInfo, rayPos, rayDir, lastHitPrimitiveId);

		for (int boxIndex = 0; boxIndex < dataRoot->m_world.m_numBoxes; ++boxIndex)
			RayIntersectAABox(&boxes[boxIndex], &collisionInfo, rayPos, rayDir, lastHitPrimitiveId);

		// if no hit, set pixel to ambient light and bail out
		if (collisionInfo.m_objectHit == c_invalidObjectId)
		{
			*pixelColor += ambientLight * colorMultiplier;
			return;
		}

		__constant const struct SMaterial *material = &materials[collisionInfo.m_materialIndex];

		// get the diffuse color of the object we hit
		float3 diffuseColor = material->m_diffuseColorAndAmount.xyz * material->m_diffuseColorAndAmount.w;

		// apply ambient lighting and emissive color
		diffuseColor = diffuseColor * ambientLight + material->m_emissiveColor;

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
				boxes
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
			if (collisionInfo.m_fromInside)
				collisionInfo.m_surfaceNormal *= -1.0f;
				
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
	__constant struct SSharedDataRoot *dataRoot,
	__constant struct SPointLight *lights,
	__constant struct SSphere *spheres,
	__constant struct SAABox *boxes,
	__constant struct SMaterial *materials
)
{
    const int2 dims = (int2)(get_image_width(texOut), get_image_height(texOut));
	const int2 coord = (int2)(get_global_id(0), get_global_id(1));

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
	TraceRay(dataRoot, dataRoot->m_camera.m_pos, rayDir, &color, lights, spheres, boxes, materials);
	write_imagef(texOut, coord, (float4)(color, 1.0)); 
}
