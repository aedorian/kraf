#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in float AO;

uniform sampler2D textures; // blocks textures
uniform float sunLight;

void main()
{
	vec4 texColor = texture(textures, TexCoord);

	// texColor = vec4(1.0);

	if (texColor.a == 0) {
		discard;
	}

	FragColor = texColor * AO * sunLight;
}