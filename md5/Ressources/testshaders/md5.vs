#version 330 core

#ifndef MAX_BONES
#define MAX_BONES 58
#endif 

#define aID_position 0
#define aID_normal 1
#define aID_tangent 2
#define aID_bitangent 3
#define aID_UV 4
#define aID_matrixIndex 5
#define aID_weight 6

layout(location = aID_position) in vec4 a_position;//in model space
layout(location = aID_normal) in vec3 a_normal;
layout(location = aID_tangent) in vec3 a_tangent;
layout(location = aID_bitangent) in vec3 a_bitangent;
layout(location = aID_UV) in vec4 a_UV;
layout(location = aID_matrixIndex) in vec4 a_matrixIndex;
layout(location = aID_weight) in vec4 a_weight;


struct s_light
{
    vec4 position;
    vec4 ambient, diffuse, specular;
};

uniform s_light lighting;
uniform mat4 gBoneMatrix[MAX_BONES];

out vec2 texCoords;
out vec3 ts_lightDir, ts_eyeVec;

void main()
{
	/*lighting.ambient = vec4(0.0,0.0,0.0,0.0);
    lighting.diffuse = vec4(1.0,1.0,1.0,1.0);
    lighting.specular = vec4(1.0,1.0,1.0,1.0);*/
	
	mat4 matTransform = gBoneMatrix[int(a_matrixIndex.x)] * a_weight.x;
    matTransform += gBoneMatrix[int(a_matrixIndex.y)] * a_weight.y;
    matTransform += gBoneMatrix[int(a_matrixIndex.z)] * a_weight.z;
    float finalWeight = 1.0f - ( a_weight.x + a_weight.y + a_weight.z );
    matTransform += gBoneMatrix[int(a_matrixIndex.w)] * finalWeight;
 
    vec4 objPos = matTransform *  a_position;
    gl_Position = gl_ModelViewProjectionMatrix * objPos ;
	
	vec4 ws_lightPos = lighting.position;
	
	texCoords = a_UV;
	vec3 ws_normal = normalize(gl_NormalMatrix*mat3(matTransform)*a_normal);

	mat3 TBN = transpose(mat3(
        gl_ModelViewMatrix*a_tangent,
        gl_ModelViewMatrix*a_bitangent,
        // gl_ModelViewMatrix*a_normal
    ));
	vec4 ts_lightPos = TBN*(gl_ModelViewMatrix*ws_lightPos);
	vec3 ts_vVertex = vec3(TBN*(gl_ModelViewMatrix * objPos));
	
	if(lighting.position[3]==0)
		lightDir = vec3((lightin.position).xyz);
	else
		lightDir = vec3((lightin.position).xyz - vVertex);
		
	vec4 ts_lightDir = TBN*lightDir;		
	vec3 eyeVec = -vVertex;
	ts_eyeVec = TBN*eyeVec;
	
	gl_ClipVertex = gl_ModelViewMatrix * a_position;
//gl_FrontColor = vec4(a_normal,0);	
}