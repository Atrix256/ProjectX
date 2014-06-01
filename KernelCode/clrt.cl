/*==================================================================================================

clrt.cl

The kernel code

==================================================================================================*/

#include "Shared/SSharedDataRoot.h"
#include "Shared/SharedGeometry.h"
#include "KernelMath.h"

#define c_maxRayBounces SETTINGS_RAYBOUNCES
#define c_maxRayLength 1000.0f

#if SETTINGS_TEXTUREFILTER == 1
const sampler_t g_textureSampler = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_REPEAT | CLK_FILTER_LINEAR;
#else
const sampler_t g_textureSampler = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_REPEAT | CLK_FILTER_NEAREST;
#endif

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
	float3				m_debugAdditiveColor; // for debugging!
	unsigned int		m_materialIndex;
	unsigned int		m_portalIndex;
};

inline bool IsReflective (__global const struct SMaterial *material)
{
	return material->m_reflectionAmount > 0.0f;
}

inline bool IsRefractive (__global const struct SMaterial *material)
{
	return !IsReflective(material) && material->m_refractionAmount > 0.0f;
}

inline bool RayHitsSphere(const float4 sphere, const float3 rayPos, const float3 rayDir, float3 *sphereStartPoint, float3 *sphereEndPoint)
{
	// get the vector from the center of this circle to where the ray begins.
	float3 m = rayPos - sphere.xyz;

    // get the dot product of the above vector and the ray's vector
	float b = dot(m, rayDir);

	float c = dot(m, m) - sphere.w * sphere.w;

	//exit if r's origin outside s (c > 0) and r pointing away from s (b > 0)
	if(c > 0.0 && b > 0.0)
		return false;

	//calculate discriminant
	float discr = b * b - c;

	//a negative discriminant corresponds to ray missing sphere
	if(discr < 0.0)
		return false;

	//ray now found to intersect sphere, compute smallest t value of intersection
	float collisionTime = -b - sqrt(discr);

	//if t is negative, ray started inside sphere so use that as the sphere start point, else use the place we hit the sphere
	*sphereStartPoint = (collisionTime < 0.0) ? rayPos : rayPos + rayDir * collisionTime;

	// the sphere end point is the other side of the sphere hit
	collisionTime = -b + sqrt(discr);
	*sphereEndPoint = rayPos + rayDir * collisionTime;

	return true;
}

bool RayIntersectSphere (__global const struct SSphere *sphere, struct SCollisionInfo *info, const float3 rayPos, const float3 rayDir, const TObjectId ignorePrimitiveId)
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
	info->m_textureCoordinates += sphere->m_textureOffset;

	// we found a hit!
	info->m_objectHit = sphere->m_objectId;
	return true;
}

inline bool RayIntersectTriangle (__global const struct SModelTriangle *triangle, struct SCollisionInfo *info, const float3 rayPos, const float3 rayDir, const TObjectId ignorePrimitiveId, bool backFaceCulling, cl_uint materialIndex, cl_uint portalIndex)
{
	if (ignorePrimitiveId == triangle->m_objectId)
		return false;

	// do back face culling if we are allowed.  It seems to make no impact on performance from what i can tell though unfortunately ):
	if (backFaceCulling && dot(rayDir, triangle->m_plane.xyz) > 0.0f)
		return false;

	// distance of p (start point) and q (some other point) to triangle plane
	// could do backface culling here with distp and distq (check book if you want to do that later!) 
	float distp = dot(rayPos, triangle->m_plane.xyz) - triangle->m_plane.w;
	float distq = dot((rayPos + rayDir), triangle->m_plane.xyz) - triangle->m_plane.w;

	// calculate t value of impact
	float denom = distp - distq;
	float t = distp / denom;

	// enforce min and max distance
	if(t < 0 || t > info->m_intersectionTime)
		return false;

	// calculate point of impact s
	float3 s = rayPos + t * rayDir;

	// calculate barycentric coordinate u, exit if outside of 0-1
	float u = dot(s, triangle->m_planeBC.xyz) - triangle->m_planeBC.w;
	if (u < 0.0f || u > 1.0f)
		return false;

	// calculate barycentric coordinate u, exit if negative
	float v = dot(s, triangle->m_planeCA.xyz) - triangle->m_planeCA.w;
	if (v < 0.0f)
		return false;

	// calculate w, exit if negative
	float w = 1.0f - u - v;
	if (w < 0.0f)
		return false;

	// set all the info params since we are garaunteed a hit at this point 
	info->m_materialIndex = materialIndex;
	info->m_portalIndex = portalIndex;

	//compute the point of intersection
	info->m_intersectionPoint = rayPos + rayDir * t;
	info->m_intersectionTime = t;

	// calculate the normal
	info->m_surfaceNormal = triangle->m_plane.xyz;
	info->m_fromInside = dot(rayDir, info->m_surfaceNormal) > 0;

	// set the tangent and bitangent
	info->m_surfaceU = triangle->m_tangent;
	info->m_surfaceV = triangle->m_bitangent;

	// texture coordinates - get from texture coordinates on triangle
	info->m_textureCoordinates = triangle->m_textureA * u + triangle->m_textureB * v + triangle->m_textureC * w;

	#if DEBUG_TRIANGLES
	if (u < 0.025f)
		info->m_debugAdditiveColor += (float3)(0.3f,0.0f,0.0f);
	if (v < 0.025f)
		info->m_debugAdditiveColor += (float3)(0.0f,0.3f,0.0f);
	if (w < 0.025f)
		info->m_debugAdditiveColor += (float3)(0.0f,0.0f,0.3f);
	#endif

	// barycentric coordinates debugging
	//const float factor = 0.25f;
	//info->m_debugAdditiveColor += (float3)(u*factor,v*factor,w*factor);

	// we found a hit!
	info->m_objectHit = triangle->m_objectId;
	return true;
}

