#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D textures; // blocks textures

void main()
{
	vec4 texColor = texture(textures, TexCoord);

	if (texColor.a == 0) {
		discard;
	}

	FragColor = texColor;
}