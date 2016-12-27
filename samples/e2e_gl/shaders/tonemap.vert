#version 330 core

layout(location=0) in vec2 inPos;
layout(location=1) in vec3 inCol;
layout(location=2) in vec2 inTexCoord;

//out vec4 pos;
//out vec4 col;
out vec2 texCoord;

void main()
{
    //pos = vec4(inPos, 0.0f, 1.0f);
    gl_Position = vec4(inPos, 0.0f, 1.0f);
    //col = vec4(inCol, 1.0f);
    texCoord = inTexCoord;
}
