/************************************************************************/
/* File: ShaderSource.cpp
/* Author: Andrew Chase
/* Date: April 10th, 2018
/* Description: Source file for built-in shader code
/************************************************************************/
#include "Engine/Rendering/Shaders/ShaderSource.hpp"

// Sorting orders
const unsigned int ShaderSource::DEFAULT_OPAQUE_LAYER = 0;
const unsigned int ShaderSource::DEFAULT_ALPHA_LAYER = 0;
const SortingQueue ShaderSource::DEFAULT_OPAQUE_QUEUE = SORTING_QUEUE_OPAQUE;
const SortingQueue ShaderSource::DEFAULT_ALPHA_QUEUE = SORTING_QUEUE_ALPHA;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Default Opaque Blending Shader
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
const char* ShaderSource::DEFAULT_OPAQUE_NAME = "Default_Opaque";
const RenderState ShaderSource::DEFAULT_OPAQUE_STATE; // Just use default values (no blending)

// Vertex Shader
const char* ShaderSource::DEFAULT_OPAQUE_VS = R"(

	#version 420 core												
	
	layout(binding=1, std140) uniform cameraUBO
	{
		mat4 VIEW;
		mat4 PROJECTION;
	};
	
	layout(binding=2, std140) uniform modelUBO
	{
		mat4 MODEL;
	};
																												
	in vec3 POSITION;												
	in vec4 COLOR;													
	in vec2 UV;														
																	
	out vec2 passUV;												
	out vec4 passColor;												
																														
	void main( void )												
	{																										
		vec4 world_pos = vec4( POSITION, 1 ); 						
		vec4 clip_pos = PROJECTION * VIEW * MODEL * world_pos; 				
																	
		passUV = UV;												
		passColor = COLOR;											
		gl_Position = clip_pos; 								
	})";											 

// Fragment Shader
const char* ShaderSource::DEFAULT_OPAQUE_FS = R"(
	
	#version 420 core											
																											
	in vec2 passUV;												
	in vec4 passColor;											
																  										
	layout(binding = 0) uniform sampler2D gTexDiffuse;			
																												
	out vec4 outColor; 											
																
	// Entry Point												
	void main( void )											
	{																																			
		vec4 diffuse = texture(gTexDiffuse, passUV);	
		outColor = diffuse * passColor;	 				
	})";


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Default Alpha Blending Shader
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
const char* ShaderSource::DEFAULT_ALPHA_NAME = "Default_Alpha";
const char* ShaderSource::DEFAULT_ALPHA_VS = ShaderSource::DEFAULT_OPAQUE_VS;	// Uses the same default program
const char* ShaderSource::DEFAULT_ALPHA_FS = ShaderSource::DEFAULT_OPAQUE_FS;

const RenderState ShaderSource::DEFAULT_ALPHA_STATE = RenderState(
	CULL_MODE_BACK,							// Cull mode
	FILL_MODE_SOLID, 						// Fill mode
	WIND_COUNTER_CLOCKWISE, 				// Wind order
	DEPTH_TEST_LESS, 						// Depth compare method
	true, 									// Write to depth buffer on draws?
	BLEND_OP_ADD, 							// Color blend OP
	BLEND_FACTOR_SOURCE_ALPHA, 				// Color source factor
	BLEND_FACTOR_ONE_MINUS_SOURCE_ALPHA, 	// Color destination factor
	BLEND_OP_ADD, 							// Alpha blend OP
	BLEND_FACTOR_ONE, 						// Alpha source factor
	BLEND_FACTOR_ONE						// Alpha destination factor
);


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Default Opaque Blending Instanced Shader
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
const char* ShaderSource::DEFAULT_OPAQUE_INSTANCED_NAME = "Default_Opaque_Instanced";
const char* ShaderSource::DEFAULT_OPAQUE_INSTANCED_VS = R"(

	#version 420 core												
	
	layout(binding=1, std140) uniform cameraUBO
	{
		mat4 VIEW;
		mat4 PROJECTION;
	};
																												
	in vec3 POSITION;												
	in vec4 COLOR;													
	in vec2 UV;	
	in mat4 INSTANCE_MODEL_MATRIX;													
																	
	out vec2 passUV;												
	out vec4 passColor;												
																														
	void main( void )												
	{																										
		vec4 world_pos = vec4( POSITION, 1 ); 						
		vec4 clip_pos = PROJECTION * VIEW * INSTANCE_MODEL_MATRIX * world_pos; 				
																	
		passUV = UV;												
		passColor = COLOR;											
		gl_Position = clip_pos; 								
	})";

const char* ShaderSource::DEFAULT_OPAQUE_INSTANCED_FS = R"(
	
	#version 420 core											
																											
	in vec2 passUV;												
	in vec4 passColor;											
																  										
	layout(binding = 0) uniform sampler2D gTexDiffuse;			
																												
	out vec4 outColor; 											
																
	// Entry Point												
	void main( void )											
	{																																			
		vec4 diffuse = texture(gTexDiffuse, passUV);	
		outColor = diffuse * passColor;	 				
	})";

const RenderState ShaderSource::DEFAULT_OPAQUE_INSTANCED_STATE;


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Default Alpha Blending Instanced Shader
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
const char* ShaderSource::DEFAULT_ALPHA_INSTANCED_NAME = "Default_Alpha_Instanced";
const char* ShaderSource::DEFAULT_ALPHA_INSTANCED_VS = ShaderSource::DEFAULT_OPAQUE_INSTANCED_VS;
const char* ShaderSource::DEFAULT_ALPHA_INSTANCED_FS = ShaderSource::DEFAULT_OPAQUE_INSTANCED_FS;
const RenderState ShaderSource::DEFAULT_ALPHA_INSTANCED_STATE = ShaderSource::DEFAULT_ALPHA_STATE;


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Skybox Shader
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
const char* ShaderSource::SKYBOX_SHADER_NAME = "Skybox";
const char* ShaderSource::SKYBOX_SHADER_VS = R"(

	#version 420 core												

	layout(binding=1, std140) uniform cameraUBO
	{
		mat4 VIEW;
		mat4 PROJECTION;
	};
																												
	in vec3 POSITION;																																					
																	
	out vec3 passWorldPosition;																							
	
	void main( void )												
	{																										
		// 1, since I don't want to translate
	   	vec4 local_pos = vec4(POSITION, 0.0f);	
	
	   	vec4 world_pos = local_pos;
		vec4 camera_pos = VIEW * world_pos;
	   	vec4 clip_pos = PROJECTION * vec4(camera_pos.xyz, 1.0f); 				
																	
		passWorldPosition = world_pos.xyz;												
	
		gl_Position = clip_pos.xyww; 	
							
	})";

const char* ShaderSource::SKYBOX_SHADER_FS = R"(

	#version 420 core											
																																																
	in vec3 passWorldPosition;
	
	layout(binding = 0) uniform samplerCube gTexSky;			
																												
	out vec4 outColor; 											
																
	// Entry Point												
	void main( void )											
	{	
	   	vec3 normal = normalize(passWorldPosition); 
		vec4 tex_color = texture(gTexSky, normal);	
		outColor = tex_color;	 				
	})";

const RenderState ShaderSource::SKYBOX_SHADER_STATE = RenderState(
	CULL_MODE_NONE,								// Cull mode
	FILL_MODE_SOLID, 							// Fill mode
	WIND_COUNTER_CLOCKWISE, 					// Wind order
	DEPTH_TEST_LEQUAL, 							// Depth compare method
	false, 										// Write to depth buffer on draws?
	BLEND_OP_ADD, 								// Color blend OP
	BLEND_FACTOR_ONE_MINUS_DESTINATION_ALPHA, 	// Color source factor
	BLEND_FACTOR_DESTINATION_ALPHA, 			// Color destination factor
	BLEND_OP_ADD, 								// Alpha blend OP
	BLEND_FACTOR_ONE, 							// Alpha source factor
	BLEND_FACTOR_ONE							// Alpha destination factor
);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Font Shader
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
const char* ShaderSource::UI_SHADER_NAME = "UI";
const char* ShaderSource::UI_SHADER_VS = R"(

	#version 420 core												
	
	layout(binding=1, std140) uniform cameraUBO
	{
		mat4 VIEW;
		mat4 PROJECTION;
	};
																												
	in vec3 POSITION;												
	in vec4 COLOR;													
	in vec2 UV;														
																	
	out vec2 passUV;												
	out vec4 passColor;												
																														
	void main( void )												
	{																										
		vec4 world_pos = vec4( POSITION, 1 ); 						
		vec4 clip_pos = PROJECTION * VIEW * world_pos; 				
																	
		passUV = UV;												
		passColor = COLOR;											
		gl_Position = clip_pos; 
								
	})";

