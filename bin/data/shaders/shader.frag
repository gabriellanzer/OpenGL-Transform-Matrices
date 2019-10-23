#version 330

uniform sampler2D tex;

in vec2 vUV;
out vec4 frag;

void main()
{
	frag = texture(tex, vUV);
}