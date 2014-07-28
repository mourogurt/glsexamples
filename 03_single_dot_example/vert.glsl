#version 330 core

attribute vec4 vcolor;
attribute vec4 position;
out vec4 fcolor;

void main(void)
{
    gl_Position = position;
    fcolor = vcolor;
}
