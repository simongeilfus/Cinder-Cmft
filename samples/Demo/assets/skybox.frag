#version 150

uniform samplerCube uSampler;
in vec3				vNormal;
out vec4 			oColor;

void main()
{
	oColor = texture( uSampler, vNormal );
}