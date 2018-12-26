#version 330

in vec3 VertColor;
in vec2 TextCoord;

uniform sampler2D tex;
uniform sampler2D tex2;

out vec4 color;


void main()
{
	//color = mix(texture(tex, TextCoord), texture(tex2, TextCoord), 0.3) * vec4(VertColor, 1.0f);
	//color = texture(tex, TextCoord) * vec4(VertColor, 1.0f);
	vec4 argbRes = texture2D(tex, vec2(TextCoord.x,1.0 -TextCoord.y));
	color = vec4(argbRes.b,argbRes.g,argbRes.r,argbRes.a);
}