#define		USE_CUSTOM					0
#define		USE_CUSTOM_SELECT			0
#define		USE_ANTI_ALIASING			1
#define		USE_PRE_SHARPEN				1
#define		USE_BLOOM					1
#define		USE_HDR						1
#define		USE_TECHNICOLOR				0
#define		USE_TONEMAP					1
#define		USE_SEPIA					0
#define		USE_NOISE					1
#define		USE_RADIAL					1
#define		USE_SCANLINES				0
#define		USE_ANAGLYPH				0
#define		USE_VIGNETTE				0
#define		USE_POST_SHARPEN			1
#define		USE_FINAL_LIMITER			0
#define		USE_SPLITSCREEN				0
#define		QUALITY__PRESET				9
float		MyQualitySubpix			=	0.3885;
float		MyQualityEdgeThreshold	=	0.3133;
float		MyQualityEdgeThresholdMin =	0.0801;
bool		highQualitySharpen		=	1;
float		Sharpen					=	0.028;
float		AverageBlur				=	0.15;
float		CoefficientsBlur		=	1.3999;
float		CoefficientsOriginal	=	2.3999;
float		SharpenEdge				=	0.28;
float		SharpenContour			=	0.055;
float		BloomThreshold			=	17.625;
float		BloomPower				=	2.115;
float		BloomWidth				=	0.0355;
float		HDRPower				=	0.84;
float		radius2					=	0.831;
#define		TechniAmount				0.11
#define		TechniPower					2.4
#define		redNegativeAmount			0.99
#define		greenNegativeAmount			0.99
#define		blueNegativeAmount			0.99
#define		Gamma						1.15
#define		Exposure					0
#define		Saturation					0
#define		Bleach						0.5
#define		Defog						0
#define		FogColor					float4(1.46,		2.05,		0.07,		2.55)
#define		BlueShift					0
#define		ColorTone					float3(1.94,		1.78,		1.28)
float		SepiaPower				=	0.11;
float		GreyPower				=	0.11;
#define		NoiseStrength				0.152
#define		NoiseIntensity				0.33
#define		LineIntensity				0.22
#define		LineAmount					512
#define		NoiseMode					1
#define		RadialStrength				0.25
#define		RadialWidth					-0.0415
#define		ScanLineWidth				4
#define		ScanLineColor				1
#define		ScanLinePower				0.03
float		AnaglyphSeperation		=	0.0035;
float		AnaglyphRed				=	0;
float		AnaglyphGreen			=	0;
float		AnaglyphBlue			=	0;
#define		VignetteRadius				0.985
#define		VignetteAmount				-2
#define		VignetteStrength			0.05
float4		LimiterMinColor=			float4(0.01,0.01,0.01,0.0f);
float4		LimiterMaxColor=			float4(0.99,0.99,0.99,1.0f);
#define		SplitScreenMode				1
#define		Check1						USE_HDR
#define		Shader1						HDRPass
#define		Check2						USE_BLOOM
#define		Shader2						BloomPass
#define		Check3						USE_TECHNICOLOR
#define		Shader3						TechnicolorPass
#define		Check4						USE_TONEMAP
#define		Shader4						TonemapPass
#define		Check5						USE_SEPIA
#define		Shader5						SepiaPass
#define		Check6						USE_NOISE
#define		Shader6						NoisePass
#define		Check7						USE_RADIAL
#define		Shader7						RadialPass
#define		Check8						USE_SCANLINES
#define		Shader8						ScanlinePass
#define		Check9						USE_CUSTOM
#define		Shader9						CustomPass
#define		CustomValue1				4
#define		CustomValue2				5
#define		CustomValue3				3
#define		CustomValue4				2
#define		CustomValue5				2
#define		CustomValue6				2

