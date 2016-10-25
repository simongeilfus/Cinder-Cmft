#version 150

// Basic fresnel function
vec3 fresnel( vec3 reflectivity, float NoV, float strength )
{
    return reflectivity + ( 1.0 - reflectivity ) * pow( 1.0 - NoV, 5.0 ) * strength;
}

// Filmic tonemapping from
// http://filmicgames.com/archives/75
vec3 Uncharted2Tonemap( vec3 x )
{
	const float A = 0.15;
	const float B = 0.50;
	const float C = 0.10;
	const float D = 0.20;
	const float E = 0.02;
	const float F = 0.20;
	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

uniform samplerCube uPmremSampler;
uniform samplerCube uIemSampler;

uniform float 		uRoughness;
uniform float 		uMetallic;
uniform float 		uExposure;
uniform float 		uWhiteLevel;
uniform vec3 		uCameraPos;
uniform bool 		uIsCornelBox;

in vec3				vPosition;
in vec3				vNormal;
in vec2				vUv;

out vec4 			oColor;


void main( void )
{
	// sample the material textures
	vec3 baseColor 			= vec3(1.0);
    float roughness 		= uRoughness;
    float metallic 			= uMetallic;

    // deduce the specular and diffuse colors from the base color and "metalleness"
	vec3 specularColor 		= mix( vec3( 0.04 ), baseColor, metallic );
	vec3 diffuseColor 		= baseColor - baseColor * metallic;

	// get the eye, normal and reflection vectors
	vec3 N 					= normalize( vNormal );
	vec3 V 					= normalize( uCameraPos-vPosition );
    vec3 R 					= -reflect( V, N );
    float NoV 				= clamp( dot( N, V ), 0.0, 1.0 );

    // cheap NoL from cornel box light
    vec3 L 					= normalize( vec3(0,1,0) );
    float NoL 				= clamp( dot( N, L ), 0.0, 1.0 );

    // sample the environment maps
	const float mipCount 	= 7.0;
	float mip 				= mipCount - 1.0 - ( 1.0 - log2( roughness ) );
	vec3 fresnel 			= fresnel( specularColor, NoV, 1.0 - ( roughness * roughness ) );
	vec3 radiance   		= fresnel * textureLod( uPmremSampler, R, mip ).xyz;
	vec3 irradiance 		= diffuseColor * texture( uIemSampler, vNormal ).xyz;

	// if it's the cornel box fake some cheap directional lighting
	if( uIsCornelBox ) {
		irradiance			*= NoL * 0.5 + 0.5 * vec3( 1.0, 0.89, 0.7 );
		irradiance			+= vec3( NoL ) * 0.5 * vec3( 1.0, 0.89, 0.7 );
	}
	
	vec3 color 				= irradiance + radiance;

	// tone-mapping and gamma correction
	color					= Uncharted2Tonemap( color * uExposure );
	color					= color * ( 1.0f / Uncharted2Tonemap( vec3( uWhiteLevel ) ) );
	color 					= pow( color, vec3( 1.0 / 2.2 ) );

	oColor 					= vec4( color, 1.0f );

}