bool RayIntersectSector (__global const struct SSector *sector, struct SCollisionInfo *info, const float3 rayPos, const float3 rayDir, const TObjectId ignorePrimitiveId)
{
	float closestHitTime = info->m_intersectionTime;
	int closestHitPlaneIndex = SSECTOR_NUMPLANES;
	float3 closestHitSurfaceNormal;
	float3 closestHitSurfaceU;
	float3 closestHitSurfaceV;

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
				closestHitSurfaceNormal = GetSectorPlaneNormal(0);
				closestHitSurfaceU = GetSectorPlaneU(0);
				closestHitSurfaceV = GetSectorPlaneV(0);
				closestHitPlaneIndex = 0;
				closestHitTime = time1;
			}
		}
		else if (time2 > 0.0f && time2 < closestHitTime)
		{
			closestHitSurfaceNormal = GetSectorPlaneNormal(1);
			closestHitSurfaceU = GetSectorPlaneU(1);
			closestHitSurfaceV = GetSectorPlaneV(1);
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
				closestHitSurfaceNormal = GetSectorPlaneNormal(2);
				closestHitSurfaceU = GetSectorPlaneU(2);
				closestHitSurfaceV = GetSectorPlaneV(2);
				closestHitPlaneIndex = 2;
				closestHitTime = time1;
			}
		}
		else if (time2 > 0.0f && time2 < closestHitTime)
		{
			closestHitSurfaceNormal = GetSectorPlaneNormal(3);
			closestHitSurfaceU = GetSectorPlaneU(3);
			closestHitSurfaceV = GetSectorPlaneV(3);
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
				closestHitSurfaceNormal = GetSectorPlaneNormal(4);
				closestHitSurfaceU = GetSectorPlaneU(4); 
				closestHitSurfaceV = GetSectorPlaneV(4);
				closestHitPlaneIndex = 4;
				closestHitTime = time1;
			}
		}
		else if (time2 > 0.0f && time2 < closestHitTime)
		{
			closestHitSurfaceNormal = GetSectorPlaneNormal(5);
			closestHitSurfaceU = GetSectorPlaneU(5);
			closestHitSurfaceV = GetSectorPlaneV(5);
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
	float portalU = dot(info->m_intersectionPoint, closestHitSurfaceU);
	float portalV = dot(info->m_intersectionPoint, closestHitSurfaceV);
	if (sector->m_planes[closestHitPlaneIndex].m_portalIndex != -1
	 && portalU >= sector->m_planes[closestHitPlaneIndex].m_portalWindow.x
	 && portalV >= sector->m_planes[closestHitPlaneIndex].m_portalWindow.y
	 && portalU <= sector->m_planes[closestHitPlaneIndex].m_portalWindow.z
	 && portalV <= sector->m_planes[closestHitPlaneIndex].m_portalWindow.w)
	{
		info->m_portalIndex = sector->m_planes[closestHitPlaneIndex].m_portalIndex;
	}
	else
	{
		info->m_portalIndex = -1;
	}

	// scale the texture coordinates
	info->m_textureCoordinates *= sector->m_planes[closestHitPlaneIndex].m_textureScale;
	info->m_textureCoordinates += sector->m_planes[closestHitPlaneIndex].m_textureOffset;

	info->m_fromInside = false;
	info->m_materialIndex = sector->m_planes[closestHitPlaneIndex].m_materialIndex;

	// we found a hit!
	info->m_objectHit = sector->m_planes[closestHitPlaneIndex].m_objectId;
	return true;
}

