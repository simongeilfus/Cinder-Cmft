#version 150

uniform mat4	ciModelViewProjection;
in vec4			ciPosition;
out vec3		vNormal;

void main()
{
	vNormal = vec3( ciPosition );
	gl_Position = ciModelViewProjection * ciPosition;
}
