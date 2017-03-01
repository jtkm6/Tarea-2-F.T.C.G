#version 330 core

in VertexOut{
	vec3 coord_pixel;
	vec3 normal;
} In;

uniform float reflective_index;
uniform vec3 camera_position;
uniform samplerCube skybox;

out vec4 color;

void main(){
    float ratio = 1.00 / reflective_index;
    vec3 I = normalize(In.coord_pixel - camera_position);
    vec3 R = refract(I, normalize(In.normal), ratio);
    color = texture(skybox, R);
}