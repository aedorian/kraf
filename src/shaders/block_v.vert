#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in float aTexIndex;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec2 texOffsets[6]; // offsets for each face

int n_textures = 6; // number of textures in a column/line in the atlas texture

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0f);

	TexCoord = vec2(aTexCoord.x, aTexCoord.y);
	int index = int(aTexIndex);
	vec2 texOffset = texOffsets[index];

	TexCoord = (TexCoord + texOffset) / n_textures;
}