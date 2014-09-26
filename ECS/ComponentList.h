/*==================================================================================================

ComponentList.h

This defines the components available to entities.

ComponentBegin(name, hint) - defines a component.  Makes a class called CECSComponent<name> that
	recieves a const SData_Component<name>& in the constructor for initialization.  Make sure and
	define the component type also in the game data schema.  It does this so you can define
	component starting values in data.

ComponentData(type, name, hint) - adds a member variable to the component

ComponentEnd - end the component definition

==================================================================================================*/

// define all the macros which are not defined, as a convinience to the users of this list
#ifndef ComponentBegin
	#define ComponentBegin(name, hint)
#endif

#ifndef ComponentData
	#define ComponentData(type, name, hint)
#endif

#ifndef ComponentEnd
	#define ComponentEnd
#endif

//=================================================COMPONENT LIST=================================================================

ComponentBegin(Bearings, "The bearings store a location and rotation")
	ComponentData(unsigned int, sector, "The sector the entity is in")
	ComponentData(float3, position, "The position in the sector")
ComponentEnd

ComponentBegin(Input, "The input from a single frame is stored here")
	ComponentData(float, mouseMoveX, "How much the mouse has moved on the x axis")
	ComponentData(float, mouseMoveY, "How much the mouse has moved on the y axis")
	ComponentData(bool , keyWalkForward, "Tracks key state")
	ComponentData(bool , keyWalkLeft, "Tracks key state")
	ComponentData(bool , keyWalkRight, "Tracks key state")
	ComponentData(bool , keyWalkBack, "Tracks key state")
	ComponentData(bool , keyJump, "Tracks key state")
	ComponentData(bool , keyCrouch, "Tracks key state")
	ComponentData(bool , keySprint, "Tracks key state")
ComponentEnd

ComponentBegin(Camera, "The data needed for any kind of camera.  Up to the system to decide what sort of camera it is.")
	ComponentData(float, yaw, "Rotation around the vertical axis - looking left and right.")
	ComponentData(float, pitch, "Rotation around the horizontal axis - looking up and down.")
	ComponentData(const float, pitchMax, "The highest pitch value allowed.")
	ComponentData(const float, pitchMin, "The lowest pitch value allowed.")
ComponentEnd

ComponentBegin(Physics, "Entity physics information")
	ComponentData(float3, cylinderHalfDims, "The half dims of the physics cylinder. X,Z axis are the radius, Y axis is the half height")
	ComponentData(float3, positionDelta, "How much the object wants to move this frame")
ComponentEnd

//=============================================================================================================================

// undefine the macros, as a convinience to the users of this list
#undef ComponentBegin
#undef ComponentData
#undef ComponentEnd