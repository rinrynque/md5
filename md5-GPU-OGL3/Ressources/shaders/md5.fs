#version 330 core

in vec2 texCoords;

uniform sampler2D baseTexture;

void main()
{
    gl_FragColor = texture2D(baseTexture, texCoords);
	//gl_FragColor = gl_Color;
}