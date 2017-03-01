// Std. Includes
#include <string>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include <Model.hpp>
#include <Shader.hpp>
#include <Camera.hpp>

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Other Libs
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <AntTweakBar.h>

#define NOMINMAX
#include <Windows.h>

// Properties
GLuint screenWidth = 800, screenHeight = 600;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void window_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void char_callback(GLFWwindow* window, unsigned int codepoint);
void Modifi_The_Bar();
void Do_Movement();

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool keys[1024];
GLfloat mouse_xpos = 0.0f, mouse_ypos = 0.0f;
GLfloat xoffset = 0.0f, yoffset = 0.0f, mouse_xpos_last = 0.0f, mouse_ypos_last = 0.0f;
bool mouse_movement_flag = false, mouse_selection_flag = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
GLfloat brightness;

TwBar *bars[6]; GLuint element_selected = 0;

struct ModelParameters{
	GLuint bridge_phong = 128, bridge_blinn_phong = 128;
	GLuint reflectance_disfuse = 1, reflectance_specular = 1;
	GLfloat fresnel = 0.8f, rugosity_c = 0.9f, albedo = 0.7f, rugosity_o = 0.9f;
	glm::quat rotation;
	glm::vec3 scalation;
	glm::vec3 translation;
	glm::vec4 color_ambient;
	glm::vec4 color_diffuse;
	glm::vec4 color_specular;
} model_data[6];

GLfloat reflective_index = 1.52f;
glm::vec3 ligth_dir(0.f, -1.f, 0.f);
glm::vec3 ligth_position(0.0f, 12.0f, 0.0f);
glm::vec4 light_color_ambient(0.f, 0.f, 0.f, 1.f);
glm::vec4 light_color_diffuse(1.f, 1.f, 1.f, 1.f);
glm::vec4 light_color_specular(1.f, 1.f, 1.f, 1.f);
GLuint light_type = 1, interpolation_mode = 1;
GLfloat light_r_cos_in = 0.9f, light_r_cos_out = 0.8f, light_r_const = 1.f, light_r_lineal = 0.09f, light_r_cuad = 0.03f;
GLfloat light_p_const = 1.f, light_p_lineal = 0.09f, light_p_cuad = 0.03f;