const char* ShaderSource::UI_SHADER_FS = R"(
	
	#version 420 core											
																											
	in vec2 passUV;												
	in vec4 passColor;											
																  										
	layout(binding = 0) uniform sampler2D gTexDiffuse;			
																												
	out vec4 outColor; 											
																
	// Entry Point												
	void main( void )											
	{																																			
		vec4 diffuse = texture(gTexDiffuse, passUV);	
		outColor = diffuse * passColor;	 				
	})";

const RenderState ShaderSource::UI_SHADER_STATE = RenderState(
	CULL_MODE_BACK,							// Cull mode
	FILL_MODE_SOLID, 						// Fill mode
	WIND_COUNTER_CLOCKWISE, 				// Wind order
	DEPTH_TEST_ALWAYS, 						// Depth compare method
	false, 									// Write to depth buffer on draws?
	BLEND_OP_ADD, 							// Color blend OP
	BLEND_FACTOR_SOURCE_ALPHA, 				// Color source factor
	BLEND_FACTOR_ONE_MINUS_SOURCE_ALPHA, 						// Color destination factor
	BLEND_OP_ADD, 							// Alpha blend OP
	BLEND_FACTOR_SOURCE_ALPHA, 				// Alpha source factor
	BLEND_FACTOR_ONE_MINUS_SOURCE_ALPHA		// Alpha destination factor
);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Invalid Shader
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
const char* ShaderSource::INVALID_SHADER_NAME = "Invalid";
const RenderState ShaderSource::INVALID_RENDER_STATE; // Default values

// Vertex Shader
const char* ShaderSource::INVALID_VS = R"(

	#version 420 core												
	
	layout(binding=1, std140) uniform cameraUBO
	{
		mat4 VIEW;
		mat4 PROJECTION;
	};
	
	layout(binding=2, std140) uniform modelUBO
	{
		mat4 MODEL;
	};
																												
	in vec3 POSITION;																																						
																																																																							
	void main( void )												
	{																										
		vec4 world_pos = vec4( POSITION, 1 ); 						
		vec4 clip_pos = PROJECTION * VIEW * MODEL * world_pos; 				
																																							
		gl_Position = clip_pos; 								
	})";

// Fragment Shader
const char* ShaderSource::INVALID_FS = R"(
	
	#version 420 core								
																																	
	out vec4 outColor; 								
																						
	void main( void )								
	{												
		// Always output magenta					
		outColor = vec4( 1, 0, 1, 1 ); 		
	})";


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Debug Render - Use Depth
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
const char* ShaderSource::DEBUG_RENDER_NAME = "Debug_Render";
const RenderState ShaderSource::DEBUG_RENDER_STATE;		// Default values

// Vertex Shader
const char* ShaderSource::DEBUG_RENDER_VS = R"(
	
	#version 420 core												
	
	layout(binding=1, std140) uniform cameraUBO
	{
		mat4 VIEW;
		mat4 PROJECTION;
	};
	
	layout(binding=2, std140) uniform modelUBO
	{
		mat4 MODEL;
	};
																												
	in vec3 POSITION;
	in vec4 COLOR;																									
	in vec2 UV;														
																	
	out vec2 passUV;												
	out vec4 passColor;
																								
	void main( void )												
	{																										
		vec4 world_pos = vec4( POSITION, 1 ); 						
		vec4 clip_pos = PROJECTION * VIEW * MODEL * world_pos; 				
																	
		passUV = UV;
		passColor = COLOR;																						
		gl_Position = clip_pos; 
										
	})";											 

// Fragment Shader
const char* ShaderSource::DEBUG_RENDER_FS = R"(
	
	#version 420 core											
		
	layout(binding=8, std140) uniform tintUBO
	{
		vec4 TINT;
	};				
																					
	in vec2 passUV;												
	in vec4 passColor;
										  										
	layout(binding = 0) uniform sampler2D gTexDiffuse;			
																												
	out vec4 outColor; 											
																
	// Entry Point												
	void main( void )											
	{																																			
		vec4 diffuse = texture(gTexDiffuse, passUV);	
		outColor = diffuse * TINT * passColor;	 				
	})";


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Phong Opaque
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
const char* ShaderSource::PHONG_OPAQUE_NAME = "Phong_Opaque";
const RenderState ShaderSource::PHONG_OPAQUE_STATE; // Default values


const char* ShaderSource::PHONG_OPAQUE_VS = R"(
	
#version 420 core												

layout(binding=1, std140) uniform cameraUBO
{
	mat4 VIEW;
	mat4 PROJECTION;

	mat4 CAMERA_MATRIX;

	vec3	CAMERA_RIGHT;
	float	PADDING_0;
	vec3	CAMERA_UP;
	float	PADDING_1;
	vec3	CAMERA_FORWARD;
	float	PADDING_2;
	vec3	CAMERA_POSITION;
	float	PADDING_3;
};

layout(binding=2, std140) uniform modelUBO
{
	mat4 MODEL;
};
																											
in vec3 POSITION;												
in vec4 COLOR;													
in vec2 UV;	
in vec3 NORMAL;	
in vec4 TANGENT;												
																
out vec2 passUV;												
out vec4 passColor;
out vec3 passWorldPosition; // For determining light direction
out mat4 passTBNTransform;					
out vec3 passEyePosition;

																									
void main( void )												
{						
	vec4 localPosition = vec4(POSITION, 1);																				
	vec4 worldPosition = MODEL * localPosition; 						
	vec4 clipPosition = PROJECTION * VIEW * worldPosition; 				
																
	passUV = UV;												
	passColor = COLOR;

	passWorldPosition = worldPosition.xyz;

	// Calculate the TBN transform
	vec3 worldNormal = normalize((MODEL * vec4(NORMAL, 0.f)).xyz);
	vec3 worldTangent = normalize((MODEL * vec4(TANGENT.xyz, 0.f)).xyz);
	vec3 worldBitangent = cross(worldTangent, worldNormal) * TANGENT.w;

	passTBNTransform = mat4(vec4(worldTangent, 0.f), vec4(worldBitangent, 0.f), vec4(worldNormal, 0.f), vec4(passWorldPosition, 1.0f));
	passEyePosition = CAMERA_POSITION;

	gl_Position = clipPosition; 
									
})";

