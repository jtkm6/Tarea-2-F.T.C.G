#version 330 core
layout (location = 0) in vec3 coord_position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 coord_texture;

out VertexOut{
	vec2 coord_texture;
	vec3 coord_frag;
	vec3 normal;
	vec4 color;
} Out;

uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;

uniform sampler2D texture_diffuse;

uniform vec3 view_position;
uniform vec3 light_position;
uniform vec3 light_spot_direction;

uniform uint light_type;
uniform uint diffuse_type;
uniform uint specular_type;
uniform uint shiny_phong;
uniform uint shiny_blinn_phong;
uniform uint interpolation_mode;

uniform float albedo;
uniform float fresnel;
uniform float roughness_oren;
uniform float roughness_cook;

uniform float in_cos;
uniform float out_cos;
uniform float linear_punctual;
uniform float constant_punctual;
uniform float quadratic_punctual;
uniform float linear_reflector;
uniform float constant_reflector;
uniform float quadratic_reflector;

uniform vec4 color_material_ambient;
uniform vec4 color_material_diffuse;
uniform vec4 color_material_specular;

uniform vec4 color_light_ambient;
uniform vec4 color_light_diffuse;
uniform vec4 color_light_specular;

void main(){
    gl_Position = projection * view * model * vec4(coord_position, 1.0f);
    Out.coord_texture = coord_texture;
    Out.coord_frag = vec3(model * vec4(coord_position, 1.0));
    Out.normal = mat3(transpose(inverse(model))) * normal;
    
    if(interpolation_mode == 1u)
    	return;

    vec3 normal = normalize(Out.normal);
    vec3 light_direction = normalize(light_position - Out.coord_frag);
    vec4 ambient = vec4(1.f, 1.f, 1.f, 1.f), diffuse = vec4(1.f, 1.f, 1.f, 1.f), specular = vec4(1.f, 1.f, 1.f, 1.f);
    // Refleccion Difusa
    if(diffuse_type == 1u){// Lambert		
		float diff = max(dot(normal, light_direction), 0.f);
		diffuse = vec4(diff, diff, diff, 1.f);
	}else if(diffuse_type == 2u){//Oren-Nayar
		vec3 view_direction = normalize(view_position - Out.coord_frag);
		float LdotV = dot(light_direction, view_direction);
		float NdotL = dot(light_direction, normal);
		float NdotV = dot(normal, view_direction);
		float s = LdotV - NdotL * NdotV;
		float t = (s > 0.f)? 1.f : max(NdotL, NdotV);
		float sigma = (roughness_oren * roughness_oren);
		float A = 1.f + sigma * (albedo / (sigma + 0.13f) + 0.5f / (sigma + 0.33f));
		float B = 0.45f * sigma / (sigma + 0.09f);
		float diff = albedo * max(0.f, NdotL) * (A + B * s / t) / 3.141593f;
		diffuse = vec4(diff, diff, diff, 1.f);
	}
	// Refleccion Especular
	if(specular_type == 1u){// Phong
		vec3 view_direction = normalize(view_position - Out.coord_frag);
		vec3 reflect_direction = reflect(-light_direction, normal);
		float spec = pow(max(dot(view_direction, reflect_direction), 0.f), shiny_phong);
		specular = vec4(spec, spec, spec, 1.f);
	}else if(specular_type == 2u){// Blinn-Phong
		vec3 view_direction = normalize(view_position - Out.coord_frag);
		vec3 halfway_direction = normalize(light_direction + view_direction);
		float spec = pow(max(dot(normal, halfway_direction), 0.f), shiny_blinn_phong);
		specular = vec4(spec, spec, spec, 1.f);
	}else if(specular_type == 3u){// Cook-Torrance   
		vec3 view_direction = normalize(view_position - Out.coord_frag);
		float VdotN = max(dot(view_direction, normal), 0.f);
		float LdotN = max(dot(light_direction, normal), 0.f);
		vec3 halfway_direction = normalize(light_direction + view_direction);
		float NdotH = max(dot(normal, halfway_direction), 0.f);
		float VdotH = max(dot(view_direction, halfway_direction), 0.000001f);
		float LdotH = max(dot(light_direction, halfway_direction), 0.000001f);
		float Gb = (2.f * NdotH * VdotN) / VdotH;
		float Gc = (2.f * NdotH * LdotN) / LdotH;
		float G = min(1.0, min(Gb, Gc));			  
		float r1 = 1.f / ( 4.f * roughness_cook * roughness_cook * pow(NdotH, 4.f));
		float r2 = (NdotH * NdotH - 1.f) / (roughness_cook * roughness_cook * NdotH * NdotH);
		float D = r1 * exp(r2);
		float F = fresnel + ((1.f - fresnel) * pow((1.f - VdotH), 5.f));
		float spec = G * F * D / max(3.141593f * VdotN * LdotN, 0.000001f);
		specular = vec4(spec, spec, spec, 1.f);
	}
	// Tipo de luz
	if(light_type == 2u){// Luz puntual 
		float distance = length(light_position - Out.coord_frag);
		float attenuation = 1.0f / (constant_punctual + linear_punctual * distance + quadratic_punctual * (distance * distance));			
		ambient *= attenuation;
		diffuse *= attenuation;
		specular *= attenuation;
	}else if(light_type == 3u){//Luz reflector
		float theta = dot(light_direction, normalize(-light_spot_direction)); 
		float epsilon = (in_cos - out_cos);
		float intensity = clamp((theta - out_cos) / epsilon, 0.f, 1.f);
		ambient *= intensity;
		diffuse *= intensity;
		specular *= intensity;
		float distance = length(light_position - Out.coord_frag);
		float attenuation = 1.f / (constant_reflector + linear_reflector * distance + quadratic_reflector * (distance * distance));    
		ambient *= attenuation;
		diffuse *= attenuation;
		specular *= attenuation;
	}
	Out.color = (color_light_ambient * color_material_ambient * ambient + color_light_diffuse * color_material_diffuse * diffuse + color_light_specular * color_material_specular * specular);
}