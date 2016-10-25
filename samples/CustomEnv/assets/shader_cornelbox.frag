#version 150

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

uniform sampler2D 	uSampler;
in vec3				vPosition;
in vec2				vUv;

uniform float 		uExposure;
uniform float 		uWhiteLevel;
uniform bool 		uApplyGammaCorrection;

out vec4 			oColor;


void main( void )
{
	// sample texture
	vec3 color 	= texture( uSampler, vUv ).rgb;

	// tone-mapping and gamma correction
	color	= Uncharted2Tonemap( color * uExposure );
	color	= color * ( 1.0f / Uncharted2Tonemap( vec3( uWhiteLevel ) ) );
	if( uApplyGammaCorrection ) {
		color	= pow( color, vec3( 1.0 / 2.2 ) );
	}
	oColor 		= vec4( color, 1.0f );
}