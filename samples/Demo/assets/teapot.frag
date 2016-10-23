#version 150

uniform samplerCube uPmremSampler;
uniform samplerCube uIemSampler;
uniform sampler2D 	uRoughnessSampler;
uniform sampler2D 	uMetallicSampler;
uniform sampler2D 	uNormalSampler;
uniform sampler2D 	uBaseColorSampler;

uniform vec3 		uCameraPos;

in vec3				vPosition;
in vec3				vNormal;
in vec2				vUv;

out vec4 			oColor;

vec3 fresnel( vec3 reflectivity, float NoV, float strength )
{
    return reflectivity + ( 1.0 - reflectivity ) * pow( 1.0 - NoV, 5.0 ) * strength;
}

// ShaderX5: Normal Mapping without Precomputed Tangents
// http://www.slideshare.net/KyuseokHwang/shaderx5-26normalmappingwithoutprecomputedtangents-130318-1
// http://www.thetenthplanet.de/archives/1180
// http://www.gamedev.net/topic/521915-normal-mapping-without-precomputed-tangents/
// http://www.pouet.net/topic.php?which=6266
mat3 calculateCotangentFrame( vec3 N, vec3 p, vec2 uv )
{
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx( p );
    vec3 dp2 = dFdy( p );
    vec2 duv1 = dFdx( uv );
    vec2 duv2 = dFdy( uv );
 
    // solve the linear system
    vec3 dp2perp = cross( dp2, N );
    vec3 dp1perp = cross( N, dp1 );
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
    // construct a scale-invariant frame 
    float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
    return mat3( T * invmax, B * invmax, N );
}


// From Epic
vec3 calculateEnvBRDFApprox( vec3 SpecularColor, float Roughness, float NoV )
{
	const vec4 c0 = vec4( -1, -0.0275, -0.572, 0.022 );
	const vec4 c1 = vec4( 1, 0.0425, 1.04, -0.04 );
	vec4 r = Roughness * c0 + c1;
	float a004 = min( r.x * r.x, exp2( -9.28 * NoV ) ) * r.x + r.y;
	vec2 AB = vec2( -1.04, 1.04 ) * a004 + r.zw;
	return SpecularColor * AB.x + AB.y;
}



// Filmic tonemapping from
// http://filmicgames.com/archives/75

const float A = 0.15;
const float B = 0.50;
const float C = 0.10;
const float D = 0.20;
const float E = 0.02;
const float F = 0.20;

vec3 Uncharted2Tonemap( vec3 x )
{
	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

const float mipCount = 7.0;

void main( void )
{
	vec3 baseColor 			= pow( texture( uBaseColorSampler, vUv ).rgb, vec3( 2.2 ) );
    float roughness 		= texture( uRoughnessSampler, vUv ).x;
    float metallic 			= texture( uMetallicSampler, vUv ).x;

	vec3 specularColor 		= mix( vec3( 0.04 ), baseColor, metallic );
	vec3 diffuseColor 		= baseColor - baseColor * metallic;
	vec3 normalSample 		= normalize( texture( uNormalSampler, vUv ).xyz * 2.0 - 1.0 );

	vec3 V 					= normalize( uCameraPos-vPosition );
	vec3 N 					= normalize( calculateCotangentFrame( normalize( vNormal ), vPosition, vUv ) * normalSample );
    vec3 R 					= -reflect( V, N );
    float NoV 				= clamp( dot( N, V ), 0.0, 1.0 );

	//float mip 				= ( roughness ) * ( mipCount - 1.0 );
	float mip 				= mipCount - 1.0 - ( 1.0 - log2( roughness ) );
	vec3 fresnel 			= fresnel( specularColor, NoV, 1.0 - roughness );
	vec3 radiance   		= fresnel * pow( textureLod( uPmremSampler, R, mip ).xyz, vec3( 1.0 ) );
	vec3 irradiance 		= diffuseColor * pow( texture( uIemSampler, vNormal ).xyz, vec3( 1.0 ) );

	vec3 color 				= irradiance + radiance;


	// apply the tone-mapping
	color					= Uncharted2Tonemap( color );
	// white balance
	color					= color * ( 1.0f / Uncharted2Tonemap( vec3( 1.5f ) ) );
	// gamma correction 
	color 					= pow( color, vec3( 1.0 / 2.2 ) );

	oColor 					= vec4( color, 1.0f );
	//oColor = vec4( vec3( mip / 6.0 ), 1.0 );
	//oColor = vec4(  );
}