inline bool PointCanSeePoint(
	const float3 startPos,
	const float3 targetPos,
	const TObjectId ignorePrimitiveId,
	__global const struct SSector *sector,
	__global struct SSphere *spheres,
	__global struct SModelTriangle *triangles,
	__global struct SModelObject *objects,
	__global struct SModelInstance *models,
	__global struct SMaterial *materials
)
{
	#if SETTINGS_SHADOWS == 1
	// see if we can hit the target point from the starting point
	struct SCollisionInfo collisionInfo = 
	{
		c_invalidObjectId,
		false,
		{ 0.0f, 0.0f, 0.0f },
		c_maxRayLength,
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		0,
		0,
	};
	
	float3 rayDir = targetPos - startPos;
	collisionInfo.m_intersectionTime = length(rayDir);
	rayDir = normalize(rayDir);

	for (int index = sector->m_staticSphereStartIndex; index < sector->m_staticSphereStopIndex; ++index)
	{
		if (spheres[index].m_castsShadows
		 && RayIntersectSphere(&spheres[index], &collisionInfo, startPos, rayDir, ignorePrimitiveId))
			return false;
	}

	for (int modelIndex = sector->m_staticModelStartIndex; modelIndex < sector->m_staticModelStopIndex; ++modelIndex)
	{
		__global struct SModelInstance *model = &models[modelIndex];
		float3 hitStart, hitEnd;
		if (RayHitsSphere(model->m_boundingSphere, startPos, rayDir, &hitStart, &hitEnd))
		{
			struct SCollisionInfo collisionInfoLocal = 
			{
				c_invalidObjectId,
				false,
				{ 0.0f, 0.0f, 0.0f },
				c_maxRayLength,
				{ 0.0f, 0.0f, 0.0f },
				{ 0.0f, 0.0f, 0.0f },
				{ 0.0f, 0.0f, 0.0f },
				{ 0.0f, 0.0f },
				collisionInfo.m_debugAdditiveColor,
				0,
				0,
			};

			// convert max intersection time from world to local space
			if (collisionInfo.m_objectHit != c_invalidObjectId)
				collisionInfoLocal.m_intersectionTime = collisionInfo.m_intersectionTime / model->m_scale;

			// convert the ray from world space to model space, making sure the ray direction is normalized to account for scaling or rounding errors
			float3 startPosLocal;
			float3 rayDirLocal;
			TransformPointByMatrix(&startPosLocal, &startPos, &model->m_worldToModelX, &model->m_worldToModelY, &model->m_worldToModelZ, &model->m_worldToModelW);
			TransformVectorByMatrix(&rayDirLocal, &rayDir, &model->m_worldToModelX, &model->m_worldToModelY, &model->m_worldToModelZ);
			rayDirLocal = normalize(rayDirLocal);

			// convert the bounding sphere hit locations from world space to model space
			float3 hitStartLocal;
			float3 hitEndLocal;
			TransformPointByMatrix(&hitStartLocal, &hitStart, &model->m_worldToModelX, &model->m_worldToModelY, &model->m_worldToModelZ, &model->m_worldToModelW);
			TransformPointByMatrix(&hitEndLocal, &hitEnd, &model->m_worldToModelX, &model->m_worldToModelY, &model->m_worldToModelZ, &model->m_worldToModelW);

			// calculate which y half spaces this segment goes through
			cl_uint halfSpaceFlags = 0;
			halfSpaceFlags |= hitStartLocal.y > 0.0f ? e_halfSpacePosY : e_halfSpaceNegY;
			halfSpaceFlags |= hitEndLocal.y > 0.0f ? e_halfSpacePosY : e_halfSpaceNegY;

			for (int objectIndex = model->m_startObjectIndex; objectIndex < model->m_stopObjectIndex; ++objectIndex)
			{
				__global struct SModelObject *object = &objects[objectIndex];
				unsigned int materialIndex = model->m_materialOverride == -1 ? object->m_materialIndex : model->m_materialOverride;
				bool backFaceCulling = !IsRefractive(&materials[materialIndex]);
				if (object->m_castsShadows)
				{
					// figure out the triangle start and stop index to test against.
					// if the segment we are testing is in only the positive y half space or only the negative y half space, we can cut out triangles
					// that are completely in the other y half space.  If it has both, we need to test all unfortunately.
					unsigned int triangleIndex = (halfSpaceFlags & e_halfSpaceNegY) ? object->m_startTriangleIndex : object->m_mixStartTriangleIndex;
					unsigned int triangleStopIndex = (halfSpaceFlags & e_halfSpacePosY) ? object->m_stopTriangleIndex : object->m_mixStopTriangleIndex;

					for (; triangleIndex < triangleStopIndex; ++triangleIndex)
					{
						if (RayIntersectTriangle(&triangles[triangleIndex], &collisionInfoLocal, startPosLocal, rayDirLocal, ignorePrimitiveId, backFaceCulling, object->m_materialIndex, object->m_portalIndex))
							return false;
					}
				}
			}
		}
	}

	#endif

	// if no hit, bail out
	return true;
}

