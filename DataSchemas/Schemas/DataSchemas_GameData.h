/*==================================================================================================

	DataSchemas_GameData.h

	This defines the schemas used by the game data.

==================================================================================================*/

SchemaBegin(Gfx, "Graphics related game data")
	Field(unsigned int, HDRBrightnessSamplingInterval, 5, "Specifies how many frames should go by before we sample the screen brightness for HDR")
	Field(float, HDRBrightnessDelta, 0.05f, "Specifies how quickly the brightness can adjust due to HDR brightness sampling")
SchemaEnd

SchemaBegin(GameData, "Data used by the game")
	Field(float, StandingHeight, 3.0f, "The height of the camera off the floor while the player is standing")
	Field(float, CrouchingHeight, 1.5f, "The height of the camera off the floor while the player is crouched")
	Field(float, CrouchSpeed, 10.0f, "how fast the player can crouch in units per second.")
	Field(float, WalkSpeedStanding, 5.0f, "How fast the player can walk while standing")
	Field(float, WalkSpeedCrouching, 1.5f, "How fast the player can walk while crouching")
	Field(float, SprintSpeedModifier, 2.0f, "A multiplier for player speed while sprinting")
	Field_Schema(Gfx, Gfx, NULL, "Graphics related data")
SchemaEnd