/*  by Carles Roch and Eric Planes, 2021 LaSalle */

//****************************************		INCLUDES DE LAS LIBRERIAS		********************************************************//
//Include de las librerias estándar
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

//Include de les llibreries OpenGL 
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//include some custom code files
#include "glfunctions.h" //include all OpenGL stuff
#include "Shader.h" //class to compile shaders
#include <string>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "imageloader.h"

using namespace std;
using namespace glm;


//****************************************		PREPARACION DE LOS OBJETOS		********************************************************//
//Informacion necesaria para preparar el Objeto 1
std::string basepath = "assets/";
std::string inputfile1 = basepath + "sphere.obj";
std::vector<tinyobj::shape_t > shapes;
std::vector<tinyobj::material_t > materials;

//Uso general para poder ser usado en todo el main Objeto 1
std::string err1;
bool ret = tinyobj::LoadObj(shapes, materials, err1, inputfile1.c_str(), basepath.c_str());

//Informacion necesaria para preparar el Objeto 2
std::string inputfile2 = basepath + "sphere.obj";
std::vector<tinyobj::shape_t > shapes2;
std::vector<tinyobj::material_t > materials2;

//Uso general para poder ser usado en todo el main Objeto 2
std::string err2;
bool ret2 = tinyobj::LoadObj(shapes2, materials2, err2, inputfile2.c_str(), basepath.c_str());

//Informacion necesaria para preparar el Objeto 3
std::string inputfile3 = basepath + "sphere.obj";
std::vector<tinyobj::shape_t > shapes3;
std::vector< tinyobj::material_t > materials3;

//Uso general para poder ser usado en todo el main Objeto 3
std::string err3;
bool ret3 = tinyobj::LoadObj(shapes3, materials3, err3, inputfile3.c_str(), basepath.c_str());


//*************************************		DECLARACION DE VARIABLES GLOBALES		************************************************//
//Medida de la ventana en px
int g_ViewportWidth = 512; int g_ViewportHeight = 512;

//Variables para la posición del mouse
double mouse_x, mouse_y;

//Color del background
const vec3 g_backgroundColor(0.2f, 0.2f, 0.2f);

//Variables de la cámara
float earth_cam_x;
float earth_cam_z;
float sun_cam_x;
float sun_cam_z;
float general_Y = 30.0f;

//Shader identifier, vao and numbre of triangles we are painting for the object 1
GLuint g_simpleShader = 0; 
GLuint g_Vao = 0; 
GLuint g_NumTriangles = 0; 

//Shader identifier, vao and numbre of triangles we are painting for the object 2
GLuint g_simpleShader_sky = 1;
GLuint g_Vao2 = 0; 
GLuint g_NumTriangles_sky = 0;

//Vao for the object 3
GLuint g_Vao3 = 0;

//Texturas de los objetos
GLuint texture_id_earth;
GLuint texture_id_sky;
GLuint texture_id_sun;

//Perspectiva de la cámara
mat4 projection_matrix = perspective(
	50.0f, //Campo de visión
	1.0f, //Aspect ratio
	0.5f, //Distáncia mínima a partir de la que se pueden observar objetos
	800.0f //Distáncia máxima hasta la que se pueden observar objetos
);