const char* ShaderSource::PHONG_OPAQUE_FS = R"(
	
	#version 420 core											
	#define MAX_LIGHTS 8
																										
	layout(binding = 0) uniform sampler2D gTexDiffuse;			
	layout(binding = 1) uniform sampler2D gTexNormal;
	layout(binding = 8) uniform sampler2D gShadowDepth;

	struct Light
	{
		vec3 m_position;
		float m_dotOuterAngle;
		vec3 m_direction;
		float m_dotInnerAngle;
		vec3 m_attenuationFactors;
		float m_directionFactor;
		vec4 m_color;
		mat4 m_shadowVP;
		vec3 m_padding;
		float m_castsShadows;
	};
	
	layout(binding=3, std140) uniform lightUBO
	{
		vec4 AMBIENT;							// xyz color, w intensity
		Light LIGHTS[MAX_LIGHTS];
	};	

	layout(binding=8, std140) uniform specularUBO
	{
		float SPECULAR_AMOUNT;
		float SPECULAR_POWER;
		vec2 PADDING_4;
	};
	
	in vec2 passUV;												
	in vec4 passColor;											
	
	in vec3 passEyePosition;
	in vec3 passWorldPosition;
	in mat4 passTBNTransform;
	
	out vec4 outColor;
	
	//---------------------------------------------------Functions-----------------------------------------------------------------
	
	// Calculates the normal given the surface normal color sample and the TBN transformation
	vec3 CalculateWorldNormal(vec4 color)
	{
		// Range map the values (Z should be between 0.5 and 1.0 always
		vec3 surfaceNormal = normalize(2.f * color.xyz - vec3(1));
		vec3 worldNormal = (passTBNTransform * vec4(surfaceNormal, 0.f)).xyz;
	
		return worldNormal;
	}
	
	// Calculates the attenuation factor for the given light information
	float CalculateAttenuation(vec3 lightPosition, vec3 attenuationFactors, float intensity)
	{
		float distance = length(lightPosition - passWorldPosition);
		float denominator = attenuationFactors.x + attenuationFactors.y * distance + attenuationFactors.z * distance * distance;
		float attenuation = (intensity / denominator);
	
		return attenuation;
	} 	
	
	
	// Calculates the cone factor (= 1.f for directional and point lights, 0.f <= x <= 1.f for spot lights)
	float CalculateConeFactor(vec3 lightPosition, vec3 lightDirection, float outerDotThreshold, float innerDotThreshold)
	{
		float dotFactor = dot(normalize(passWorldPosition - lightPosition), lightDirection);
		float coneFactor = smoothstep(outerDotThreshold, innerDotThreshold, dotFactor);
	
		return coneFactor;
	}			
	
	
	// Calculates the diffuse factor for the 
	vec3 CalculateDot3(vec3 directionToLight, vec3 normal, vec4 lightColor, float attenuation, float coneFactor)
	{
		vec3 dot3 = (max(0.f, dot(directionToLight, normal)) * lightColor.xyz * lightColor.w * attenuation * coneFactor);
		return dot3;
	}
	
	vec3 CalculateSpecular(vec3 directionToLight, vec3 normal, vec3 directionToEye, vec4 lightColor, float attenuation, float coneFactor)
	{
		vec3 reflect = reflect(-directionToLight, normal);
		float factor = max(0, dot(directionToEye, reflect));
		vec3 specular = SPECULAR_AMOUNT * pow(factor, SPECULAR_POWER) * lightColor.xyz * lightColor.w * attenuation * coneFactor;
		
		return specular;
	}
	
	float CalculateShadowFactor(vec3 fragPosition, vec3 normal, Light light)
	{
		if (light.m_castsShadows == 0.f)
		{
			return 1.0f;
		}

		vec4 clipPos = light.m_shadowVP * vec4(fragPosition, 1.0f);
		vec3 ndcPos = clipPos.xyz / clipPos.w;

		ndcPos = (ndcPos + vec3(1)) * 0.5f;

		float shadowDepth = texture(gShadowDepth, ndcPos.xy).r;

		return ndcPos.z - 0.001 > shadowDepth ? 0.f : 1.f;
	}
	
	// Entry point															
	void main( void )											
	{				
		//----------------------------SET UP VALUES-------------------------------		
		vec4 surfaceColor = texture(gTexDiffuse, passUV);
		vec3 directionToEye = normalize(passEyePosition - passWorldPosition);
	
		// Get the normal from the normal map, and transform it into TBN space
		vec4 normalColor = texture(gTexNormal, passUV);
		vec3 worldNormal = CalculateWorldNormal(normalColor);	
	
		// Calculate the direction TO light, attenuation, and cone factor for each light
		// Set up accumulation variables
		vec3 surfaceLight = vec3(0);	// How much light is hitting the surface
		vec3 reflectedLight = vec3(0);	// How much light is being reflected back
	
		//----------STEP 1: Add in the ambient light to the surface light----------
		surfaceLight = AMBIENT.xyz * AMBIENT.w;
	
		for (int lightIndex = 0; lightIndex < MAX_LIGHTS; ++lightIndex)
		{
			// Directions to the light
			vec3 directionToLight = mix(-LIGHTS[lightIndex].m_direction, normalize(LIGHTS[lightIndex].m_position - passWorldPosition), LIGHTS[lightIndex].m_directionFactor);
	
			// Attenuation
			float attenuation = CalculateAttenuation(LIGHTS[lightIndex].m_position, LIGHTS[lightIndex].m_attenuationFactors, LIGHTS[lightIndex].m_color.w);
	
			// Cone factor
			float coneFactor = CalculateConeFactor(LIGHTS[lightIndex].m_position, LIGHTS[lightIndex].m_direction, LIGHTS[lightIndex].m_dotOuterAngle, LIGHTS[lightIndex].m_dotInnerAngle);
	
	
			//-------------STEP 2: Add in the diffuse light from all lights------------	
			float shadowFactor = CalculateShadowFactor(passWorldPosition, worldNormal, LIGHTS[lightIndex]);

			surfaceLight += shadowFactor * CalculateDot3(directionToLight, worldNormal, LIGHTS[lightIndex].m_color, attenuation, coneFactor);
			
			//-----STEP 3: Calculate and add in specular lighting from all lights----------
			reflectedLight += shadowFactor * CalculateSpecular(directionToLight, worldNormal, directionToEye, LIGHTS[lightIndex].m_color, attenuation, coneFactor);
		}
	
	
		// Clamp the surface light, since it alone shouldn't blow out the surface (either it's fully lit, or not fully lit)
		surfaceLight = clamp(surfaceLight, vec3(0), vec3(1));
	
	
		//---------------------STEP 4: CALCULATE FINAL COLOR-----------------------
		// Calculate the final color, surface gets w = 1 since it is multiplied in, reflected gets w = 0 since it is added in
		vec4 finalColor = vec4(surfaceLight, 1) * surfaceColor * passColor + vec4(reflectedLight, 0);
	
		// Clamp the color
		finalColor = clamp(finalColor, vec4(0), vec4(1));
		
		outColor = finalColor; 				
	})";


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Phong Alpha
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
const char* ShaderSource::PHONG_ALPHA_NAME = "Phong_Alpha";
const char* ShaderSource::PHONG_ALPHA_VS = ShaderSource::PHONG_OPAQUE_VS;
const char* ShaderSource::PHONG_ALPHA_FS = ShaderSource::PHONG_OPAQUE_FS;

const RenderState ShaderSource::PHONG_ALPHA_STATE = RenderState(
	CULL_MODE_BACK,							// Cull mode
	FILL_MODE_SOLID, 						// Fill mode
	WIND_COUNTER_CLOCKWISE, 				// Wind order
	DEPTH_TEST_LESS, 						// Depth compare method
	true, 									// Write to depth buffer on draws?
	BLEND_OP_ADD, 							// Color blend OP
	BLEND_FACTOR_SOURCE_ALPHA, 				// Color source factor
	BLEND_FACTOR_ONE_MINUS_SOURCE_ALPHA, 	// Color destination factor
	BLEND_OP_ADD, 							// Alpha blend OP
	BLEND_FACTOR_ONE, 						// Alpha source factor
	BLEND_FACTOR_ZERO						// Alpha destination factor
);


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Phong Opaque Instanced
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
const char* ShaderSource::PHONG_OPAQUE_INSTANCED_NAME = "Phong_Opaque_Instanced";
const char* ShaderSource::PHONG_OPAQUE_INSTANCED_VS = R"(
	
	#version 420 core												
	
	layout(binding=1, std140) uniform cameraUBO
	{
		mat4 VIEW;
		mat4 PROJECTION;
	
		mat4 CAMERA_MATRIX;
	
		vec3	CAMERA_RIGHT;
		float	PADDING_0;
		vec3	CAMERA_UP;
		float	PADDING_1;
		vec3	CAMERA_FORWARD;
		float	PADDING_2;
		vec3	CAMERA_POSITION;
		float	PADDING_3;
	};
																												
	in vec3 POSITION;												
	in vec4 COLOR;													
	in vec2 UV;	
	in vec3 NORMAL;	
	in vec4 TANGENT;												
	in mat4 INSTANCE_MODEL_MATRIX;													
																
	out vec2 passUV;												
	out vec4 passColor;
	out vec3 passWorldPosition; // For determining light direction
	out mat4 passTBNTransform;					
	out vec3 passEyePosition;
	
																										
	void main( void )												
	{						
		vec4 localPosition = vec4(POSITION, 1);																				
		vec4 worldPosition = INSTANCE_MODEL_MATRIX * localPosition; 						
		vec4 clipPosition = PROJECTION * VIEW * worldPosition; 				
																	
		passUV = UV;												
		passColor = COLOR;
	
		passWorldPosition = worldPosition.xyz;
	
		// Calculate the TBN transform
		vec3 worldNormal = normalize((INSTANCE_MODEL_MATRIX * vec4(NORMAL, 0.f)).xyz);
		vec3 worldTangent = normalize((INSTANCE_MODEL_MATRIX * vec4(TANGENT.xyz, 0.f)).xyz);
		vec3 worldBitangent = cross(worldTangent, worldNormal) * TANGENT.w;
	
		passTBNTransform = mat4(vec4(worldTangent, 0.f), vec4(worldBitangent, 0.f), vec4(worldNormal, 0.f), vec4(passWorldPosition, 1.0f));
		passEyePosition = CAMERA_POSITION;
	
		gl_Position = clipPosition; 
										
	})";			