GLuint loadCubemap(std::vector<const GLchar*> faces){
	GLuint textureID;
	glGenTextures(1, &textureID);

	int width, height;
	unsigned char* image;

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for(GLuint i = 0; i < faces.size(); i++){
		image = stbi_load(faces[i], &width, &height, 0, STBI_rgb);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		stbi_image_free(image);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureID;
}

GLuint loadTexture(GLchar* path){
	//Generate texture ID and load texture data 
	GLuint textureID;
	glGenTextures(1, &textureID);
	int width, height;
	unsigned char* image = stbi_load(path, &width, &height, 0, STBI_rgb);
	// Assign texture to ID
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(image);
	return textureID;
}

void model_anttweakbar(size_t model, char *model_name){
	bars[model] = TwNewBar(model_name);
	ModelParameters &data = model_data[model];
	// Reflactance
	TwEnumVal styleDiffuse[2] = {{1, "Lambert"},{2, "Oren-Nayar"}};
	TwType diffuse = TwDefineEnum("DiffType", styleDiffuse, 2);
	TwAddVarRW(bars[model], "Reflectancia difusa", diffuse, &data.reflectance_disfuse, "");
	// Oren-Nayar parameters
	TwAddVarRW(bars[model], "Albedo", TW_TYPE_FLOAT, &data.albedo, "min=0.01 step=0.01 max=1.0 group='Oren-Nayar'");
	TwAddVarRW(bars[model], "ONR", TW_TYPE_FLOAT, &data.rugosity_o, "label = 'Rugosidad' min=0.01 step=0.01 max=1.0 group='Oren-Nayar'");
	// Reflection
	TwEnumVal styleSpecular[3] = {{1, "Phong"},{2, "Blinn-Phong"},{3, "Cook-Torrance"}};
	TwType specular = TwDefineEnum("SpecType", styleSpecular, 3);
	TwAddVarRW(bars[model], "Reflectancia Especular", specular, &data.reflectance_specular, "");
	// Phong parameters
	TwAddVarRW(bars[model], "Phong brillo", TW_TYPE_UINT32, &data.bridge_phong, "min=1 max=500");
	// Blinn-Phong parameters
	TwAddVarRW(bars[model], "Blinn-Phong brillo", TW_TYPE_UINT32, &data.bridge_blinn_phong, "min=1 max=500");
	// Cook-Torrance parameters
	TwAddVarRW(bars[model], "Fresnel", TW_TYPE_FLOAT, &data.fresnel, "min=0.01 step=0.01 max=1.0 group='Cook-Torrance'");
	TwAddVarRW(bars[model], "CT.R", TW_TYPE_FLOAT, &data.rugosity_c, "label='Rugosidad' min=0.01 step=0.01 max=1.0 group='Cook-Torrance'");
	// Model scalation
	TwAddVarRW(bars[model], "Escala en X", TW_TYPE_FLOAT, &data.scalation.x, "min=0.001 step=0.001 group='Dimension del modelo'");
	TwAddVarRW(bars[model], "Escala en Y", TW_TYPE_FLOAT, &data.scalation.y, "min=0.001 step=0.001 group='Dimension del modelo'");
	TwAddVarRW(bars[model], "Escala en Z", TW_TYPE_FLOAT, &data.scalation.z, "min=0.001 step=0.001 group='Dimension del modelo'");
	// Model translation
	TwAddVarRW(bars[model], "Desplazamiento en X", TW_TYPE_FLOAT, &data.translation.x, "group='Posision del modelo'");
	TwAddVarRW(bars[model], "Desplazamiento en Y", TW_TYPE_FLOAT, &data.translation.y, "group='Posision del modelo'");
	TwAddVarRW(bars[model], "Desplazamiento en Z", TW_TYPE_FLOAT, &data.translation.z, "group='Posision del modelo'");
	// Model rotation
	TwAddVarRW(bars[model], "Rotacion", TW_TYPE_QUAT4F, glm::value_ptr(data.rotation), "");
	// Model material
	TwAddVarRW(bars[model], "Ambiental", TW_TYPE_COLOR4F, glm::value_ptr(data.color_ambient), "group='Color'");
	TwAddVarRW(bars[model], "Difuso", TW_TYPE_COLOR4F, glm::value_ptr(data.color_diffuse), "group='Color'");
	TwAddVarRW(bars[model], "Especular", TW_TYPE_COLOR4F, glm::value_ptr(data.color_specular), "group='Color'");
	// Reflective index
	if(!model)
		TwAddVarRW(bars[model], "Indice reflectivo", TW_TYPE_FLOAT, &reflective_index, "min=1.000 step=0.001");

	// Hide the model bar
	char buffer[1024];
	sprintf_s(buffer, "'%s' visible=false", model_name);
	TwDefine(buffer);
}

void draw_model_reflection(Model &object, Shader &shader, glm::mat4 &projection, glm::mat4 &view, ModelParameters &parameters){
	shader.Enable();
	///
	glm::mat4 reflect(1.f, 0.f, 0.f, 0.f,
					  0.f, 1.f, 0.f, 0.f,
					  0.f, 0.f,-1.f, 0.f,
					  0.f, 0.f, 0.f, 1.f);
	glm::mat4 reflected_view = view * reflect;
	///
	glm::mat4 model = glm::translate(glm::mat4(), parameters.translation) * glm::mat4_cast(parameters.rotation) * glm::scale(glm::mat4(), parameters.scalation);
	shader.SetglUniformValue("projection", projection);
	shader.SetglUniformValue("model", model);
	shader.SetglUniformValue("view", reflected_view);
	shader.SetglUniformValue("interpolation_mode", interpolation_mode);
	shader.SetglUniformValue("light_type", light_type);
	shader.SetglUniformValue("light_position", ligth_position);
	shader.SetglUniformValue("light_spot_direction", ligth_dir);
	shader.SetglUniformValue("view_position", camera.Position);
	shader.SetglUniformValue("roughness_oren", parameters.rugosity_o);
	shader.SetglUniformValue("roughness_cook", parameters.rugosity_c);
	shader.SetglUniformValue("albedo", parameters.albedo);
	shader.SetglUniformValue("fresnel", parameters.fresnel);
	shader.SetglUniformValue("shiny_phong", parameters.bridge_phong);
	shader.SetglUniformValue("shiny_blinn_phong", parameters.bridge_blinn_phong);
	shader.SetglUniformValue("diffuse_type", parameters.reflectance_disfuse);
	shader.SetglUniformValue("specular_type", parameters.reflectance_specular);
	shader.SetglUniformValue("linear_punctual", light_p_lineal);
	shader.SetglUniformValue("constant_punctual", light_p_const);
	shader.SetglUniformValue("quadratic_punctual", light_p_cuad);
	shader.SetglUniformValue("linear_reflector", light_r_lineal);
	shader.SetglUniformValue("constant_reflector", light_r_const);
	shader.SetglUniformValue("quadratic_reflector", light_r_cuad);
	shader.SetglUniformValue("in_cos", light_r_cos_in);
	shader.SetglUniformValue("out_cos", light_r_cos_out);
	shader.SetglUniformValue("color_light_specular", light_color_specular);
	shader.SetglUniformValue("color_light_ambient", light_color_ambient);
	shader.SetglUniformValue("color_light_diffuse", light_color_diffuse);
	shader.SetglUniformValue("color_material_specular", parameters.color_specular);
	shader.SetglUniformValue("color_material_ambient", parameters.color_ambient);
	shader.SetglUniformValue("color_material_diffuse", parameters.color_diffuse);
	object.Draw(shader);
	shader.Disable();
}

void draw_model(Model &object, Shader &shader, glm::mat4 &projection, glm::mat4 &view, ModelParameters &parameters){
	shader.Enable();
	glm::mat4 model = glm::translate(glm::mat4(), parameters.translation) * glm::mat4_cast(parameters.rotation) * glm::scale(glm::mat4(), parameters.scalation);
	shader.SetglUniformValue("projection", projection);
	shader.SetglUniformValue("model", model);
	shader.SetglUniformValue("view", view);
	shader.SetglUniformValue("interpolation_mode", interpolation_mode);
	shader.SetglUniformValue("light_type", light_type);
	shader.SetglUniformValue("light_position", ligth_position);
	shader.SetglUniformValue("light_spot_direction", ligth_dir);
	shader.SetglUniformValue("view_position", camera.Position);
	shader.SetglUniformValue("roughness_oren", parameters.rugosity_o);
	shader.SetglUniformValue("roughness_cook", parameters.rugosity_c);
	shader.SetglUniformValue("albedo", parameters.albedo);
	shader.SetglUniformValue("fresnel", parameters.fresnel);
	shader.SetglUniformValue("shiny_phong", parameters.bridge_phong);
	shader.SetglUniformValue("shiny_blinn_phong", parameters.bridge_blinn_phong);
	shader.SetglUniformValue("diffuse_type", parameters.reflectance_disfuse);
	shader.SetglUniformValue("specular_type", parameters.reflectance_specular);
	shader.SetglUniformValue("linear_punctual", light_p_lineal);
	shader.SetglUniformValue("constant_punctual", light_p_const);
	shader.SetglUniformValue("quadratic_punctual", light_p_cuad);
	shader.SetglUniformValue("linear_reflector", light_r_lineal);
	shader.SetglUniformValue("constant_reflector", light_r_const);
	shader.SetglUniformValue("quadratic_reflector", light_r_cuad);
	shader.SetglUniformValue("in_cos", light_r_cos_in);
	shader.SetglUniformValue("out_cos", light_r_cos_out);
	shader.SetglUniformValue("color_light_specular", light_color_specular);
	shader.SetglUniformValue("color_light_ambient", light_color_ambient);
	shader.SetglUniformValue("color_light_diffuse", light_color_diffuse);
	shader.SetglUniformValue("color_material_specular", parameters.color_specular);
	shader.SetglUniformValue("color_material_ambient", parameters.color_ambient);
	shader.SetglUniformValue("color_material_diffuse", parameters.color_diffuse);
	object.Draw(shader);
	shader.Disable();
}

void light_anttweakbar(){
	bars[5] = TwNewBar("Light");
	// Interpolation Type
	TwEnumVal styleShader[2] = {{1, "Gouraud"},{2, "Phong"}};
	TwType interpolation = TwDefineEnum("ShaderType", styleShader, 2);
	TwAddVarRW(bars[5], "Interpolación", interpolation, &interpolation_mode, "group='Tipo de interpolacion'");
	// Light Type
	TwEnumVal styleLight[3] = {{1, "direccional"},{2, "puntual"},{3, "reflector"}};
	TwType light = TwDefineEnum("LightType", styleLight, 3);
	TwAddVarRW(bars[5], "Luz", light, &light_type, "group='Tipo de luz'");
	// Light Position
	TwAddVarRW(bars[5], "X", TW_TYPE_FLOAT, &ligth_position.x, "group='Posision de la luz'");
	TwAddVarRW(bars[5], "Y", TW_TYPE_FLOAT, &ligth_position.y, "group='Posision de la luz'");
	TwAddVarRW(bars[5], "Z", TW_TYPE_FLOAT, &ligth_position.z, "group='Posision de la luz'");
	// Light Color
	TwAddVarRW(bars[5], "Ambiental", TW_TYPE_COLOR4F, glm::value_ptr(light_color_ambient), "group='Color'");
	TwAddVarRW(bars[5], "Difuso", TW_TYPE_COLOR4F, glm::value_ptr(light_color_diffuse), "group='Color'");
	TwAddVarRW(bars[5], "Especular", TW_TYPE_COLOR4F, glm::value_ptr(light_color_specular), "group='Color'");
	/////////////////////////////////////////////// Luz REFLECTOR ///////////////////////////////////////////////
	// Light Direction
	TwAddVarRW(bars[5], "Light Direction", TW_TYPE_DIR3F, glm::value_ptr(ligth_dir), "group='Luz reflector'");
	// Light Parameters
	TwAddVarRW(bars[5], "Cos. Interno", TW_TYPE_FLOAT, &light_r_cos_in, "min=0.001 max=1.0 step=0.001 group='Luz reflector'");
	TwAddVarRW(bars[5], "Cos. Externo", TW_TYPE_FLOAT, &light_r_cos_out, "min=0.001 max=1.0 step=0.001 group='Luz reflector'");
	TwAddVarRW(bars[5], "VCoR", TW_TYPE_FLOAT, &light_r_const, "min=0.001 max=1.0 step=0.001 group='Luz reflector' label='Valor Constante'");
	TwAddVarRW(bars[5], "VLR", TW_TYPE_FLOAT, &light_r_lineal, "min=0.001 max=1.0 step=0.001 group='Luz reflector' label='Valor Lineal'");
	TwAddVarRW(bars[5], "VCR", TW_TYPE_FLOAT, &light_r_cuad, "min=0.001 max=1.8 step=0.001 group='Luz reflector' label='Valor Cuadratico'");
	/////////////////////////////////////////////// Luz PUNTUAL ///////////////////////////////////////////////
	TwAddVarRW(bars[5], "VCoP", TW_TYPE_FLOAT, &light_p_const, "min=0.001 max=1.0 step=0.001 group='Luz puntual' label='Valor Constante'");
	TwAddVarRW(bars[5], "VLP", TW_TYPE_FLOAT, &light_p_lineal, "min=0.001 max=1.0 step=0.001 group='Luz puntual' label='Valor Lineal'");
	TwAddVarRW(bars[5], "VCP", TW_TYPE_FLOAT, &light_p_cuad, "min=0.001 max=1.8 step=0.001 group='Luz puntual' label='Valor Cuadratico'");
	// Hide the Light bar
	TwDefine("Light visible=false");
}


// The MAIN function, from here we start our application and run our Game loop
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow){
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if(!glfwInit()){
		OutputDebugStringA("ERROR::MAIN::glfwInit()\n");
		return EXIT_FAILURE;
	}

	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "F.T.C.G. Tarea #2 | Jorge Khabazze | 23.692.079", nullptr, nullptr);
	if(!window){
		OutputDebugStringA("ERROR::MAIN::glfwCreateWindow(...)\n");
		return EXIT_FAILURE;
	}
	glfwMakeContextCurrent(window);

