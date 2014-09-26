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
// The systems are updated in the order defined below
//=============================================================================================================================

SystemBegin(FPSCamera, SYSTEM_SINGLE, "FPS style camera.  Only one entity can register with the camera at a time.")
	SystemComponent(Input,		SYSTEM_READONLY,	"FPS camera needs to read mouse movement.")

	SystemComponent(Camera,		SYSTEM_READWRITE,	"FPS camera needs to modify camera values.")
SystemEnd

SystemBegin(PlayerController,	SYSTEM_SINGLE,		"This system translates input into desired player movement.  Only one entity can use the player controller at a time.")
	SystemComponent(Camera,		SYSTEM_READONLY,	"The player controller needs the camera basis vectors")

	SystemComponent(Physics,	SYSTEM_READWRITE,	"The player controller modifies physics data based on input state.")
	SystemComponent(Input,		SYSTEM_READWRITE,	"We clear input each frame")
SystemEnd

SystemBegin(CreaturePhysics, SYSTEM_MANY, "The physics system is what lets creatures walk around the world, and also handles projectile impacts.")
	SystemComponent(Bearings,	SYSTEM_READWRITE,	"The physics system uses the physics world to resolve collisions and update creature bearings")
	SystemComponent(Physics,	SYSTEM_READWRITE,	"Physics information")
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