const char* ShaderSource::PHONG_OPAQUE_INSTANCED_FS = R"(
	
	#version 420 core											
	#define MAX_LIGHTS 8
																										
	layout(binding = 0) uniform sampler2D gTexDiffuse;			
	layout(binding = 1) uniform sampler2D gTexNormal;
	layout(binding = 8) uniform sampler2D gShadowDepth;

	struct Light
	{
		vec3 m_position;
		float m_dotOuterAngle;
		vec3 m_direction;
		float m_dotInnerAngle;
		vec3 m_attenuationFactors;
		float m_directionFactor;
		vec4 m_color;
		mat4 m_shadowVP;
		vec3 m_padding;
		float m_castsShadows;
	};
	
	layout(binding=3, std140) uniform lightUBO
	{
		vec4 AMBIENT;							// xyz color, w intensity
		Light LIGHTS[MAX_LIGHTS];
	};	
	
	layout(binding=8, std140) uniform specularUBO
	{
		float SPECULAR_AMOUNT;
		float SPECULAR_POWER;
		vec2 PADDING_4;
	};
	
	in vec2 passUV;												
	in vec4 passColor;											
	
	in vec3 passEyePosition;
	in vec3 passWorldPosition;
	in mat4 passTBNTransform;
	
	out vec4 outColor;
	
	//---------------------------------------------------Functions-----------------------------------------------------------------
	
	// Calculates the normal given the surface normal color sample and the TBN transformation
	vec3 CalculateWorldNormal(vec4 color)
	{
		// Range map the values (Z should be between 0.5 and 1.0 always
		vec3 surfaceNormal = normalize(2.f * color.xyz - vec3(1));
		vec3 worldNormal = (passTBNTransform * vec4(surfaceNormal, 0.f)).xyz;
	
		return worldNormal;
	}
	
	// Calculates the attenuation factor for the given light information
	float CalculateAttenuation(vec3 lightPosition, vec3 attenuationFactors, float intensity)
	{
		float distance = length(lightPosition - passWorldPosition);
		float denominator = attenuationFactors.x + attenuationFactors.y * distance + attenuationFactors.z * distance * distance;
		float attenuation = (intensity / denominator);
	
		return attenuation;
	} 	
	
	
	// Calculates the cone factor (= 1.f for directional and point lights, 0.f <= x <= 1.f for spot lights)
	float CalculateConeFactor(vec3 lightPosition, vec3 lightDirection, float outerDotThreshold, float innerDotThreshold)
	{
		float dotFactor = dot(normalize(passWorldPosition - lightPosition), lightDirection);
		float coneFactor = smoothstep(outerDotThreshold, innerDotThreshold, dotFactor);
	
		return coneFactor;
	}			
	
	
	// Calculates the diffuse factor for the 
	vec3 CalculateDot3(vec3 directionToLight, vec3 normal, vec4 lightColor, float attenuation, float coneFactor)
	{
		vec3 dot3 = (max(0.f, dot(directionToLight, normal)) * lightColor.xyz * lightColor.w * attenuation * coneFactor);
		return dot3;
	}
	
	vec3 CalculateSpecular(vec3 directionToLight, vec3 normal, vec3 directionToEye, vec4 lightColor, float attenuation, float coneFactor)
	{
		vec3 reflect = reflect(-directionToLight, normal);
		float factor = max(0, dot(directionToEye, reflect));
		vec3 specular = SPECULAR_AMOUNT * pow(factor, SPECULAR_POWER) * lightColor.xyz * lightColor.w * attenuation * coneFactor;
		
		return specular;
	}
	
	float CalculateShadowFactor(vec3 fragPosition, vec3 normal, Light light)
	{
		if (light.m_castsShadows == 0.f)
		{
			return 1.0f;
		}

		vec4 clipPos = light.m_shadowVP * vec4(fragPosition, 1.0f);
		vec3 ndcPos = clipPos.xyz / clipPos.w;

		ndcPos = (ndcPos + vec3(1)) * 0.5f;

		float shadowDepth = texture(gShadowDepth, ndcPos.xy).r;

		return ndcPos.z - 0.001 > shadowDepth ? 0.f : 1.f;
	}
	
	// Entry point															
	void main( void )											
	{				
		//----------------------------SET UP VALUES-------------------------------		
		vec4 surfaceColor = texture(gTexDiffuse, passUV);
		vec3 directionToEye = normalize(passEyePosition - passWorldPosition);
	
		// Get the normal from the normal map, and transform it into TBN space
		vec4 normalColor = texture(gTexNormal, passUV);
		vec3 worldNormal = CalculateWorldNormal(normalColor);	
	
		// Calculate the direction TO light, attenuation, and cone factor for each light
		// Set up accumulation variables
		vec3 surfaceLight = vec3(0);	// How much light is hitting the surface
		vec3 reflectedLight = vec3(0);	// How much light is being reflected back
	
		//----------STEP 1: Add in the ambient light to the surface light----------
		surfaceLight = AMBIENT.xyz * AMBIENT.w;
	
		for (int lightIndex = 0; lightIndex < MAX_LIGHTS; ++lightIndex)
		{
			// Directions to the light
			vec3 directionToLight = mix(-LIGHTS[lightIndex].m_direction, normalize(LIGHTS[lightIndex].m_position - passWorldPosition), LIGHTS[lightIndex].m_directionFactor);
	
			// Attenuation
			float attenuation = CalculateAttenuation(LIGHTS[lightIndex].m_position, LIGHTS[lightIndex].m_attenuationFactors, LIGHTS[lightIndex].m_color.w);
	
			// Cone factor
			float coneFactor = CalculateConeFactor(LIGHTS[lightIndex].m_position, LIGHTS[lightIndex].m_direction, LIGHTS[lightIndex].m_dotOuterAngle, LIGHTS[lightIndex].m_dotInnerAngle);
	
	
			//-------------STEP 2: Add in the diffuse light from all lights------------	
			float shadowFactor = CalculateShadowFactor(passWorldPosition, worldNormal, LIGHTS[lightIndex]);
			surfaceLight += shadowFactor * CalculateDot3(directionToLight, worldNormal, LIGHTS[lightIndex].m_color, attenuation, coneFactor);
			
			//-----STEP 3: Calculate and add in specular lighting from all lights----------
			reflectedLight += shadowFactor * CalculateSpecular(directionToLight, worldNormal, directionToEye, LIGHTS[lightIndex].m_color, attenuation, coneFactor);
		}
	
	
		// Clamp the surface light, since it alone shouldn't blow out the surface (either it's fully lit, or not fully lit)
		surfaceLight = clamp(surfaceLight, vec3(0), vec3(1));
	
	
		//---------------------STEP 4: CALCULATE FINAL COLOR-----------------------
		// Calculate the final color, surface gets w = 1 since it is multiplied in, reflected gets w = 0 since it is added in
		vec4 finalColor = vec4(surfaceLight, 1) * surfaceColor * passColor + vec4(reflectedLight, 0);
	
		// Clamp the color
		finalColor = clamp(finalColor, vec4(0), vec4(1));
		
		outColor = finalColor; 				
	})";

