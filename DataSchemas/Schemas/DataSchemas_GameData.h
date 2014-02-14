/*==================================================================================================

	DataSchemas_GameData.h

	This defines the schemas used by the game data.

==================================================================================================*/

SchemaBegin(Gfx)
	Field(unsigned int, HDRBrightnessSamplingInterval, 5)
	Field(float, HDRBrightnessDelta, 0.05f)
SchemaEnd

SchemaBegin(GameData)
	Field(float, StandingHeight, 3.0f)
	Field(float, CrouchingHeight, 1.5f)
	Field(float, CrouchSpeed, 10.0f)
	Field(float, WalkSpeedStanding, 5.0f)
	Field(float, WalkSpeedCrouching, 1.5f)
	Field(float, SprintSpeedModifier, 2.0f)
	Field_Schema(Gfx, Gfx, NULL)
SchemaEnd