void ApplyPointLight (
	float3 *pixelColor,
	const struct SCollisionInfo *collisionInfo,
	__global const struct SSector *sector,
	__global const struct SMaterial *material,
	__global const struct SPointLight *light,
	const float3 colorMultiplier,
	const float3 rayDir,
	__global struct SSphere *spheres,
	__global struct SModelTriangle *triangles,
	__global struct SModelObject *objects,
	__global struct SModelInstance *models,
	__global struct SMaterial *materials,
	float3 diffuseColor
)
{
	float3 hitToLight = normalize(light->m_position - collisionInfo->m_intersectionPoint);

	float coneAngle = dot(light->m_spotLightReverseDir, hitToLight);
	if (coneAngle <= light->m_spotLightcosPhiOver2)
		return;

	if (!PointCanSeePoint(
		collisionInfo->m_intersectionPoint,
		light->m_position,
		collisionInfo->m_objectHit,
		sector,
		spheres,
		triangles,
		objects,
		models,
		materials
		)
	)
		return;

	// light attenuation for high quality lights
	float attenuation = 1.0f;
	#if SETTINGS_HIQLIGHTS == 1
		// spot light attenuation
		if (light->m_spotLightFalloffFactor != 0 && coneAngle < light->m_spotLightcosThetaOver2)
			attenuation *= pow((coneAngle - light->m_spotLightcosPhiOver2) / (light->m_spotLightcosThetaOver2 - light->m_spotLightcosPhiOver2), light->m_spotLightFalloffFactor);

		// distance attenuation
		if (light->m_attenuationConstDistDistsq.x != 1.0f ||
			light->m_attenuationConstDistDistsq.y != 0.0f ||
			light->m_attenuationConstDistDistsq.z != 0.0f)
		{
			float distanceToLight = length(light->m_position - collisionInfo->m_intersectionPoint);
			attenuation /= (
				light->m_attenuationConstDistDistsq.x +
				distanceToLight * light->m_attenuationConstDistDistsq.y + 
				distanceToLight * distanceToLight * light->m_attenuationConstDistDistsq.z);
		}
	#endif

	// diffuse
	float dp = dot(collisionInfo->m_surfaceNormal, hitToLight);
	if(dp > 0.0)
		*pixelColor += diffuseColor * dp * light->m_color * colorMultiplier * attenuation;
				
	// specular
	float3 reflection = reflect(hitToLight, collisionInfo->m_surfaceNormal);
	dp = dot(rayDir, reflection);
	if (dp > 0.0)
		*pixelColor += material->m_specularColorAndPower.xyz * pow(dp, material->m_specularColorAndPower.w) * light->m_color * colorMultiplier * attenuation;
}