const RenderState ShaderSource::PHONG_OPAQUE_INSTANCED_STATE = ShaderSource::PHONG_OPAQUE_STATE;


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Phong Alpha Instanced
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
const char* ShaderSource::PHONG_ALPHA_INSTANCED_NAME = "Phong_Alpha_Instanced";
const char* ShaderSource::PHONG_ALPHA_INSTANCED_VS = ShaderSource::PHONG_OPAQUE_INSTANCED_VS;											 
const char* ShaderSource::PHONG_ALPHA_INSTANCED_FS = ShaderSource::PHONG_OPAQUE_INSTANCED_FS;	
const RenderState ShaderSource::PHONG_ALPHA_INSTANCED_STATE = ShaderSource::PHONG_ALPHA_STATE;


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Vertex Normal
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
const char* ShaderSource::VERTEX_NORMAL_NAME = "Vertex_Normal";
const char* ShaderSource::VERTEX_NORMAL_VS = R"(
	
#version 420 core												

layout(binding=1, std140) uniform cameraUBO
{
	mat4 VIEW;
	mat4 PROJECTION;

	mat4 CAMERA_MATRIX;

	vec3	CAMERA_RIGHT;
	float	PADDING_0;
	vec3	CAMERA_UP;
	float	PADDING_1;
	vec3	CAMERA_FORWARD;
	float	PADDING_2;
	vec3	CAMERA_POSITION;
	float	PADDING_3;
};

layout(binding=2, std140) uniform modelUBO
{
	mat4 MODEL;
};
																												
in vec3 POSITION;																								
in vec3 NORMAL;													
																												
out vec3 passVertexNormal;					
																											
void main( void )												
{						
	vec4 localPosition = vec4(POSITION, 1);																				
	vec4 worldPosition = MODEL * localPosition; 						
	vec4 clipPosition = PROJECTION * VIEW * worldPosition; 				

	passVertexNormal = (MODEL * vec4(NORMAL, 0.f)).xyz;															

	gl_Position = clipPosition; 
									
})";

const char* ShaderSource::VERTEX_NORMAL_FS = R"(
	

#version 420 core											
#define MAX_LIGHTS 8
																																												
in vec3 passVertexNormal;

out vec4 outColor;

// Entry point															
void main( void )											
{				
	outColor = vec4((passVertexNormal + vec3(1)) * 0.5f, 1.0f); 				
})";

const RenderState ShaderSource::VERTEX_NORMAL_STATE;	// Default state


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Vertex Tangent
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
const char* ShaderSource::VERTEX_TANGENT_NAME = "Vertex_Tangent";
const char* ShaderSource::VERTEX_TANGENT_VS = R"(
	
#version 420 core												

layout(binding=1, std140) uniform cameraUBO
{
	mat4 VIEW;
	mat4 PROJECTION;

	mat4 CAMERA_MATRIX;

	vec3	CAMERA_RIGHT;
	float	PADDING_0;
	vec3	CAMERA_UP;
	float	PADDING_1;
	vec3	CAMERA_FORWARD;
	float	PADDING_2;
	vec3	CAMERA_POSITION;
	float	PADDING_3;
};

layout(binding=2, std140) uniform modelUBO
{
	mat4 MODEL;
};
																												
in vec3 POSITION;													
in vec4 TANGENT;												
																												
out vec3 passVertexTangent;					
																											
void main( void )												
{						
	vec4 localPosition = vec4(POSITION, 1);																				
	vec4 worldPosition = MODEL * localPosition; 						
	vec4 clipPosition = PROJECTION * VIEW * worldPosition; 				

	passVertexTangent = (MODEL * vec4(TANGENT.xyz, 0.f)).xyz;															

	gl_Position = clipPosition; 
									
})";		


const char* ShaderSource::VERTEX_TANGENT_FS = R"(
	

#version 420 core											
#define MAX_LIGHTS 8
																																												
in vec3 passVertexTangent;

out vec4 outColor;

// Entry point															
void main( void )											
{				
	outColor = vec4((passVertexTangent + vec3(1)) * 0.5f, 1.0f); 				
})";


const RenderState ShaderSource::VERTEX_TANGENT_STATE; // Default state
const unsigned int VERTEX_TANGENT_LAYER = 0;
const SortingQueue VERTEX_TANGENT_QUEUE = SORTING_QUEUE_OPAQUE;


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Vertex Bitangent
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
const char* ShaderSource::VERTEX_BITANGENT_NAME = "Vertex_Bitangent";
const char* ShaderSource::VERTEX_BITANGENT_VS = R"(
	
#version 420 core												

layout(binding=1, std140) uniform cameraUBO
{
	mat4 VIEW;
	mat4 PROJECTION;

	mat4 CAMERA_MATRIX;

	vec3	CAMERA_RIGHT;
	float	PADDING_0;
	vec3	CAMERA_UP;
	float	PADDING_1;
	vec3	CAMERA_FORWARD;
	float	PADDING_2;
	vec3	CAMERA_POSITION;
	float	PADDING_3;
};

layout(binding=2, std140) uniform modelUBO
{
	mat4 MODEL;
};
																												
in vec3 POSITION;
in vec3 NORMAL;													
in vec4 TANGENT;												
																												
out vec3 passVertexBitangent;					
																											
void main( void )												
{						
	vec4 localPosition = vec4(POSITION, 1);																				
	vec4 worldPosition = MODEL * localPosition; 						
	vec4 clipPosition = PROJECTION * VIEW * worldPosition; 				

	vec3 localCross = (cross(TANGENT.xyz, NORMAL) * TANGENT.w);
	passVertexBitangent = (MODEL * vec4(localCross, 0.f)).xyz;															

	gl_Position = clipPosition; 
									
})";	

const char* ShaderSource::VERTEX_BITANGENT_FS = R"(
	

#version 420 core											
#define MAX_LIGHTS 8
																																												
in vec3 passVertexBitangent;

out vec4 outColor;

// Entry point															
void main( void )											
{				
	outColor = vec4((passVertexBitangent + vec3(1)) * 0.5f, 1.0f); 				
})";


const RenderState ShaderSource::VERTEX_BITANGENT_STATE; // Default state


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Surface Normal (Normal map sampling)
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
const char* ShaderSource::SURFACE_NORMAL_NAME = "Surface_Normal";
const char* ShaderSource::SURFACE_NORMAL_VS = R"(
	
#version 420 core												

layout(binding=1, std140) uniform cameraUBO
{
	mat4 VIEW;
	mat4 PROJECTION;

	mat4 CAMERA_MATRIX;

	vec3	CAMERA_RIGHT;
	float	PADDING_0;
	vec3	CAMERA_UP;
	float	PADDING_1;
	vec3	CAMERA_FORWARD;
	float	PADDING_2;
	vec3	CAMERA_POSITION;
	float	PADDING_3;
};

layout(binding=2, std140) uniform modelUBO
{
	mat4 MODEL;
};
																												
in vec3 POSITION;												
in vec2 UV;

out vec2 passUV;																																
																											
void main( void )												
{						
	vec4 localPosition = vec4(POSITION, 1);																				
	vec4 worldPosition = MODEL * localPosition; 						
	vec4 clipPosition = PROJECTION * VIEW * worldPosition; 																			

	passUV = UV;

	gl_Position = clipPosition; 
									
})";	

const char* ShaderSource::SURFACE_NORMAL_FS = R"(
	

#version 420 core											
#define MAX_LIGHTS 8																																											

layout(binding = 1) uniform sampler2D gTexNormal;

in vec2 passUV;
out vec4 outColor;

// Entry point															
void main( void )											
{				
	outColor = texture(gTexNormal, passUV); 				
})";


const RenderState ShaderSource::SURFACE_NORMAL_STATE; // Default state


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// World Normal
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
const char* ShaderSource::WORLD_NORMAL_NAME = "World_Normal";
const char* ShaderSource::WORLD_NORMAL_VS = R"(
	
#version 420 core												

layout(binding=1, std140) uniform cameraUBO
{
	mat4 VIEW;
	mat4 PROJECTION;

	mat4 CAMERA_MATRIX;

	vec3	CAMERA_RIGHT;
	float	PADDING_0;
	vec3	CAMERA_UP;
	float	PADDING_1;
	vec3	CAMERA_FORWARD;
	float	PADDING_2;
	vec3	CAMERA_POSITION;
	float	PADDING_3;
};

layout(binding=2, std140) uniform modelUBO
{
	mat4 MODEL;
};
																												
in vec3 POSITION;																								
in vec2 UV;	
in vec3 NORMAL;	
in vec4 TANGENT;												
																
