/** @author Andy Wang  */
DROP TABLE IF EXISTS [Scene];
CREATE TABLE [Scene](
/** Common Attribute */
	[ID] INTEGER PRIMARY KEY,
	[SceneNameStringIndex] [int] UNIQUE NOT NULL,	/** Index into string table */
	
/** Skybox Attribute */
	[SkyboxColorRed] [int] NOT NULL,
	[SkyboxColorGreen] [int] NOT NULL,
	[SkyboxColorBlue] [int] NOT NULL,
	[SkyboxTexFilePath] [nchar] (64) NOT NULL,
	
/** Ocean Attribute */
	[OceanColorRed] [int] NOT NULL,
	[OceanColorGreen] [int] NOT NULL,
	[OceanColorBlue] [int] NOT NULL,
	
/** Fog Attribute */
	[FogRange] [float] NOT NULL,
	
/** Camera Attribute */
	[NearPlane] [float] NOT NULL,
	[FarPlane] [float] NOT NULL,
	[EyePos] [float] (3) NOT NULL,
	[LookatPos] [float] (3) NOT NULL,
	[CameraMode] [int] NOT NULL,
	
/** Performance Attribute */
	[PerformanceRate] [float] NOT NULL DEFAULT ((0)),
	[IsShadow] [smallint] NOT NULL DEFAULT ((1)),
	[IsSunlight] [smallint] NOT NULL DEFAULT ((1)),
	
/** Reserved Attribute */
	[Reserved1] [nvarchar] (16) NULL,
	[Reserved2] [nvarchar] (16) NULL,
	[Reserved3] [nvarchar] (16) NULL,
	[Reserved4] [nvarchar] (16) NULL,
	[Reserved5] [nvarchar] (16) NULL
);
