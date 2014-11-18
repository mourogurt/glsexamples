#version 330 core

in vec4 pos;
in vec2 uv;
out vec2 outuv;
uniform mat4 proj;
uniform mat4 view;

void main(void)
{
    gl_Position = proj * view * pos;
    outuv = uv;
}
