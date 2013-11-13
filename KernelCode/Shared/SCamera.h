/*==================================================================================================

SCamera.h

A struct used by both host and kernel code

==================================================================================================*/

struct SCamera
{
	float3 m_pos;
	float3 m_fwd;
	float3 m_up;
	float3 m_left;
	// putting view width and view height in here causes a size mismatch in host vs kernel code due to padding differences.
	// it would be nice if we could figure out how to fix that
};