//********************************************			LOAD FUNCTION			*******************************************************//
// This function manually creates a square geometry (defined in the array vertices[])
void load()
{
	//**********************
	// CODE TO SET GEOMETRY
	//**********************

	//Cargar las texturas de la tierra
	Image* image_earth = loadBMP("assets/earthmap1k.bmp");
	glGenTextures(1, &texture_id_earth);
	glBindTexture(GL_TEXTURE_2D, texture_id_earth);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGB,
		image_earth->width,
		image_earth->height,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		image_earth->pixels
		);

	//Cargar las texturas del espacio
	Image* image_sky = loadBMP("assets/milkyway.bmp");
	glGenTextures(1, &texture_id_sky);
	glBindTexture(GL_TEXTURE_2D, texture_id_sky);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGB,
		image_sky->width,
		image_sky->height,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		image_sky->pixels
	);

	//Cargar las texturas del sol
	Image* image_sun = loadBMP("assets/sunmap.bmp");
	glGenTextures(1, &texture_id_sun);
	glBindTexture(GL_TEXTURE_2D, texture_id_sun);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGB,
		image_sun->width,
		image_sun->height,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		image_sun->pixels
	);
	
	//Vectores para los shaders
	GLuint uvs_size;
	GLfloat* uvs;
	GLuint normal_size;
	GLfloat* normal;

 	//**********************
	// CODE TO LOAD EVERYTHING INTO MEMORY
	//**********************

	//load the shader
	Shader simpleShader("src/shader.vert", "src/shader.frag");
	g_simpleShader = simpleShader.program;
	Shader simpleShader2("src/shader_sky.vert", "src/shader_sky.frag");
	g_simpleShader_sky = simpleShader2.program;

	//****Cree el VAO donde almacenamos toda la geometría (almacenada en g_Vao)****//
	g_Vao = gl_createAndBindVAO();

	// Inicializar el vector normal 
	normal = &(shapes[0].mesh.normals[0]);
	normal_size = shapes[0].mesh.texcoords.size() * sizeof(GLfloat);
	gl_createAndBindAttribute(normal, normal_size, g_simpleShader, "a_normal", 3);

	// Inicializar el vector uvs 
	uvs = &(shapes[0].mesh.texcoords[0]);
	uvs_size = shapes[0].mesh.texcoords.size() * sizeof(GLfloat);
	gl_createAndBindAttribute(uvs, uvs_size, g_simpleShader, "a_uv", 2);

	// Vincular los vectores / matrices a los atributos en los shaders 
	gl_createAndBindAttribute(&(shapes[0].mesh.positions[0]), shapes[0].mesh.positions.size() * sizeof(float), g_simpleShader, "a_vertex", 3);
	gl_createIndexBuffer(&(shapes[0].mesh.indices[0]), shapes[0].mesh.indices.size() * sizeof(unsigned int));

	//Desvincular todo lo anterior
	gl_unbindVAO();
	

	//****Cree el VAO donde almacenamos toda la geometría (almacenada en g_Vao2)****
	g_Vao2 = gl_createAndBindVAO();

	//Inicializar el vector normal 
	normal = &(shapes2[0].mesh.normals[0]);
	normal_size = shapes2[0].mesh.texcoords.size() * sizeof(GLfloat);
	gl_createAndBindAttribute(normal, normal_size, g_simpleShader_sky, "a_normal", 3);

	// Inicializar el vector uvs 
	uvs = &(shapes2[0].mesh.texcoords[0]);
	uvs_size = shapes2[0].mesh.texcoords.size() * sizeof(GLfloat);
	gl_createAndBindAttribute(uvs, uvs_size, g_simpleShader_sky, "a_uv", 2);

	// Vincular los vectores / matrices a los atributos en los shaders 2
	gl_createAndBindAttribute(&(shapes2[0].mesh.positions[0]), shapes2[0].mesh.positions.size() * sizeof(float), g_simpleShader, "a_vertex", 3);
	gl_createIndexBuffer(&(shapes2[0].mesh.indices[0]), shapes2[0].mesh.indices.size() * sizeof(unsigned int));

	//Desvincular todo lo anterior
	gl_unbindVAO();


	//****Cree el VAO donde almacenamos toda la geometría (almacenada en g_Vao3)****
	g_Vao3 = gl_createAndBindVAO();

	//Inicializar el vector normal 
	normal = &(shapes3[0].mesh.normals[0]);
	normal_size = shapes3[0].mesh.texcoords.size() * sizeof(GLfloat);
	gl_createAndBindAttribute(normal, normal_size, g_simpleShader_sky, "a_normal", 3);

	// Inicializar el vector uvs 
	uvs = &(shapes3[0].mesh.texcoords[0]);
	uvs_size = shapes3[0].mesh.texcoords.size() * sizeof(GLfloat);
	gl_createAndBindAttribute(uvs, uvs_size, g_simpleShader_sky, "a_uv", 2);

	// Vincular los vectores / matrices a los atributos en los shaders 3
	gl_createAndBindAttribute(&(shapes3[0].mesh.positions[0]), shapes3[0].mesh.positions.size() * sizeof(float), g_simpleShader, "a_vertex", 3);
	gl_createIndexBuffer(&(shapes3[0].mesh.indices[0]), shapes3[0].mesh.indices.size() * sizeof(unsigned int));

	//Desvincular todo lo anterior
	gl_unbindVAO();

	//Guardar el número de triangulos para usarlo más tarde en el draw() funciton.
	g_NumTriangles = shapes[0].mesh.indices.size() / 3;
	g_NumTriangles_sky = shapes2[0].mesh.indices.size() / 3;

}
//********************************************			LOAD FUNCTION			*******************************************************//
// This function actually draws to screen and called non-stop, in a loop
void draw(int objectToDraw)
{	
	glm::mat4 view_matrix/*, view_matrix_mars*/;
	float earth_radius = 10.0f;
	float sun_radius = 25.0f;
	earth_cam_x = sin(glfwGetTime()) * earth_radius;
	earth_cam_z = cos(glfwGetTime()) * earth_radius;
	sun_cam_x = sin(glfwGetTime()/50) * sun_radius;
	sun_cam_z = cos(glfwGetTime()/50) * sun_radius;
	vec3 g_light_dir(vec3(0, 0, 0) - vec3(earth_cam_x, 0, earth_cam_z));
	
	view_matrix = glm::lookAt(glm::vec3(sun_cam_x, general_Y, sun_cam_z), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	mat4 model;

	switch (objectToDraw) {
	case 0:
		model = translate(mat4(1.0f), vec3(0, 0, 0));
		model = scale(mat4(1.0f), vec3(60, 60, 60));
		break;
	case 1:
		model = rotate(model, (float)glfwGetTime() * 50, vec3(0.0f, 10000.0f, 10.0f));
		model = translate(model, vec3(earth_cam_x, 0, earth_cam_z));
		break;
	case 2:
		model = rotate(model, (float)glfwGetTime() * 100, vec3(0.0f, 10000.0f, 10.0f));
		model = translate(model, vec3(0, 0, 0));
		break;
	}

	//Activar el shader
	switch (objectToDraw) {
	case 1: glUseProgram(g_simpleShader);
		break;
	default:
		glUseProgram(g_simpleShader_sky);
		break;
	}

	GLuint model_loc = glGetUniformLocation(g_simpleShader, "u_model");
	glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
	GLuint view_loc = glGetUniformLocation(g_simpleShader, "u_view");
	glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));
	GLuint projection_loc = glGetUniformLocation(g_simpleShader, "u_projection");
	glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

	if(objectToDraw == 1){
		GLuint light_color = glGetUniformLocation(g_simpleShader, "u_light_color");
		glUniform3f(light_color, 1.0f, 1.0f, 1.0f);

		GLuint eye = glGetUniformLocation(g_simpleShader, "u_eye");
		glUniform3f(eye, sun_cam_x, general_Y, sun_cam_z);

		GLuint glossiness = glGetUniformLocation(g_simpleShader, "u_glossiness");
		glUniform1f(glossiness, 50.0f);
	}
	
	GLuint u_light_dir = glGetUniformLocation(g_simpleShader, "u_light_dir");
	glUniform3f(u_light_dir, g_light_dir.x, g_light_dir.y, g_light_dir.z);

	//Inicializar las u_texture
	GLuint u_texture_earth = glGetUniformLocation(g_simpleShader, "u_texture");
	GLuint u_texture_sky = glGetUniformLocation(g_simpleShader_sky, "u_texture_sky");
	GLuint u_texture_sun = glGetUniformLocation(g_simpleShader, "u_texture_sun");

	//Vincular la geometria
	switch (objectToDraw) {
	case 0:
		glUniform1i(u_texture_sky, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_id_sky);
		glBindVertexArray(g_Vao);
		glDrawElements(GL_TRIANGLES, 3 * g_NumTriangles, GL_UNSIGNED_INT, 0);
		break;
	case 1:
		glUniform1i(u_texture_earth, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_id_earth);
		glBindVertexArray(g_Vao);
		glDrawElements(GL_TRIANGLES, 3 * g_NumTriangles, GL_UNSIGNED_INT, 0);
		break;
	case 2:
		glUniform1i(u_texture_sun, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_id_sun);
		glBindVertexArray(g_Vao3);
		glDrawElements(GL_TRIANGLES, 3 * g_NumTriangles, GL_UNSIGNED_INT, 0);
		break;
	}
}

