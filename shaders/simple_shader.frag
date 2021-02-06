#version 450

layout (location = 0) in vec3 inColor;

//output write
layout (location = 0) out vec4 outFragColor;


void main() 
{
	//return color
	outFragColor = vec4(inColor, 1.0f);
	// outFragColor = vec4(1.0, 1.0, 0.0, 1.0f);
}
