#version 150

uniform mat4	ciModelViewProjection;

in vec4			ciPosition;
in vec3			ciNormal;
in vec2 		ciTexCoord0;

out vec3 		vPosition;
out vec3		vNormal;
out vec2		vUv;

void main( void )
{
	vPosition	= ciPosition.xyz;
	vNormal 	= ciNormal;
	vUv 		= ciTexCoord0;
	gl_Position = ciModelViewProjection * ciPosition;
}