out vec2 passUV;												
out mat4 passTBNTransform;					
																									
void main( void )												
{						
	vec4 localPosition = vec4(POSITION, 1);																				
	vec4 worldPosition = MODEL * localPosition; 						
	vec4 clipPosition = PROJECTION * VIEW * worldPosition; 				
																
	passUV = UV;											

	// Calculate the TBN transform
	vec3 worldNormal = normalize((MODEL * vec4(NORMAL, 0.f)).xyz);
	vec3 worldTangent = normalize((MODEL * vec4(TANGENT.xyz, 0.f)).xyz);
	vec3 worldBitangent = cross(worldTangent, worldNormal) * TANGENT.w; // Left-handed cross product

	passTBNTransform = mat4(vec4(worldTangent, 0.f), vec4(worldBitangent, 0.f), vec4(worldNormal, 0.f), vec4(worldPosition.xyz, 1.0f));

	gl_Position = clipPosition; 
									
})";


const char* ShaderSource::WORLD_NORMAL_FS = R"(
	

#version 420 core											
#define MAX_LIGHTS 8
																												
layout(binding = 1) uniform sampler2D gTexNormal;	

in vec2 passUV;																						
in mat4 passTBNTransform;

out vec4 outColor;

// Calculates the normal given the surface normal color sample and the TBN transformation
vec3 CalculateWorldNormal(vec4 color)
{
	// Range map the values (Z should be between 0.5 and 1.0 always
	vec3 surfaceNormal = normalize(2.f * color.xyz - vec3(1));
	vec3 worldNormal = (passTBNTransform * vec4(surfaceNormal, 0.f)).xyz;

	return worldNormal;
}

// Entry point															
void main( void )											
{				
	// Get the normal from the normal map, and transform it into TBN space
	vec4 normalColor = texture(gTexNormal, passUV);
	vec3 worldNormal = CalculateWorldNormal(normalColor);	
	
	outColor = vec4((worldNormal + vec3(1)) * 0.5f, 1.0f); 	
			
})";

const RenderState ShaderSource::WORLD_NORMAL_STATE; // Default state


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Ambient + Diffuse lighting (no color)
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
const char* ShaderSource::DIFFUSE_NAME = "Diffuse_Light";
const char* ShaderSource::DIFFUSE_VS = R"(
	
#version 420 core												

layout(binding=1, std140) uniform cameraUBO
{
	mat4 VIEW;
	mat4 PROJECTION;

	mat4 CAMERA_MATRIX;

	vec3	CAMERA_RIGHT;
	float	PADDING_0;
	vec3	CAMERA_UP;
	float	PADDING_1;
	vec3	CAMERA_FORWARD;
	float	PADDING_2;
	vec3	CAMERA_POSITION;
	float	PADDING_3;
};

layout(binding=2, std140) uniform modelUBO
{
	mat4 MODEL;
};
																											
in vec3 POSITION;																								
in vec2 UV;	
in vec3 NORMAL;	
in vec4 TANGENT;												
																
out vec2 passUV;												
out vec3 passWorldPosition; // For determining light direction
out mat4 passTBNTransform;					
out vec3 passEyePosition;
																									
void main( void )												
{						
	vec4 localPosition = vec4(POSITION, 1);																				
	vec4 worldPosition = MODEL * localPosition; 						
	vec4 clipPosition = PROJECTION * VIEW * worldPosition; 				
																
	passUV = UV;												

	passWorldPosition = worldPosition.xyz;

	// Calculate the TBN transform
	vec3 worldNormal = normalize((MODEL * vec4(NORMAL, 0.f)).xyz);
	vec3 worldTangent = normalize((MODEL * vec4(TANGENT.xyz, 0.f)).xyz);
	vec3 worldBitangent = cross(worldTangent, worldNormal) * TANGENT.w;

	passTBNTransform = mat4(vec4(worldTangent, 0.f), vec4(worldBitangent, 0.f), vec4(worldNormal, 0.f), vec4(passWorldPosition, 1.0f));
	passEyePosition = CAMERA_POSITION;

	gl_Position = clipPosition; 
									
})";		

const char* ShaderSource::DIFFUSE_FS = R"(
	

#version 420 core											
#define MAX_LIGHTS 8
																									
layout(binding = 0) uniform sampler2D gTexDiffuse;			
layout(binding = 1) uniform sampler2D gTexNormal;

struct Light
{
	vec3 m_position;
	float m_dotOuterAngle;
	vec3 m_direction;
	float m_dotInnerAngle;
	vec3 m_attenuationFactors;
	float m_directionFactor;
	vec4 m_color;
	mat4 m_shadowVP;
	vec3 m_padding;
	float m_castsShadows;
};

layout(binding=3, std140) uniform lightUBO
{
	vec4 AMBIENT;							// xyz color, w intensity
	Light LIGHTS[MAX_LIGHTS];
};	

in vec2 passUV;																						

in vec3 passEyePosition;
in vec3 passWorldPosition;
in mat4 passTBNTransform;

out vec4 outColor;

//---------------------------------------------------Functions-----------------------------------------------------------------

// Calculates the normal given the surface normal color sample and the TBN transformation
vec3 CalculateWorldNormal(vec4 color)
{
	// Range map the values (Z should be between 0.5 and 1.0 always
	vec3 surfaceNormal = normalize(2.f * color.xyz - vec3(1));
	vec3 worldNormal = (passTBNTransform * vec4(surfaceNormal, 0.f)).xyz;

	return worldNormal;
}

// Calculates the attenuation factor for the given light information
float CalculateAttenuation(vec3 lightPosition, vec3 attenuationFactors, float intensity)
{
	float distance = length(lightPosition - passWorldPosition);
	float denominator = attenuationFactors.x + attenuationFactors.y * distance + attenuationFactors.z * distance * distance;
	float attenuation = (intensity / denominator);

	return attenuation;
} 	


// Calculates the cone factor (= 1.f for directional and point lights, 0.f <= x <= 1.f for spot lights)
float CalculateConeFactor(vec3 lightPosition, vec3 lightDirection, float outerDotThreshold, float innerDotThreshold)
{
	float dotFactor = dot(normalize(passWorldPosition - lightPosition), lightDirection);
	float coneFactor = smoothstep(outerDotThreshold, innerDotThreshold, dotFactor);

	return coneFactor;
}			


// Calculates the diffuse factor for the 
vec3 CalculateDot3(vec3 directionToLight, vec3 normal, vec4 lightColor, float attenuation, float coneFactor)
{
	vec3 dot3 = (max(0.f, dot(directionToLight, normal)) * lightColor.xyz * lightColor.w * attenuation * coneFactor);
	return dot3;
}


// Entry point															
void main( void )											
{				
	//----------------------------SET UP VALUES-------------------------------		
	vec3 directionToEye = normalize(passEyePosition - passWorldPosition);

	// Get the normal from the normal map, and transform it into TBN space
	vec4 normalColor = texture(gTexNormal, passUV);
	vec3 worldNormal = CalculateWorldNormal(normalColor);	

	// Calculate the direction TO light, attenuation, and cone factor for each light
	// Set up accumulation variables
	vec3 surfaceLight = vec3(0);	// How much light is hitting the surface

	//----------Add in the ambient light to the surface light----------
	surfaceLight = AMBIENT.xyz * AMBIENT.w;

	for (int lightIndex = 0; lightIndex < MAX_LIGHTS; ++lightIndex)
	{
		// Directions to the light
		vec3 directionToLight = mix(-LIGHTS[lightIndex].m_direction, normalize(LIGHTS[lightIndex].m_position - passWorldPosition), LIGHTS[lightIndex].m_directionFactor);

		// Attenuation
		float attenuation = CalculateAttenuation(LIGHTS[lightIndex].m_position, LIGHTS[lightIndex].m_attenuationFactors, LIGHTS[lightIndex].m_color.w);

		// Cone factor
		float coneFactor = CalculateConeFactor(LIGHTS[lightIndex].m_position, LIGHTS[lightIndex].m_direction, LIGHTS[lightIndex].m_dotOuterAngle, LIGHTS[lightIndex].m_dotInnerAngle);


		//-------------Add in the diffuse light from all lights------------	
		surfaceLight += CalculateDot3(directionToLight, worldNormal, LIGHTS[lightIndex].m_color, attenuation, coneFactor);
	}


	// Clamp the surface light, since it alone shouldn't blow out the surface (either it's fully lit, or not fully lit)
	surfaceLight = clamp(surfaceLight, vec3(0), vec3(1));


	//---------------------CALCULATE FINAL COLOR-----------------------
	// Calculate the final color, surface gets w = 1 since it is multiplied in, reflected gets w = 0 since it is added in
	vec4 finalColor = vec4(surfaceLight, 1);

	// Clamp the color
	finalColor = clamp(finalColor, vec4(0), vec4(1));
	
	outColor = finalColor; 			
	
})";

