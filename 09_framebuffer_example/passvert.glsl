#version 330 core

in vec4 pos;
in vec2 uv;
out vec2 outuv;

void main(void)
{
    gl_Position = pos;
    outuv = uv;
}
