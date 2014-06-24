/*==================================================================================================

SystemList.h

This defines the systems available to entities

SystemBegin(name, single, hint) - starts a system definition.  name is the unique identifier of the
	system.  single is SYSTEM_SINGLE or SYSTEM_MANY.  If true, only one entity may be registered with
	the system at a time, else any number of entities can register with it.

SystemEnd - ends a system definition

SystemComponent(type, access, hint) - specifies a component needed by the system.  Type is the type of
	component needed.  Access is SYSTEM_READONLY or SYSTEM_READWRITE.

==================================================================================================*/

// define all the macros which are not defined, as a convinience to the users of this list
#ifndef SystemBegin
	#define SystemBegin(name, single, hint)
#endif

#ifndef SystemComponent
	#define SystemComponent(type, access, hint)
#endif

#ifndef SystemEnd
	#define SystemEnd
#endif

#ifndef SYSTEM_SINGLE
	#define SYSTEM_SINGLE
#endif

#ifndef SYSTEM_MANY
	#define SYSTEM_MANY
#endif

#ifndef SYSTEM_READONLY
	#define SYSTEM_READONLY
#endif

#ifndef SYSTEM_READWRITE
	#define SYSTEM_READWRITE
#endif

//=================================================SYSTEM LIST=================================================================

SystemBegin(PlayerController, SYSTEM_SINGLE, "This system translates input into desired player movement.  Only one entity can use the player controller at a time.")
	SystemComponent(Bearings, SYSTEM_READWRITE, "The player controller modifies entity bearings based on input state.")
SystemEnd

SystemBegin(CreaturePhysics, SYSTEM_MANY, "The physics system is what lets creatures walk around the world, and also handles projectile impacts.")
	SystemComponent(Bearings, SYSTEM_READWRITE, "The physics system uses the physics world to resolve collisions and update creature bearings")
SystemEnd

// camera has it's own internal state and it gets set up by game data supposedly, then in response to mouse movement, it will yawright and pitchup
// also, it does move delta stuff based on player input, and "attempts to move" so does some physics stuff
SystemBegin(Camera, SYSTEM_SINGLE, "This system sets the camera to the orientation of an entity.  Only one entity can control the camera at a time.")
	SystemComponent(Bearings, SYSTEM_READONLY, "The camera reads the entity bearings to determine it's own orientation.")
SystemEnd

//=============================================================================================================================

// undefine the macros, as a convinience to the users of this list
#undef SystemBegin
#undef SystemComponent
#undef SystemEnd
#undef SYSTEM_SINGLE
#undef SYSTEM_MANY
#undef SYSTEM_READONLY
#undef SYSTEM_READWRITE
