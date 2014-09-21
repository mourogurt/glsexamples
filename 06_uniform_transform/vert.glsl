#version 330 core

uniform mat4 proj;
uniform mat4 view;

in vec4 pos;
in vec4 color;
out vec4 vcolor;

void main(void)
{
    gl_Position = proj * view * pos;
    vcolor = view * color;
}
