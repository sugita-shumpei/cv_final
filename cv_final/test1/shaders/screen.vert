#version 450 core;
buffer(std130) MVP{
    mat4 model;
    mat4 view;
    mat4 proj;
};
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
void main(){
    
}