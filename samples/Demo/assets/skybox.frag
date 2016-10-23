#version 150

uniform samplerCube uSampler;
in vec3				vNormal;
out vec4 			oColor;

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

void main()
{
	vec3 skybox = pow( texture( uSampler, vNormal ).rgb, vec3( 1.0 ) );
	vec3 color 	= skybox;

	// apply the tone-mapping
	color		= Uncharted2Tonemap( color );
	// white balance
	color		= color * ( 1.0f / Uncharted2Tonemap( vec3( 1.5f ) ) );
	// gamma correction 
	color 		= pow( color, vec3( 1.0 / 2.2 ) );
	oColor 		= vec4( color, 1.0f );
}