//
void TraceRay (
	__global struct SSharedDataRootHostToKernel *dataRoot,
	__read_only image3d_t tex3dIn,
	float3 rayPos,
	float3 rayDir,
	float3 *pixelColor,
	__global struct SPointLight *lights,
	__global struct SSphere *spheres,
	__global struct SModelTriangle *triangles,
	__global struct SModelObject *objects,
	__global struct SModelInstance *models,
	__global struct SSector *sectors,
	__global struct SMaterial *materials,
	__global struct SPortal *portals
)
{
	TObjectId lastHitPrimitiveId = c_invalidObjectId;

	float3 colorMultiplier = {1.0f, 1.0f, 1.0f};

	float3 absorbance = {0.0f, 0.0f, 0.0f};

	float3 rayToCameraDir = rayDir;

	unsigned int currentSector = dataRoot->m_camera.m_sector;

	for(int index = 0; index < c_maxRayBounces && currentSector != -1; ++index)
	{
		struct SCollisionInfo collisionInfo = 
		{
			c_invalidObjectId,
			false,
			{ 0.0f, 0.0f, 0.0f },
			c_maxRayLength,
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f },
			#if DEBUG_RAY_BOUNCECOUNT
			{ 1.0f / ((float)c_maxRayBounces), 1.0f / ((float)c_maxRayBounces), 1.0f / ((float)c_maxRayBounces) },
			#else
			{ 0.0f, 0.0f, 0.0f },
			#endif
			0,
			0,
		};

		__global const struct SSector *sector = &sectors[currentSector];

		const float3 ambientLight = sector->m_ambientLight;

		for (int index = sector->m_staticSphereStartIndex; index < sector->m_staticSphereStopIndex; ++index)
			RayIntersectSphere(&spheres[index], &collisionInfo, rayPos, rayDir, lastHitPrimitiveId);

		for (int modelIndex = sector->m_staticModelStartIndex; modelIndex < sector->m_staticModelStopIndex; ++modelIndex)
		{
			__global struct SModelInstance *model = &models[modelIndex];
			float3 hitStart, hitEnd;
			if (RayHitsSphere(model->m_boundingSphere, rayPos, rayDir, &hitStart, &hitEnd))
			{
				struct SCollisionInfo collisionInfoLocal = 
				{
					c_invalidObjectId,
					false,
					{ 0.0f, 0.0f, 0.0f },
					c_maxRayLength,
					{ 0.0f, 0.0f, 0.0f },
					{ 0.0f, 0.0f, 0.0f },
					{ 0.0f, 0.0f, 0.0f },
					{ 0.0f, 0.0f },
					collisionInfo.m_debugAdditiveColor,
					0,
					0,
				};

				// convert max intersection time from world to local space
				if (collisionInfo.m_objectHit != c_invalidObjectId)
					collisionInfoLocal.m_intersectionTime = collisionInfo.m_intersectionTime / model->m_scale;

				// convert the ray from world space to model space, making sure the ray direction is normalized to account for scaling or rounding errors
				float3 rayPosLocal;
				float3 rayDirLocal;
				TransformPointByMatrix(&rayPosLocal, &rayPos, &model->m_worldToModelX, &model->m_worldToModelY, &model->m_worldToModelZ, &model->m_worldToModelW);
				TransformVectorByMatrix(&rayDirLocal, &rayDir, &model->m_worldToModelX, &model->m_worldToModelY, &model->m_worldToModelZ);
				rayDirLocal = normalize(rayDirLocal);

				// convert the bounding sphere hit locations from world space to model space
				float3 hitStartLocal;
				float3 hitEndLocal;
				TransformPointByMatrix(&hitStartLocal, &hitStart, &model->m_worldToModelX, &model->m_worldToModelY, &model->m_worldToModelZ, &model->m_worldToModelW);
				TransformPointByMatrix(&hitEndLocal, &hitEnd, &model->m_worldToModelX, &model->m_worldToModelY, &model->m_worldToModelZ, &model->m_worldToModelW);

				// calculate which y half spaces this segment goes through
				cl_uint halfSpaceFlags = 0;
				halfSpaceFlags |= hitStartLocal.y > 0.0f ? e_halfSpacePosY : e_halfSpaceNegY;
				halfSpaceFlags |= hitEndLocal.y > 0.0f ? e_halfSpacePosY : e_halfSpaceNegY;

				for (int objectIndex = model->m_startObjectIndex; objectIndex < model->m_stopObjectIndex; ++objectIndex)
				{
					__global struct SModelObject *object = &objects[objectIndex];

					// allow back face culling if the triangle isn't refractive (transparent)
					unsigned int materialIndex = model->m_materialOverride == -1 ? object->m_materialIndex : model->m_materialOverride;
					bool backFaceCulling = !IsRefractive(&materials[materialIndex]);

					// figure out the triangle start and stop index to test against.
					// if the segment we are testing is in only the positive y half space or only the negative y half space, we can cut out triangles
					// that are completely in the other y half space.  If it has both, we need to test all unfortunately.
					unsigned int triangleIndex = (halfSpaceFlags & e_halfSpaceNegY) ? object->m_startTriangleIndex : object->m_mixStartTriangleIndex;
					unsigned int triangleStopIndex = (halfSpaceFlags & e_halfSpacePosY) ? object->m_stopTriangleIndex : object->m_mixStopTriangleIndex;

					for (; triangleIndex < triangleStopIndex; ++triangleIndex)
						RayIntersectTriangle(&triangles[triangleIndex], &collisionInfoLocal, rayPosLocal, rayDirLocal, lastHitPrimitiveId, backFaceCulling, materialIndex, object->m_portalIndex);
				}

				// if we hit something in local space, we need to convert the local space hit information back into world space
				if (collisionInfoLocal.m_objectHit != c_invalidObjectId)
				{
					// copy everything over
					collisionInfo = collisionInfoLocal;

					// convert collision info from model space to world space
					TransformPointByMatrixNoTemporary(&collisionInfo.m_intersectionPoint, &model->m_modelToWorldX, &model->m_modelToWorldY, &model->m_modelToWorldZ, &model->m_modelToWorldW);
					TransformVectorByMatrixNoTemporary(&collisionInfo.m_surfaceNormal, &model->m_modelToWorldX, &model->m_modelToWorldY, &model->m_modelToWorldZ);
					TransformVectorByMatrixNoTemporary(&collisionInfo.m_surfaceU, &model->m_modelToWorldX, &model->m_modelToWorldY, &model->m_modelToWorldZ);
					TransformVectorByMatrixNoTemporary(&collisionInfo.m_surfaceV, &model->m_modelToWorldX, &model->m_modelToWorldY, &model->m_modelToWorldZ);
					collisionInfo.m_intersectionTime *= model->m_scale;

					// make sure things are normalized as is appropriate (to account for scaling and rounding errors)
					collisionInfo.m_surfaceNormal = normalize(collisionInfo.m_surfaceNormal);
					collisionInfo.m_surfaceU = normalize(collisionInfo.m_surfaceU);
					collisionInfo.m_surfaceV = normalize(collisionInfo.m_surfaceV);
				}

				#if DEBUG_MODEL_BOUNDING_SPHERE
				collisionInfo.m_debugAdditiveColor += (halfSpaceFlags == e_halfSpacePosY) ? (float3)(0.0f,0.2f,0.0f) : (float3)(0.0f,0.0f,0.0f);
				collisionInfo.m_debugAdditiveColor += (halfSpaceFlags == e_halfSpaceNegY) ? (float3)(0.2f,0.0f,0.0f) : (float3)(0.0f,0.0f,0.0f);
				collisionInfo.m_debugAdditiveColor += (halfSpaceFlags == (e_halfSpaceNegY | e_halfSpacePosY)) ? (float3)(0.2f,0.2f,0.2f) : (float3)(0.0f,0.0f,0.0f);
				#endif
			}
		}

		RayIntersectSector(sector, &collisionInfo, rayPos, rayDir, lastHitPrimitiveId);

		// add the debug color in
		*pixelColor += collisionInfo.m_debugAdditiveColor;

		// if no hit, set pixel to ambient light and bail out
		if (collisionInfo.m_objectHit == c_invalidObjectId)
		{
			*pixelColor += ambientLight * colorMultiplier;
			return;
		}

		// if we hit a portal, change our sector, transform the ray and bail out of this loop.
		if (collisionInfo.m_portalIndex != -1)
		{
			// set our point if we are supposed to
			float3 transformedPoint;
			if (portals[collisionInfo.m_portalIndex].m_setPosition)
			{
				transformedPoint = portals[collisionInfo.m_portalIndex].m_position;
			}
			// else transform the collision point into sector space
			else
			{
				TransformPointByMatrix(
					&transformedPoint,
					&collisionInfo.m_intersectionPoint,
					&portals[collisionInfo.m_portalIndex].m_xaxis,
					&portals[collisionInfo.m_portalIndex].m_yaxis,
					&portals[collisionInfo.m_portalIndex].m_zaxis,
					&portals[collisionInfo.m_portalIndex].m_waxis);
			}

			// transform the ray direction into sector space
			float3 transformedDir;
			TransformVectorByMatrix(
				&transformedDir,
				&rayDir,
				&portals[collisionInfo.m_portalIndex].m_xaxis,
				&portals[collisionInfo.m_portalIndex].m_yaxis,
				&portals[collisionInfo.m_portalIndex].m_zaxis);

			rayPos = transformedPoint;
			rayDir = normalize(transformedDir);
			currentSector = portals[collisionInfo.m_portalIndex].m_sector;
			lastHitPrimitiveId = collisionInfo.m_objectHit;
			continue;
		}

		__global const struct SMaterial *material = &materials[collisionInfo.m_materialIndex];

		if (collisionInfo.m_fromInside)
			collisionInfo.m_surfaceNormal *= -1.0f;

		// handle normal mapping if there is any
		#if SETTINGS_NORMALMAP == 1
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
		#endif

		#if SETTINGS_COLORABSORB == 1
		float3 currentAbsorbance = absorbance * -collisionInfo.m_intersectionTime;

		currentAbsorbance.x = pow(10, currentAbsorbance.x);
		currentAbsorbance.y = pow(10, currentAbsorbance.y);
		currentAbsorbance.z = pow(10, currentAbsorbance.z);

		colorMultiplier *= currentAbsorbance;
		#endif

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

		#if DEBUG_TEXTURE_UV
		diffuseColorBase = (float3)(collisionInfo.m_textureCoordinates.xy, 0.0f);
		#endif

		// apply ambient lighting and emissive color
		float3 diffuseColor = diffuseColorBase * ambientLight + emissiveColor;

		// apply diffuse / specular from a point light
		for (int index = sector->m_staticLightStartIndex; index < sector->m_staticLightStopIndex; ++index)
			ApplyPointLight(
				&diffuseColor,
				&collisionInfo,
				sector,
				material,
				&lights[index],
				colorMultiplier,
				rayDir,
				spheres,
				triangles,
				objects,
				models,
				materials,
				diffuseColorBase
			);

		// add the diffuse color in
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
			
			if (collisionInfo.m_fromInside)
				absorbance -= material->m_absorbance;
			else
				absorbance += material->m_absorbance;

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
	__global struct SSharedDataRootHostToKernel *dataRoot,
	__global struct SPointLight *lights,
	__global struct SSphere *spheres,
	__global struct SModelTriangle *triangles,
	__global struct SModelObject *objects,
	__global struct SModelInstance *models,
	__global struct SSector *sectors,
	__global struct SMaterial *materials,
	__global struct SPortal *portals
	//__global struct SSharedDataRootKernelToHost *outDataRoot
)
{
    const int2 dims = (int2)(get_image_width(texOut), get_image_height(texOut));
	const int2 coord = (int2)(get_global_id(0), get_global_id(1));

	#if SETTINGS_INTERLACED == 1
	if ((coord.y > dims.y / 2) == (dataRoot->m_camera.m_frameCount % 2))
		return;
	//if ((coord.y / 16) % 2 == dataRoot->m_camera.m_frameCount % 2)
	//	return;
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
	TraceRay(dataRoot, tex3dIn, dataRoot->m_camera.m_pos, rayDir, &color, lights, spheres, triangles, objects, models, sectors, materials, portals);

	// record the max brightness if we should
	//if (dataRoot->m_camera.m_frameCount % dataRoot->m_camera.m_HDRBrightnessSamplingInterval == 0)
	//  atomic_max(&outDataRoot->m_maxBrightness1000x, (unsigned int)(ColorToGray(&color) * 1000.0f));

	// adjust for brightness
	color *= dataRoot->m_camera.m_brightnessMultiplier;

	#if SETTINGS_REDBLUE3D == 1	
		float grayLeft = ColorToGray(&color);
		color = (float3)(0);

		// trace the ray for the other eye
		float3 rightEyePos = dataRoot->m_camera.m_pos + dataRoot->m_camera.m_left * SETTINGS_REDBLUEWIDTH;
		TraceRay(dataRoot, tex3dIn, rightEyePos, rayDir, &color, lights, spheres, triangles, objects, sectors, materials, portals);
		color *= dataRoot->m_camera.m_brightnessMultiplier;
		float grayRight = ColorToGray(&color);

		color.x = grayLeft;
		color.y = 0.0f;
		color.z = grayRight;
	#endif

	write_imagef(texOut, coord, (float4)(color, 1.0)); 
}