#ifdef _WIN32
	// Turn on vertical screen sync under Windows.
	// (I.e. it uses the WGL_EXT_swap_control extension)
	typedef BOOL(WINAPI *PFNWGLSWAPINTERVALEXTPROC)(int interval);
	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	if(wglSwapIntervalEXT)
		wglSwapIntervalEXT(1);
#endif

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, mouse_scroll_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCharCallback(window, char_callback);

	// Initialize GLEW to setup the OpenGL Function pointers
	glewExperimental = GL_TRUE;
	glewInit();

	// Initialize AntTweakBar
	TwInit(TW_OPENGL, NULL);

	// Disable AntTweakBar help
	TwDefine("TW_HELP visible=false");

	// Add bar's
	light_anttweakbar();
	model_anttweakbar(0, "Madera");
	model_data[0].rotation = glm::quat();
	model_data[0].translation = glm::vec3(.0f, 5.0f, 25.0f);
	model_data[0].scalation = glm::vec3(0.1f, 0.1f, 0.1f);
	model_data[0].color_ambient = glm::vec4(0.f, 0.f, 0.f, 1.f);
	model_data[0].color_diffuse = glm::vec4(1.f, 1.f, 1.f, 1.f);
	model_data[0].color_specular = glm::vec4(1.f, 1.f, 1.f, 1.f);
	model_anttweakbar(1, "Pared");
	model_data[1].rotation = glm::quat(0.f, 0.f, 0.7f, 0.7f);
	model_data[1].translation = glm::vec3(0.0f, 8.0f, -12.0f);
	model_data[1].scalation = glm::vec3(0.5f, 0.5f, 0.5f);
	model_data[1].color_ambient = glm::vec4(0.f, 0.f, 0.f, 1.f);
	model_data[1].color_diffuse = glm::vec4(1.f, 1.f, 1.f, 1.f);
	model_data[1].color_specular = glm::vec4(1.f, 1.f, 1.f, 1.f);
	model_anttweakbar(2, "Tierra");
	model_data[2].rotation = glm::quat();
	model_data[2].translation = glm::vec3(0.0f, 0.f, 8.0f);
	model_data[2].scalation = glm::vec3(0.005f, 0.005f, 0.005f);
	model_data[2].color_ambient = glm::vec4(0.f, 0.f, 0.f, 1.f);
	model_data[2].color_diffuse = glm::vec4(1.f, 1.f, 1.f, 1.f);
	model_data[2].color_specular = glm::vec4(1.f, 1.f, 1.f, 1.f);
	model_anttweakbar(3, "Silla");
	model_data[3].rotation = glm::quat();
	model_data[3].translation = glm::vec3(6.0f, 5.0f, 12.0f);
	model_data[3].scalation = glm::vec3(0.05f, 0.05f, 0.05f);
	model_data[3].color_ambient = glm::vec4(0.f, 0.f, 0.f, 1.f);
	model_data[3].color_diffuse = glm::vec4(1.f, 1.f, 1.f, 1.f);
	model_data[3].color_specular = glm::vec4(1.f, 1.f, 1.f, 1.f);
	model_anttweakbar(4, "Barril");
	model_data[4].rotation = glm::quat();
	model_data[4].translation = glm::vec3(-10.0f, -2.0f, 10.0f);
	model_data[4].scalation = glm::vec3(2.0f, 2.0f, 2.0f);
	model_data[4].color_ambient = glm::vec4(0.f, 0.f, 0.f, 1.f);
	model_data[4].color_diffuse = glm::vec4(1.f, 1.f, 1.f, 1.f);
	model_data[4].color_specular = glm::vec4(1.f, 1.f, 1.f, 1.f);
	model_anttweakbar(4, "Escena");
	model_data[5].rotation = glm::quat(0.f, 0.f, 0.7f, 0.7f);
	model_data[5].translation = glm::vec3(.0f, -10.f, .0f);
	model_data[5].scalation = glm::vec3(1.f, 1.f, 1.f);
	model_data[5].color_ambient = glm::vec4(0.f, 0.f, 0.f, 1.f);
	model_data[5].color_diffuse = glm::vec4(1.f, 1.f, 1.f, 1.f);
	model_data[5].color_specular = glm::vec4(1.f, 1.f, 1.f, 1.f);
	// Send the new window size to AntTweakBar
	TwWindowSize(screenWidth, screenHeight);

	// Define the viewport dimensions
	glViewport(0, 0, screenWidth, screenHeight);

	// Setup some OpenGL options
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Setup and compile our shaders
	Shader shader("shaders/full_light.vert", "shaders/full_light.frag");
	Shader cubemap_s("shaders/cubemap/cubemap.vs", "shaders/cubemap/cubemap.fs");
	Shader light_s("shaders/light/light.vs", "shaders/light/light.fs");
	Shader skybox_s("shaders/skybox/skybox.vs", "shaders/skybox/skybox.fs");
	Shader normal_mapping_s("shaders/normal_mapping/normal_mapping.vs", "shaders/normal_mapping/normal_mapping.fs");
	// Load models
	Model scene_m("models/scene/floor.obj");
	Model wood_m("models/wood/Wood.obj");
	Model wall_m("models/wall/wall.obj");
	Model earth_m("models/earth/earth2.obj");
	GLuint normalMapEarth = loadTexture("models/earth/4096_normal.jpg");
	Model chair_m("models/chair/Wooden Chair.obj");
	Model barrel_m("models/barrel/Barrel.obj");
	GLuint normalMapBarrel = loadTexture("models/barrel/Barril.jpg");
	Model light_m("models/light/sphere.obj");
	Model cube_m("models/cube.obj");
	//////////////////////// CONFIG MIRROR ////////////////////////
	model_data[1].color_ambient.a = 0.15f;
	model_data[1].color_diffuse.a = 0.15f;
	model_data[1].color_specular.a = 0.15f;
	//////////////////////// CONFIG MIRROR ////////////////////////
	//////////////////////// CREATE CUBE MAP ////////////////////////
	GLfloat skyboxVertices[] = {
		// Positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};
	// Setup skybox VAO
	GLuint skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);
	std::vector<const GLchar*> faces;
	faces.push_back("skybox/right.jpg");
	faces.push_back("skybox/left.jpg");
	faces.push_back("skybox/top.jpg");
	faces.push_back("skybox/bottom.jpg");
	faces.push_back("skybox/back.jpg");
	faces.push_back("skybox/front.jpg");
	GLuint cubemapTexture = loadCubemap(faces);
	//////////////////////// CREATE CUBE MAP ////////////////////////
	// Draw in wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Configure depth map FBO
	const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	GLuint depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	// - Create depth texture
	GLuint depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	while(!glfwWindowShouldClose(window)){
		// Set frame time
		GLfloat currentFrame = (GLfloat)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check and call events
		glfwPollEvents();
		Do_Movement();
		Modifi_The_Bar();

		// Clear the colorbuffer
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		shader.Enable();
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 2, 0xFF);
		glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
		draw_model(wall_m, shader, projection, view, model_data[1]);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);

		glStencilFunc(GL_EQUAL, 2, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		draw_model_reflection(wood_m, shader, projection, view, model_data[0]);
		draw_model_reflection(chair_m, shader, projection, view, model_data[3]);
		draw_model_reflection(earth_m, shader, projection, view, model_data[2]);
		draw_model_reflection(barrel_m, shader, projection, view, model_data[4]);

		glStencilFunc(GL_ALWAYS, 2, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0xFF);
		draw_model(wall_m, shader, projection, view, model_data[1]);
		glDisable(GL_STENCIL_TEST);
		shader.Disable();
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		shader.Enable();
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 0, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0xFF);
		draw_model(scene_m, shader, projection, view, model_data[5]);
		glDisable(GL_STENCIL_TEST);
		shader.Disable();
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0xFF);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glEnable(GL_TEXTURE_CUBE_MAP);
		cubemap_s.Enable();
		glm::mat4 model_wood = glm::translate(glm::mat4(), model_data[0].translation) * glm::mat4_cast(model_data[0].rotation) * glm::scale(glm::mat4(), model_data[0].scalation);
		cubemap_s.SetglUniformValue("reflective_index", reflective_index);
		cubemap_s.SetglUniformValue("camera_position", camera.Position);
		cubemap_s.SetglUniformValue("projection", projection);
		cubemap_s.SetglUniformValue("model", model_wood);
		cubemap_s.SetglUniformValue("view", view);
		wood_m.Draw(cubemap_s);
		cubemap_s.Disable();
		glDisable(GL_TEXTURE_CUBE_MAP);
		glDisable(GL_STENCIL_TEST);
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		normal_mapping_s.Enable();
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 3, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0xFF);

		glm::mat4 model_earth = glm::translate(glm::mat4(), model_data[2].translation) * glm::mat4_cast(model_data[2].rotation) * glm::scale(glm::mat4(), model_data[2].scalation);
		normal_mapping_s.SetglUniformValue("projection", projection);
		normal_mapping_s.SetglUniformValue("model", model_earth);
		normal_mapping_s.SetglUniformValue("view", view);
		normal_mapping_s.SetglUniformValue("interpolation_mode", interpolation_mode);
		normal_mapping_s.SetglUniformValue("light_type", light_type);
		normal_mapping_s.SetglUniformValue("light_position", ligth_position);
		normal_mapping_s.SetglUniformValue("light_spot_direction", ligth_dir);
		normal_mapping_s.SetglUniformValue("view_position", camera.Position);
		normal_mapping_s.SetglUniformValue("roughness_oren", model_data[2].rugosity_o);
		normal_mapping_s.SetglUniformValue("roughness_cook", model_data[2].rugosity_c);
		normal_mapping_s.SetglUniformValue("albedo", model_data[2].albedo);
		normal_mapping_s.SetglUniformValue("fresnel", model_data[2].fresnel);
		normal_mapping_s.SetglUniformValue("shiny_phong", model_data[2].bridge_phong);
		normal_mapping_s.SetglUniformValue("shiny_blinn_phong", model_data[2].bridge_blinn_phong);
		normal_mapping_s.SetglUniformValue("diffuse_type", model_data[2].reflectance_disfuse);
		normal_mapping_s.SetglUniformValue("specular_type", model_data[2].reflectance_specular);
		normal_mapping_s.SetglUniformValue("linear_punctual", light_p_lineal);
		normal_mapping_s.SetglUniformValue("constant_punctual", light_p_const);
		normal_mapping_s.SetglUniformValue("quadratic_punctual", light_p_cuad);
		normal_mapping_s.SetglUniformValue("linear_reflector", light_r_lineal);
		normal_mapping_s.SetglUniformValue("constant_reflector", light_r_const);
		normal_mapping_s.SetglUniformValue("quadratic_reflector", light_r_cuad);
		normal_mapping_s.SetglUniformValue("in_cos", light_r_cos_in);
		normal_mapping_s.SetglUniformValue("out_cos", light_r_cos_out);
		normal_mapping_s.SetglUniformValue("color_light_specular", light_color_specular);
		normal_mapping_s.SetglUniformValue("color_light_ambient", light_color_ambient);
		normal_mapping_s.SetglUniformValue("color_light_diffuse", light_color_diffuse);
		normal_mapping_s.SetglUniformValue("color_material_specular", model_data[2].color_specular);
		normal_mapping_s.SetglUniformValue("color_material_ambient", model_data[2].color_ambient);
		normal_mapping_s.SetglUniformValue("color_material_diffuse", model_data[2].color_diffuse);
		normal_mapping_s.SetglUniformValue("texture_normal1", 1);
		normal_mapping_s.SetglUniformValue("texture_diffuse1", 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, earth_m.meshes[0].textures[0].id);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalMapEarth);


		glBindVertexArray(earth_m.meshes[0].VAO);
		glDrawElements(GL_TRIANGLES, (GLsizei)earth_m.meshes[0].indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);


		earth_m.Draw(normal_mapping_s);
		glDisable(GL_STENCIL_TEST);
		normal_mapping_s.Disable();
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		shader.Enable();
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 4, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0xFF);
		draw_model(chair_m, shader, projection, view, model_data[3]);
		glDisable(GL_STENCIL_TEST);
		shader.Disable();
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		normal_mapping_s.Enable();
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 5, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0xFF);

		glm::mat4 model_barrel = glm::translate(glm::mat4(), model_data[4].translation) * glm::mat4_cast(model_data[4].rotation) * glm::scale(glm::mat4(), model_data[4].scalation);
		normal_mapping_s.SetglUniformValue("projection", projection);
		normal_mapping_s.SetglUniformValue("model", model_barrel);
		normal_mapping_s.SetglUniformValue("view", view);
		normal_mapping_s.SetglUniformValue("interpolation_mode", interpolation_mode);
		normal_mapping_s.SetglUniformValue("light_type", light_type);
		normal_mapping_s.SetglUniformValue("light_position", ligth_position);
		normal_mapping_s.SetglUniformValue("light_spot_direction", ligth_dir);
		normal_mapping_s.SetglUniformValue("view_position", camera.Position);
		normal_mapping_s.SetglUniformValue("roughness_oren", model_data[4].rugosity_o);
		normal_mapping_s.SetglUniformValue("roughness_cook", model_data[4].rugosity_c);
		normal_mapping_s.SetglUniformValue("albedo", model_data[4].albedo);
		normal_mapping_s.SetglUniformValue("fresnel", model_data[4].fresnel);
		normal_mapping_s.SetglUniformValue("shiny_phong", model_data[4].bridge_phong);
		normal_mapping_s.SetglUniformValue("shiny_blinn_phong", model_data[4].bridge_blinn_phong);
		normal_mapping_s.SetglUniformValue("diffuse_type", model_data[4].reflectance_disfuse);
		normal_mapping_s.SetglUniformValue("specular_type", model_data[4].reflectance_specular);
		normal_mapping_s.SetglUniformValue("linear_punctual", light_p_lineal);
		normal_mapping_s.SetglUniformValue("constant_punctual", light_p_const);
		normal_mapping_s.SetglUniformValue("quadratic_punctual", light_p_cuad);
		normal_mapping_s.SetglUniformValue("linear_reflector", light_r_lineal);
		normal_mapping_s.SetglUniformValue("constant_reflector", light_r_const);
		normal_mapping_s.SetglUniformValue("quadratic_reflector", light_r_cuad);
		normal_mapping_s.SetglUniformValue("in_cos", light_r_cos_in);
		normal_mapping_s.SetglUniformValue("out_cos", light_r_cos_out);
		normal_mapping_s.SetglUniformValue("color_light_specular", light_color_specular);
		normal_mapping_s.SetglUniformValue("color_light_ambient", light_color_ambient);
		normal_mapping_s.SetglUniformValue("color_light_diffuse", light_color_diffuse);
		normal_mapping_s.SetglUniformValue("color_material_specular", model_data[4].color_specular);
		normal_mapping_s.SetglUniformValue("color_material_ambient", model_data[4].color_ambient);
		normal_mapping_s.SetglUniformValue("color_material_diffuse", model_data[4].color_diffuse);
		normal_mapping_s.SetglUniformValue("texture_normal1", 1);
		normal_mapping_s.SetglUniformValue("texture_diffuse1", 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, barrel_m.meshes[0].textures[0].id);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalMapBarrel);

		glBindVertexArray(barrel_m.meshes[0].VAO);
		glDrawElements(GL_TRIANGLES, (GLsizei)barrel_m.meshes[0].indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glDisable(GL_STENCIL_TEST);
		normal_mapping_s.Disable();
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 6, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0xFF);
		light_s.Enable();
		glm::mat4 light_model = glm::translate(glm::mat4(), ligth_position) * glm::scale(glm::mat4(), glm::vec3(0.03f, 0.03f, 0.03f));
		light_s.SetglUniformValue("projection", projection);
		light_s.SetglUniformValue("model", light_model);
		light_s.SetglUniformValue("view", view);
		light_m.Draw(light_s);
		light_s.Disable();
		glDisable(GL_STENCIL_TEST);
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		glEnable(GL_TEXTURE_CUBE_MAP);
		skybox_s.Enable();
		glDepthFunc(GL_LEQUAL);
		glm::mat4 skybox_model = glm::scale(glm::mat4(), glm::vec3(50.0f, 50.0f, 50.0f));
		skybox_s.SetglUniformValue("projection", projection);
		skybox_s.SetglUniformValue("model", skybox_model);
		skybox_s.SetglUniformValue("view", glm::mat4(glm::mat3(view)));
		glBindVertexArray(skyboxVAO);
		//glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);
		skybox_s.Disable();
		glDisable(GL_TEXTURE_CUBE_MAP);
		// Draw tweak bars
		TwDraw();

		// Swap the buffers
		glfwSwapBuffers(window);
	}
	TwTerminate();
	glfwTerminate();
	return 0;
}

