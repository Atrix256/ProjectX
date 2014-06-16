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
	ComponentData(float3, rotationAxisX, "The x axis basis vector for the rotation matrix")
	ComponentData(float3, rotationAxisY, "The y axis basis vector for the rotation matrix")
	ComponentData(float3, rotationAxisZ, "The z axis basis vector for the rotation matrix")
ComponentEnd
	
//=============================================================================================================================

// undefine the macros, as a convinience to the users of this list
#undef ComponentBegin
#undef ComponentData
#undef ComponentEnd