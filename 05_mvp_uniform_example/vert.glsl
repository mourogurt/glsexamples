#version 330 core

//attribute vec4 vcolor;
in vec4 pos;
out vec4 fcolor;
uniform mat4 mvp;
uniform vec2 vcolor;

void main(void)
{
    gl_Position = mvp * pos;
    fcolor = vec4(vcolor,0.0,1.0);
}
