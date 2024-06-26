// ------------------------------------------------------------------ 
// | *** THIS FILE IS GENERATED USING TEMPORAL SHADER GENERATION *** | 
// ------------------------------------------------------------------- 

#version 450

layout(location=0) in vec3 Position;
layout(location=1) in vec3 Color;


layout(location=0) out vec4 FragColor; 

void main()
{
	gl_Position = vec4(Position, 1.0);
	FragColor = vec4(Color, 1.0);
}