const RenderState ShaderSource::DIFFUSE_STATE; // Default values


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Specular lighting (no color)
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
const char* ShaderSource::SPECULAR_NAME = "Specular_Light";
const char* ShaderSource::SPECULAR_VS =  R"(
	
#version 420 core												

layout(binding=1, std140) uniform cameraUBO
{
	mat4 VIEW;
	mat4 PROJECTION;

	mat4 CAMERA_MATRIX;

	vec3	CAMERA_RIGHT;
	float	PADDING_0;
	vec3	CAMERA_UP;
	float	PADDING_1;
	vec3	CAMERA_FORWARD;
	float	PADDING_2;
	vec3	CAMERA_POSITION;
	float	PADDING_3;
};

layout(binding=2, std140) uniform modelUBO
{
	mat4 MODEL;
};
																												
in vec3 POSITION;																								
in vec2 UV;	
in vec3 NORMAL;	
in vec4 TANGENT;												
																
out vec2 passUV;												
out vec3 passWorldPosition; // For determining light direction
out mat4 passTBNTransform;					
out vec3 passEyePosition;
																								
void main( void )												
{						
	vec4 localPosition = vec4(POSITION, 1);																				
	vec4 worldPosition = MODEL * localPosition; 						
	vec4 clipPosition = PROJECTION * VIEW * worldPosition; 				
																
	passUV = UV;												

	passWorldPosition = worldPosition.xyz;

	// Calculate the TBN transform
	vec3 worldNormal = normalize((MODEL * vec4(NORMAL, 0.f)).xyz);
	vec3 worldTangent = normalize((MODEL * vec4(TANGENT.xyz, 0.f)).xyz);
	vec3 worldBitangent = cross(worldTangent, worldNormal) * TANGENT.w;

	passTBNTransform = mat4(vec4(worldTangent, 0.f), vec4(worldBitangent, 0.f), vec4(worldNormal, 0.f), vec4(passWorldPosition, 1.0f));
	passEyePosition = CAMERA_POSITION;

	gl_Position = clipPosition; 
									
})";

const char* ShaderSource::SPECULAR_FS = R"(
	

#version 420 core											
#define MAX_LIGHTS 8
																									
layout(binding = 0) uniform sampler2D gTexDiffuse;			
layout(binding = 1) uniform sampler2D gTexNormal;

struct Light
{
	vec3 m_position;
	float m_dotOuterAngle;
	vec3 m_direction;
	float m_dotInnerAngle;
	vec3 m_attenuationFactors;
	float m_directionFactor;
	vec4 m_color;
	mat4 m_shadowVP;
	vec3 m_padding;
	float m_castsShadows;
};

layout(binding=3, std140) uniform lightUBO
{
	vec4 AMBIENT;							// xyz color, w intensity
	Light LIGHTS[MAX_LIGHTS];
};	

layout(binding=8, std140) uniform specularUBO
{
	float SPECULAR_AMOUNT;
	float SPECULAR_POWER;
	vec2 PADDING_4;
};

in vec2 passUV;																							

in vec3 passEyePosition;
in vec3 passWorldPosition;
in mat4 passTBNTransform;

out vec4 outColor;

//---------------------------------------------------Functions-----------------------------------------------------------------

// Calculates the normal given the surface normal color sample and the TBN transformation
vec3 CalculateWorldNormal(vec4 color)
{
	// Range map the values (Z should be between 0.5 and 1.0 always
	vec3 surfaceNormal = normalize(2.f * color.xyz - vec3(1));
	vec3 worldNormal = (passTBNTransform * vec4(surfaceNormal, 0.f)).xyz;

	return worldNormal;
}

// Calculates the attenuation factor for the given light information
float CalculateAttenuation(vec3 lightPosition, vec3 attenuationFactors, float intensity)
{
	float distance = length(lightPosition - passWorldPosition);
	float denominator = attenuationFactors.x + attenuationFactors.y * distance + attenuationFactors.z * distance * distance;
	float attenuation = (intensity / denominator);

	return attenuation;
} 	


// Calculates the cone factor (= 1.f for directional and point lights, 0.f <= x <= 1.f for spot lights)
float CalculateConeFactor(vec3 lightPosition, vec3 lightDirection, float outerDotThreshold, float innerDotThreshold)
{
	float dotFactor = dot(normalize(passWorldPosition - lightPosition), lightDirection);
	float coneFactor = smoothstep(outerDotThreshold, innerDotThreshold, dotFactor);

	return coneFactor;
}			

vec3 CalculateSpecular(vec3 directionToLight, vec3 normal, vec3 directionToEye, vec4 lightColor, float attenuation, float coneFactor)
{
	vec3 reflect = reflect(-directionToLight, normal);
	float factor = max(0, dot(directionToEye, reflect));
	vec3 specular = SPECULAR_AMOUNT * pow(factor, SPECULAR_POWER) * lightColor.xyz * lightColor.w * attenuation * coneFactor;
	
	return specular;
}


// Entry point															
void main( void )											
{				
	//----------------------------SET UP VALUES-------------------------------		
	vec3 directionToEye = normalize(passEyePosition - passWorldPosition);

	// Get the normal from the normal map, and transform it into TBN space
	vec4 normalColor = texture(gTexNormal, passUV);
	vec3 worldNormal = CalculateWorldNormal(normalColor);	

	// Calculate the direction TO light, attenuation, and cone factor for each light
	// Set up accumulation variables
	vec3 reflectedLight = vec3(0);	// How much light is being reflected back

	for (int lightIndex = 0; lightIndex < MAX_LIGHTS; ++lightIndex)
	{
		// Directions to the light
		vec3 directionToLight = mix(-LIGHTS[lightIndex].m_direction, normalize(LIGHTS[lightIndex].m_position - passWorldPosition), LIGHTS[lightIndex].m_directionFactor);

		// Attenuation
		float attenuation = CalculateAttenuation(LIGHTS[lightIndex].m_position, LIGHTS[lightIndex].m_attenuationFactors, LIGHTS[lightIndex].m_color.w);

		// Cone factor
		float coneFactor = CalculateConeFactor(LIGHTS[lightIndex].m_position, LIGHTS[lightIndex].m_direction, LIGHTS[lightIndex].m_dotOuterAngle, LIGHTS[lightIndex].m_dotInnerAngle);
		
		//-----Calculate and add in specular lighting from all lights----------
		reflectedLight += CalculateSpecular(directionToLight, worldNormal, directionToEye, LIGHTS[lightIndex].m_color, attenuation, coneFactor);
	}
	
	outColor = vec4(reflectedLight, 1.f);
 				
})";

const RenderState ShaderSource::SPECULAR_STATE; // Default state
const unsigned int SPECULAR_LAYER = 0;
const SortingQueue SPECULAR_QUEUE = SORTING_QUEUE_OPAQUE;


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// All lighting (no color)
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
const char* ShaderSource::LIGHTING_NAME = "Lighting_Only";
const char* ShaderSource::LIGHTING_VS = R"(
	
#version 420 core												

layout(binding=1, std140) uniform cameraUBO
{
	mat4 VIEW;
	mat4 PROJECTION;

	mat4 CAMERA_MATRIX;

	vec3	CAMERA_RIGHT;
	float	PADDING_0;
	vec3	CAMERA_UP;
	float	PADDING_1;
	vec3	CAMERA_FORWARD;
	float	PADDING_2;
	vec3	CAMERA_POSITION;
	float	PADDING_3;
};

layout(binding=2, std140) uniform modelUBO
{
	mat4 MODEL;
};
																												
in vec3 POSITION;																									
in vec2 UV;	
in vec3 NORMAL;	
in vec4 TANGENT;												
																
