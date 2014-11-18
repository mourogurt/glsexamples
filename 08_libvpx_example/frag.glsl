#version 330 core

uniform sampler2D s;

out vec4 color;
in vec2 outuv;

void main(void)
{
    color = texture2D(s,vec2(outuv.x, 1.0f -  outuv.y));
}