// This function is called every time you press a screen
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, 1);
}

// This function is called every time you click the mouse
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        cout << "Left mouse down at" << mouse_x << ", " << mouse_y << endl;
    }
}

int main(void)
{
	//setup window and boring stuff, defined in glfunctions.cpp
	GLFWwindow* window;
	if (!glfwInit()) return -1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(g_ViewportWidth, g_ViewportHeight, "Hello OpenGL!", NULL, NULL);
	if (!window) {glfwTerminate();	return -1;}
	glfwMakeContextCurrent(window);
	glewInit();

	//input callbacks
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);

	//load all the resources
	load();

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_CULL_FACE);

		//UNIVERSO
		glDisable(GL_DEPTH_TEST);
		glCullFace(GL_FRONT);
		draw(0);

		//TIERRA 
		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);
		draw(1);

		//SUN
		draw(2);

        // Swap front and back buffers
        glfwSwapBuffers(window);
        
        // Poll for and process events
        glfwPollEvents();
        
        //mouse position must be tracked constantly (callbacks do not give accurate delta)
        glfwGetCursorPos(window, &mouse_x, &mouse_y);
    }

	//test it loaded correctly
	if (!err1.empty()) { // `err1` may contain warning message.
		std::cerr << err1 << std::endl;
	}

	if (!err2.empty()) { // `err2` may contain warning message.
		std::cerr << err2 << std::endl;
	}

	if (!err3.empty()) { // `err3` may contain warning message.
		std::cerr << err3 << std::endl;
	}

    //terminate glfw and exit
    glfwTerminate();
    return 0;
}


