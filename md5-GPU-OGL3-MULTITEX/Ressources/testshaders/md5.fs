#version 330 core

struct s_material
{
    vec4 ambient, diffuse, specular;
    float shininess;
};
struct s_light
{
    vec4 position;
    vec4 ambient, diffuse, specular;
};

//uniform s_light lighting;
//uniform s_material material;

in vec2 texCoords;
in vec3 ts_lightDir, ts_eyeVec;

uniform sampler2D diffuseTex;
uniform sampler2D normalMap;
uniform sampler2D heightMap;

void main()
{
	s_light lighting;
	lighting.ambient = vec4(0.0,0.0,0.0,1.0);
    lighting.diffuse = vec4(1.0,1.0,1.0,1.0);
    lighting.specular = vec4(1.0,1.0,1.0,1.0);
	
	vec3 ts_texNormal = normalize(texture2D( normalMap, texCoords ).rgb*2.0 - 1.0);

	s_material material;
	material.ambient = texture2D(diffuseTex, texCoords);
	//material.diffuse  = texture2D(heightMap, texCoords);	
	//material.specular = texture2D(normalMap, texCoords);
	//material.ambient = vec4(0.5,0.5,0.5,1.0);
	material.diffuse = material.ambient;
	material.specular = vec4(0.2,0.2,0.2,1.0);
	material.shininess = 60.0;
	
	vec4 final_color =
	(gl_FrontLightModelProduct.sceneColor * material.ambient) +
	(lighting.ambient * material.ambient);

	vec3 N = normalize(ts_texNormal);
	vec3 L = normalize(ts_lightDir);

	float lambertTerm = dot(N,L);

	if(lambertTerm > 0.0)
	{
		final_color += lighting.diffuse *
		               material.diffuse *
					   lambertTerm;

		vec3 V = normalize(ts_eyeVec);

		vec3 R = reflect(-L, N);
		/*float specular = pow( max(dot(R, E), 0.0),
		                 material.shininess );*/
        float specular = pow(max(dot(R,V),0.0), material.shininess);
		                 //specular = 0.0;
		final_color += lighting.specular *
		               material.specular *
					   specular;
	}

	gl_FragColor = final_color;
}