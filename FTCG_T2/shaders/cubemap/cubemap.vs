#version 330 core
layout (location = 0) in vec3 coord_position;
layout (location = 1) in vec3 normal;

uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;

out VertexOut{
	vec3 coord_pixel;
	vec3 normal;
} Out;

void main(){
    gl_Position = projection * view * model * vec4(coord_position, 1.0f);
    Out.coord_pixel = vec3(model * vec4(coord_position, 1.0));
    Out.normal = mat3(transpose(inverse(model))) * normal;
}