out vec2 passUV;												
out vec3 passWorldPosition; // For determining light direction
out mat4 passTBNTransform;					
out vec3 passEyePosition;

																									
void main( void )												
{						
	vec4 localPosition = vec4(POSITION, 1);																				
	vec4 worldPosition = MODEL * localPosition; 						
	vec4 clipPosition = PROJECTION * VIEW * worldPosition; 				
																
	passUV = UV;												

	passWorldPosition = worldPosition.xyz;

	// Calculate the TBN transform
	vec3 worldNormal = normalize((MODEL * vec4(NORMAL, 0.f)).xyz);
	vec3 worldTangent = normalize((MODEL * vec4(TANGENT.xyz, 0.f)).xyz);
	vec3 worldBitangent = cross(worldTangent, worldNormal) * TANGENT.w;

	passTBNTransform = mat4(vec4(worldTangent, 0.f), vec4(worldBitangent, 0.f), vec4(worldNormal, 0.f), vec4(passWorldPosition, 1.0f));
	passEyePosition = CAMERA_POSITION;

	gl_Position = clipPosition; 
									
})";			

const char* ShaderSource::LIGHTING_FS = R"(
	
#version 420 core											
#define MAX_LIGHTS 8
																									
layout(binding = 0) uniform sampler2D gTexDiffuse;			
layout(binding = 1) uniform sampler2D gTexNormal;

struct Light
{
	vec3 m_position;
	float m_dotOuterAngle;
	vec3 m_direction;
	float m_dotInnerAngle;
	vec3 m_attenuationFactors;
	float m_directionFactor;
	vec4 m_color;
	mat4 m_shadowVP;
	vec3 m_padding;
	float m_castsShadows;
};

layout(binding=3, std140) uniform lightUBO
{
	vec4 AMBIENT;							// xyz color, w intensity
	Light LIGHTS[MAX_LIGHTS];
};	

layout(binding=8, std140) uniform specularUBO
{
	float SPECULAR_AMOUNT;
	float SPECULAR_POWER;
	vec2 PADDING_4;
};

in vec2 passUV;																						

in vec3 passEyePosition;
in vec3 passWorldPosition;
in mat4 passTBNTransform;

out vec4 outColor;

//---------------------------------------------------Functions-----------------------------------------------------------------

// Calculates the normal given the surface normal color sample and the TBN transformation
vec3 CalculateWorldNormal(vec4 color)
{
	// Range map the values (Z should be between 0.5 and 1.0 always
	vec3 surfaceNormal = normalize(2.f * color.xyz - vec3(1));
	vec3 worldNormal = (passTBNTransform * vec4(surfaceNormal, 0.f)).xyz;

	return worldNormal;
}

// Calculates the attenuation factor for the given light information
float CalculateAttenuation(vec3 lightPosition, vec3 attenuationFactors, float intensity)
{
	float distance = length(lightPosition - passWorldPosition);
	float denominator = attenuationFactors.x + attenuationFactors.y * distance + attenuationFactors.z * distance * distance;
	float attenuation = (intensity / denominator);

	return attenuation;
} 	


// Calculates the cone factor (= 1.f for directional and point lights, 0.f <= x <= 1.f for spot lights)
float CalculateConeFactor(vec3 lightPosition, vec3 lightDirection, float outerDotThreshold, float innerDotThreshold)
{
	float dotFactor = dot(normalize(passWorldPosition - lightPosition), lightDirection);
	float coneFactor = smoothstep(outerDotThreshold, innerDotThreshold, dotFactor);

	return coneFactor;
}			


// Calculates the diffuse factor for the 
vec3 CalculateDot3(vec3 directionToLight, vec3 normal, vec4 lightColor, float attenuation, float coneFactor)
{
	vec3 dot3 = (max(0.f, dot(directionToLight, normal)) * lightColor.xyz * lightColor.w * attenuation * coneFactor);
	return dot3;
}

vec3 CalculateSpecular(vec3 directionToLight, vec3 normal, vec3 directionToEye, vec4 lightColor, float attenuation, float coneFactor)
{
	vec3 reflect = reflect(-directionToLight, normal);
	float factor = max(0, dot(directionToEye, reflect));
	vec3 specular = SPECULAR_AMOUNT * pow(factor, SPECULAR_POWER) * lightColor.xyz * lightColor.w * attenuation * coneFactor;
	
	return specular;
}

// Entry point															
void main( void )											
{				
	//----------------------------SET UP VALUES-------------------------------		
	vec3 directionToEye = normalize(passEyePosition - passWorldPosition);

	// Get the normal from the normal map, and transform it into TBN space
	vec4 normalColor = texture(gTexNormal, passUV);
	vec3 worldNormal = CalculateWorldNormal(normalColor);	

	// Calculate the direction TO light, attenuation, and cone factor for each light
	// Set up accumulation variables
	vec3 surfaceLight = vec3(0);	// How much light is hitting the surface
	vec3 reflectedLight = vec3(0);	// How much light is being reflected back

	//----------STEP 1: Add in the ambient light to the surface light----------
	surfaceLight = AMBIENT.xyz * AMBIENT.w;

	for (int lightIndex = 0; lightIndex < MAX_LIGHTS; ++lightIndex)
	{
		// Directions to the light
		vec3 directionToLight = mix(-LIGHTS[lightIndex].m_direction, normalize(LIGHTS[lightIndex].m_position - passWorldPosition), LIGHTS[lightIndex].m_directionFactor);

		// Attenuation
		float attenuation = CalculateAttenuation(LIGHTS[lightIndex].m_position, LIGHTS[lightIndex].m_attenuationFactors, LIGHTS[lightIndex].m_color.w);

		// Cone factor
		float coneFactor = CalculateConeFactor(LIGHTS[lightIndex].m_position, LIGHTS[lightIndex].m_direction, LIGHTS[lightIndex].m_dotOuterAngle, LIGHTS[lightIndex].m_dotInnerAngle);


		//-------------STEP 2: Add in the diffuse light from all lights------------	
		surfaceLight += CalculateDot3(directionToLight, worldNormal, LIGHTS[lightIndex].m_color, attenuation, coneFactor);
		
		//-----STEP 3: Calculate and add in specular lighting from all lights----------
		reflectedLight += CalculateSpecular(directionToLight, worldNormal, directionToEye, LIGHTS[lightIndex].m_color, attenuation, coneFactor);
	}


	// Clamp the surface light, since it alone shouldn't blow out the surface (either it's fully lit, or not fully lit)
	surfaceLight = clamp(surfaceLight, vec3(0), vec3(1));


	//---------------------STEP 4: CALCULATE FINAL COLOR-----------------------
	// Calculate the final color, surface gets w = 1 since it is multiplied in, reflected gets w = 0 since it is added in
	vec4 finalColor = vec4(surfaceLight, 1) + vec4(reflectedLight, 0);

	// Clamp the color
	finalColor = clamp(finalColor, vec4(0), vec4(1));
	
	outColor = finalColor; 				
})";

const RenderState ShaderSource::LIGHTING_STATE; // Default state


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// UV visualizer
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
const char* ShaderSource::UV_NAME = "UV";
const char* ShaderSource::UV_VS = R"(

#version 420 core												

layout(binding=1, std140) uniform cameraUBO
{
	mat4 VIEW;
	mat4 PROJECTION;
};

layout(binding=2, std140) uniform modelUBO
{
	mat4 MODEL;
};
																												
in vec3 POSITION;												
in vec4 COLOR;													
in vec2 UV;														
																
out vec2 passUV;																							
																													
void main( void )												
{																										
	vec4 world_pos = vec4( POSITION, 1 ); 						
	vec4 clip_pos = PROJECTION * VIEW * MODEL * world_pos; 				
																
	passUV = UV;																							
	gl_Position = clip_pos; 								
})";	

const char* ShaderSource::UV_FS = R"(
	
#version 420 core											
																										
in vec2 passUV;												
															  										
layout(binding = 0) uniform sampler2D gTexDiffuse;			
																											
out vec4 outColor; 											
															
// Entry Point												
void main( void )											
{																																				
	outColor = vec4(passUV, 0.f, 1.f); 				
})";

const RenderState ShaderSource::UV_STATE; // Default state