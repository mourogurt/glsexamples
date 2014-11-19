#version 330 core

uniform sampler2D s;

out vec4 color;
in vec2 outuv;

void main(void)
{
    //color = vec4 (1.0f,1.0f,0.0f,1.0f);
    color = texture2D(s,vec2(outuv.x, outuv.y));
}
