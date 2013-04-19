#version 330 core

#ifndef MAX_BONES
#define MAX_BONES 58
#endif 

#define aID_position 0
#define aID_normal 1
#define aID_UV 2
#define aID_matrixIndex 3
#define aID_weight 4

layout(location = aID_position) in vec4 a_position;//in model space
layout(location = aID_UV) in vec4 a_UV;
layout(location = aID_matrixIndex) in vec4 a_matrixIndex;
layout(location = aID_weight) in vec4 a_weight;

uniform sampler2D baseTexture;
uniform mat4 gBoneMatrix[MAX_BONES];

out vec2 texCoords;

void main()
{
	mat4 matTransform = gBoneMatrix[int(a_matrixIndex.x)] * a_weight.x;
    matTransform += gBoneMatrix[int(a_matrixIndex.y)] * a_weight.y;
    matTransform += gBoneMatrix[int(a_matrixIndex.z)] * a_weight.z;
    float finalWeight = 1.0f - ( a_weight.x + a_weight.y + a_weight.z );
    matTransform += gBoneMatrix[int(a_matrixIndex.w)] * finalWeight;
 
    vec4 objPos = matTransform *  a_position;
    //objNormal = matTransform * vec4( normal, 0.0 ) );
 
    gl_Position = gl_ModelViewProjectionMatrix * objPos ;
	
	texCoords = a_UV;
	
	gl_ClipVertex = gl_ModelViewMatrix * a_position;
	
}