void Do_Movement(){
	xoffset = mouse_xpos_last - mouse_xpos;
	yoffset = mouse_ypos - mouse_ypos_last;
	mouse_xpos_last = mouse_xpos;
	mouse_ypos_last = mouse_ypos;

	if(keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if(keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if(keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if(keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if(mouse_movement_flag){
		camera.ProcessMouseMovement(xoffset, yoffset);
		if(xoffset != 0.0f || yoffset != 0.0f){
			mouse_selection_flag = false;
		}
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode){
	TwEventKeyGLFW(key, action);
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if(action == GLFW_PRESS)
		keys[key] = true;
	else if(action == GLFW_RELEASE)
		keys[key] = false;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos){
	mouse_xpos = (GLfloat)xpos;
	mouse_ypos = (GLfloat)ypos;
	TwEventMousePosGLFW((int)mouse_xpos, (int)mouse_ypos);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
	if(!mouse_movement_flag && TwEventMouseButtonGLFW(button, action))
		return;
	if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
		mouse_movement_flag = true;
	}else if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE){
		mouse_movement_flag = false;
		if(mouse_selection_flag){
			GLuint index;
			glReadPixels((GLint)mouse_xpos, (GLint)(screenHeight - mouse_ypos), 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &index);
			if(index == 6){
				element_selected = 5;
				TwDefine("Light visible=true");
				TwDefine("Barril visible=false");
				TwDefine("Silla visible=false");
				TwDefine("Pared visible=false");
				TwDefine("Madera visible=false");
				TwDefine("Tierra visible=false");
			}else if(index == 5){
				element_selected = 4;
				TwDefine("Light visible=false");
				TwDefine("Barril visible=true");
				TwDefine("Silla visible=false");
				TwDefine("Pared visible=false");
				TwDefine("Madera visible=false");
				TwDefine("Tierra visible=false");
			}else if(index == 4){
				element_selected = 3;
				TwDefine("Light visible=false");
				TwDefine("Barril visible=false");
				TwDefine("Silla visible=true");
				TwDefine("Pared visible=false");
				TwDefine("Madera visible=false");
				TwDefine("Tierra visible=false");
			}else if(index == 3){
				element_selected = 2;
				TwDefine("Light visible=false");
				TwDefine("Barril visible=false");
				TwDefine("Silla visible=false");
				TwDefine("Pared visible=false");
				TwDefine("Madera visible=false");
				TwDefine("Tierra visible=true");
			}else if(index == 2){
				element_selected = 1;
				TwDefine("Light visible=false");
				TwDefine("Barril visible=false");
				TwDefine("Silla visible=false");
				TwDefine("Pared visible=true");
				TwDefine("Madera visible=false");
				TwDefine("Tierra visible=false");
			}else if(index == 1){
				element_selected = 0;
				TwDefine("Light visible=false");
				TwDefine("Barril visible=false");
				TwDefine("Silla visible=false");
				TwDefine("Pared visible=false");
				TwDefine("Madera visible=true");
				TwDefine("Tierra visible=false");
			}else{
				TwDefine("Light visible=false");
				TwDefine("Barril visible=false");
				TwDefine("Silla visible=false");
				TwDefine("Pared visible=false");
				TwDefine("Madera visible=false");
				TwDefine("Tierra visible=false");
			}
		}
		mouse_selection_flag = true;
	}
}

void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
	TwEventMouseWheelGLFW((int)yoffset);
}

void char_callback(GLFWwindow* window, unsigned int codepoint){
	TwEventCharGLFW(codepoint, GLFW_PRESS);
}

void window_size_callback(GLFWwindow* window, int width, int height){
	screenWidth = width; screenHeight = height;
	glViewport(0, 0, screenWidth, screenHeight);
	TwWindowSize(screenWidth, screenHeight);
}

void Modifi_The_Bar(){
	if(light_type == 1){
		TwDefine("Light/'Luz reflector' visible=false");
		TwDefine("Light/'Luz puntual' visible=false");
	}else if(light_type == 2){
		TwDefine("Light/'Luz reflector' visible=false");
		TwDefine("Light/'Luz puntual' visible=true");
	} else if(light_type == 3){
		TwDefine("Light/'Luz reflector' visible=true");
		TwDefine("Light/'Luz puntual' visible=false");
	}

	if(model_data[element_selected].reflectance_disfuse == 1){
		TwDefine("Barril/Oren-Nayar visible=false");
		TwDefine("Silla/Oren-Nayar visible=false");
		TwDefine("Pared/Oren-Nayar visible=false");
		TwDefine("Madera/Oren-Nayar visible=false");
		TwDefine("Tierra/Oren-Nayar visible=false");
	}else if(model_data[element_selected].reflectance_disfuse == 2){
		TwDefine("Barril/Oren-Nayar visible=true");
		TwDefine("Silla/Oren-Nayar visible=true");
		TwDefine("Pared/Oren-Nayar visible=true");
		TwDefine("Madera/Oren-Nayar visible=true");
		TwDefine("Tierra/Oren-Nayar visible=true");
	}

	if(model_data[element_selected].reflectance_specular == 1){
		TwDefine("Barril/Cook-Torrance visible=false");
		TwDefine("Silla/Cook-Torrance visible=false");
		TwDefine("Pared/Cook-Torrance visible=false");
		TwDefine("Madera/Cook-Torrance visible=false");
		TwDefine("Tierra/Cook-Torrance visible=false");
		TwDefine("Barril/'Blinn-Phong brillo' visible=false");
		TwDefine("Silla/'Blinn-Phong brillo' visible=false");
		TwDefine("Pared/'Blinn-Phong brillo' visible=false");
		TwDefine("Madera/'Blinn-Phong brillo' visible=false");
		TwDefine("Tierra/'Blinn-Phong brillo' visible=false");
		TwDefine("Barril/'Phong brillo' visible=true");
		TwDefine("Silla/'Phong brillo' visible=true");
		TwDefine("Pared/'Phong brillo' visible=true");
		TwDefine("Madera/'Phong brillo' visible=true");
		TwDefine("Tierra/'Phong brillo' visible=true");
	}else if(model_data[element_selected].reflectance_specular == 2){
		TwDefine("Barril/Cook-Torrance visible=false");
		TwDefine("Silla/Cook-Torrance visible=false");
		TwDefine("Pared/Cook-Torrance visible=false");
		TwDefine("Madera/Cook-Torrance visible=false");
		TwDefine("Tierra/Cook-Torrance visible=false");
		TwDefine("Barril/'Blinn-Phong brillo' visible=true");
		TwDefine("Silla/'Blinn-Phong brillo' visible=true");
		TwDefine("Pared/'Blinn-Phong brillo' visible=true");
		TwDefine("Madera/'Blinn-Phong brillo' visible=true");
		TwDefine("Tierra/'Blinn-Phong brillo' visible=true");
		TwDefine("Barril/'Phong brillo' visible=false");
		TwDefine("Silla/'Phong brillo' visible=false");
		TwDefine("Pared/'Phong brillo' visible=false");
		TwDefine("Madera/'Phong brillo' visible=false");
		TwDefine("Tierra/'Phong brillo' visible=false");
	}else if(model_data[element_selected].reflectance_specular == 3){
		TwDefine("Barril/Cook-Torrance visible=true");
		TwDefine("Silla/Cook-Torrance visible=true");
		TwDefine("Pared/Cook-Torrance visible=true");
		TwDefine("Madera/Cook-Torrance visible=true");
		TwDefine("Tierra/Cook-Torrance visible=true");
		TwDefine("Barril/'Blinn-Phong brillo' visible=false");
		TwDefine("Silla/'Blinn-Phong brillo' visible=false");
		TwDefine("Pared/'Blinn-Phong brillo' visible=false");
		TwDefine("Madera/'Blinn-Phong brillo' visible=false");
		TwDefine("Tierra/'Blinn-Phong brillo' visible=false");
		TwDefine("Barril/'Phong brillo' visible=false");
		TwDefine("Silla/'Phong brillo' visible=false");
		TwDefine("Pared/'Phong brillo' visible=false");
		TwDefine("Madera/'Phong brillo' visible=false");
		TwDefine("Tierra/'Phong brillo' visible=false");
	}
}