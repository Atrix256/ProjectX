/*==================================================================================================

	DataSchemas_GameData.h

	This defines the schemas used by the game data.

==================================================================================================*/

SchemaBegin(Gfx, "Graphics related game data")
	Field(unsigned int, HDRBrightnessSamplingInterval, 5, "Specifies how many frames should go by before we sample the screen brightness for HDR")
	Field(float, HDRBrightnessDelta, 0.05f, "Specifies how quickly the brightness can adjust due to HDR brightness sampling")
SchemaEnd

SchemaBegin(ComponentBearings, "The bearings component.  This holds info about the position and rotation of an entity")
	Field(std::string, id, "", "the id (unique name) of the component")
	Field(std::string, Sector, "", "The id of sector the ")
	Field_Schema(Vec3, Position, "0,0,0", "The location within the current sector")
	Field_Schema(Vec3, Rotation, "0,0,0", "Rotation around the X,Y,Z axis, in degrees. It applies X axis rotation, then Y axis, then Z axis.")
SchemaEnd

SchemaBegin(ComponentCamera, "Used at runtime by various camera systems")
	Field(std::string, id, "", "the id (unique name) of the component")
	Field(float, Pitch, 0.0f, "The starting value for pitch, in degrees.")
	Field(float, Yaw, 0.0f, "The starting value for yaw, in degrees.")
	Field(float, PitchMax, 45.0f, "The maximum pitch value allowed, in degrees.")
	Field(float, PitchMin, -45.0f, "The minimum pitch value allowed, in degrees.")
SchemaEnd

SchemaBegin(ComponentInput, "Used at runtime to hold information about the player input")
	Field(std::string, id, "", "the id (unique name) of the component")
SchemaEnd

SchemaBegin(ComponentPhysics, "Used at runtime to hold information about the player physics")
	Field(std::string, id, "", "the id (unique name) of the component")
	Field(float, CylinderRadius, 1.0f, "The radius of the physics cylinder")
	Field(float, CylinderHeight, 5.0f, "The height of the physics cylinder")
SchemaEnd

SchemaBegin(ECSEntity, "Describes an entity for the entity-component-system system")
	Field(std::string, id, "", "the id (unique name) of the entity")
	Field(std::string, ComponentBearings, "", "the id (unique name) of the component to use with this entity.  Blank means no component")
	Field(std::string, ComponentCamera, "", "the id (unique name) of the component to use with this entity.  Blank means no component")
	Field(std::string, ComponentInput, "", "the id (unique name) of the component to use with this entity.  Blank means no component")
	Field(std::string, ComponentPhysics, "", "the id (unique name) of the component to use with this entity.  Blank means no component")
	Field(bool, SystemFPSCamera, false, "Whether or not the FPS camera system should process this entity")
	Field(bool, SystemPlayerController, false, "Whether or not the player controller system should process this entity")
	Field(bool, SystemCreaturePhysics, false, "Whether or not the creature physics system should process this entity")
SchemaEnd

SchemaBegin(GameData, "Data used by the game")
	Field(float, StandingHeight, 3.0f, "The height of the camera off the floor while the player is standing")
	Field(float, CrouchingHeight, 1.5f, "The height of the camera off the floor while the player is crouched")
	Field(float, CrouchSpeed, 10.0f, "how fast the player can crouch in units per second.")
	Field(float, WalkSpeedStanding, 5.0f, "How fast the player can walk while standing")
	Field(float, WalkSpeedCrouching, 1.5f, "How fast the player can walk while crouching")
	Field(float, SprintSpeedModifier, 2.0f, "A multiplier for player speed while sprinting")
	Field_Schema(Gfx, Gfx, NULL, "Graphics related data")
	Field_Schema_Array(ComponentBearings, ComponentBearings, "The list of bearings components")
	Field_Schema_Array(ComponentCamera, ComponentCamera, "The list of camera components")
	Field_Schema_Array(ComponentInput, ComponentInput, "The list of input components")
	Field_Schema_Array(ComponentPhysics, ComponentPhysics, "The list of physics components")
	Field_Schema_Array(ECSEntity, Entity, "The list of entities")
	Field(std::string, PlayerEntity, "", "The entity to use for the player")
SchemaEnd

//SchemaBegin(