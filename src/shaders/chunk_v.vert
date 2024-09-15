#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec2 aTexOffset;
layout (location = 3) in float aAO;

out vec2 TexCoord;
out float AO;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

int n_textures = 6; // number of textures in a column/line in the atlas texture

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0f);

	TexCoord = vec2(aTexCoord.x, aTexCoord.y);
	vec2 texOffset = vec2(aTexOffset.x, aTexOffset.y);

	TexCoord = (TexCoord + texOffset) / n_textures;
	AO = aAO;
}