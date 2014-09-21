#version 330 core

uniform block
{
    vec2 iResolution;
    float iGlobalTime;
    mat4 proj;
    mat4 view;
};

in vec4 pos;

void main(void)
{
    gl_Position = proj * view * pos;
}
