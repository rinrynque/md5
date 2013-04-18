#ifndef MAX_BONES
#define MAX_BONES 58
#endif 
//attribute vec4 weight;

uniform sampler2D baseTexture;
uniform mat4 gBoneMatrix[MAX_BONES];

varying vec2 texCoords;

void main()
{
	vec4 matrixIndex = gl_MultiTexCoord2;
	vec4 weight = gl_MultiTexCoord1;
	mat4 matTransform = gBoneMatrix[int(matrixIndex.x)] * weight.x;
    matTransform += gBoneMatrix[int(matrixIndex.y)] * weight.y;
    matTransform += gBoneMatrix[int(matrixIndex.z)] * weight.z;
    float finalWeight = 1.0f - ( weight.x + weight.y + weight.z );
    matTransform += gBoneMatrix[int(matrixIndex.w)] * finalWeight;
 
    vec4 objPos = matTransform *  gl_Vertex;
    //objNormal = matTransform * vec4( normal, 0.0 ) );
 
    gl_Position = gl_ModelViewProjectionMatrix * objPos ;
	
	texCoords = gl_MultiTexCoord0;
	
	gl_FrontColor.r =  finalWeight;
	gl_FrontColor.g	= finalWeight;
	gl_FrontColor.b = finalWeight;
	
	gl_ClipVertex = gl_ModelViewMatrix * gl_Position;
	
}