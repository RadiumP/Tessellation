#include <GL/glew.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>
#include <iostream>
#include <vector>
#include <windows.h> 
#include <GLFW\glfw3.h>
#include <random>

using namespace std;
using namespace glm;
#include "Shader.h"
#include "Camera.h"
#include "LoadMesh.h"


GLuint screenWidth = 800, screenHeight = 600;



//camera control
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();


//noise texture
void generateNoiseTex(int width, int height );
vec3 lightPos(1.2f, 1.0f, 2.0f);
Camera camera(vec3(0.0f, 0.0f, 3.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

float tessLevel = 1.0f;

GLfloat lerp(GLfloat a, GLfloat b, GLfloat f)
{
	return a + f * (b - a);
}



int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "TessMesh", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewExperimental = GL_TRUE;
	glewInit();

	glViewport(0, 0, screenWidth, screenHeight);

	glEnable(GL_DEPTH_TEST);

	//Shader with gs
	Shader dragonShader("../Shaders/Dragon/Dragon.vs", "../Shaders/Dragon/Dragon.fs", "../Shaders/Dragon/Dragon.tcs", "../Shaders/Dragon/Dragon.tes", "../Shaders/Dragon/Dragon.gs");
	
	Shader ogShader("../Shaders/Dragon/DragonOG.vs", "../Shaders/Dragon/Dragon.fs", "../Shaders/Dragon/DragonOG.tcs", "../Shaders/Dragon/DragonOG.tes", "../Shaders/Dragon/Dragon.gs");
	Shader phongShader("../Shaders/Dragon/phoTess.vs", "../Shaders/Dragon/phoTess.fs", "../Shaders/Dragon/phoTess.tcs", "../Shaders/Dragon/phoTess.tes", "../Shaders/Dragon/phoTess.gs");

	


	// w/o gs
	//Shader dragonShader("../Shaders/Dragon/Dragon.vs", "../Shaders/Dragon/Dragon.fs", "../Shaders/Dragon/Dragon.tcs", "../Shaders/Dragon/Dragon.tes");
	//Shader ogShader("../Shaders/Dragon/DragonOG.vs", "../Shaders/Dragon/DragonOG.fs", "../Shaders/Dragon/DragonOG.tcs", "../Shaders/Dragon/DragonOG.tes");
	//Shader phongShader("../Shaders/Dragon/phoTess.vs", "../Shaders/Dragon/phoTess.fs", "../Shaders/Dragon/phoTess.tcs", "../Shaders/Dragon/phoTess.tes");
	
	
	//Shader ogShader("../Shaders/Dragon/DragonOG.vs", "../Shaders/Dragon/DragonOG.fs");
	//Shader dragonShader("../Shaders/Dragon/Dragon.vs", "../Shaders/Dragon/Dragon.fs");


	//mesh
	Model myDragon("../Obj/bunny/myBunny.obj");
	//Model myDragon("../Obj/bunny/bunny2.obj");
	//Model myDragon("../Obj/nanosuit/nanosuit.obj");
	//Model myDragon("../Obj/icosa.obj");
	//Model myDragon("../Obj/monkey.obj");
	
	//wireframe
	
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	GLuint noiseTex;
	GLuint heightMap;

	//height map
	glGenTextures(1, &heightMap);
	glBindTexture(GL_TEXTURE_2D, heightMap);
	
	//set tex params
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//set tex filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	//load img
	int w, h;
	unsigned char* heightImg = SOIL_load_image("tex.jpg", &w, &h, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, heightImg);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(heightImg);
	glBindTexture(GL_TEXTURE_2D, 0);// unbind


	//noise map
	glGenTextures(1, &noiseTex);
	glBindTexture(GL_TEXTURE_2D, noiseTex);
	generateNoiseTex(4, 4);


	

	
	//loop
	while (!glfwWindowShouldClose(window))
	{
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		Do_Movement();

		//clear the colorbuffer
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);//write the value to gpu
		
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//clear to the color

		//texShader
		
		dragonShader.Use();

		GLint lightPosLoc = glGetUniformLocation(dragonShader.Program, "light.position");
		GLint viewPosLoc = glGetUniformLocation(dragonShader.Program, "viewPos");
		glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(viewPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);
		
		//light
		vec3 lightColor;
		lightColor.x = 1.0f; //sin(glfwGetTime() * 2.0f);
		lightColor.y = 1.0f; //sin(glfwGetTime() * 0.7f);
		lightColor.z = 1.0f; //sin(glfwGetTime() * 1.3f);
		vec3 diffuseColor = lightColor * vec3(0.5f);
		vec3 ambientColor = diffuseColor * vec3(0.2f);
		glUniform3f(glGetUniformLocation(dragonShader.Program, "light.ambient"), ambientColor.x, ambientColor.y, ambientColor.z);
		glUniform3f(glGetUniformLocation(dragonShader.Program, "light.diffuse"), diffuseColor.x, diffuseColor.y, diffuseColor.z);
		glUniform3f(glGetUniformLocation(dragonShader.Program, "light.specular"), 1.0f, 1.0f, 1.0f);

		//material
		glUniform3f(glGetUniformLocation(dragonShader.Program, "material.ambient"), 0.19225f, 0.19225f, 0.19225f);
		glUniform3f(glGetUniformLocation(dragonShader.Program, "material.diffuse"), 0.50754f, 0.50754f, 0.50754f);
		glUniform3f(glGetUniformLocation(dragonShader.Program, "material.specular"), 0.508273f, 0.508273f, 0.508273f); // Specular doesn't have full effect on this object's material
		glUniform1f(glGetUniformLocation(dragonShader.Program, "material.shininess"), 100.0f);


		mat4 view;
		view = camera.GetViewMatrix();
		mat4 projection;
		projection = perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
		glUniformMatrix4fv(glGetUniformLocation(dragonShader.Program, "projection"), 1, GL_FALSE, value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(dragonShader.Program, "view"), 1, GL_FALSE, value_ptr(view));

		glUniform1f(glGetUniformLocation(dragonShader.Program, "tess"),tessLevel);
		
		////bind tex
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, heightMap);
		//glUniform1i(glGetUniformLocation(dragonShader.Program, "texture1"), 0);

		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, noiseTex);
		//glUniform1i(glGetUniformLocation(dragonShader.Program, "texture0"), 1);

		//draw model
		mat4 model;
		model = translate(model, vec3(4.0f, 0.0f, 0.0f));
		model = scale(model, vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(glGetUniformLocation(dragonShader.Program, "model"), 1, GL_FALSE, value_ptr(model));
		myDragon.Draw(dragonShader);
		//myMonkey.Draw(dragonShader);
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		dragonShader.unbind();

		/*//SSAO
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
		glClear(GL_COLOR_BUFFER_BIT);
		SSAO.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPositionDepth);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);

*/
		//ogShader
		ogShader.Use();
		mat4 viewOG;
		viewOG = camera.GetViewMatrix();
		mat4 projectionOG;
		projectionOG = perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f);
		glUniformMatrix4fv(glGetUniformLocation(ogShader.Program, "projection"), 1, GL_FALSE, value_ptr(projectionOG));
		glUniformMatrix4fv(glGetUniformLocation(ogShader.Program, "view"), 1, GL_FALSE, value_ptr(viewOG));


		//GLint lightPosLoc = glGetUniformLocation(ogShader.Program, "light.position");
		//GLint viewPosLoc = glGetUniformLocation(ogShader.Program, "viewPos");
		glUniform3f(glGetUniformLocation(ogShader.Program, "light.position"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(ogShader.Program, "viewPos"), camera.Position.x, camera.Position.y, camera.Position.z);

		//light
	
		glUniform3f(glGetUniformLocation(ogShader.Program, "light.ambient"), ambientColor.x, ambientColor.y, ambientColor.z);
		glUniform3f(glGetUniformLocation(ogShader.Program, "light.diffuse"), diffuseColor.x, diffuseColor.y, diffuseColor.z);
		glUniform3f(glGetUniformLocation(ogShader.Program, "light.specular"), 1.0f, 1.0f, 1.0f);

		//material
		glUniform3f(glGetUniformLocation(ogShader.Program, "material.ambient"), 1.0f, 0.5f, 0.31f);
		glUniform3f(glGetUniformLocation(ogShader.Program, "material.diffuse"), 1.0f, 0.5f, 0.31f);
		glUniform3f(glGetUniformLocation(ogShader.Program, "material.specular"), 0.5f, 0.5f, 0.5f); // Specular doesn't have full effect on this object's material
		glUniform1f(glGetUniformLocation(ogShader.Program, "material.shininess"), 32.0f);

		glUniform1f(glGetUniformLocation(ogShader.Program, "tess"), tessLevel);

		//bind tex
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, heightMap);
		glUniform1i(glGetUniformLocation(ogShader.Program, "texture1"), 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, noiseTex);
		glUniform1i(glGetUniformLocation(ogShader.Program, "texture0"), 1);




		//draw model
		mat4 modelOG;
		modelOG = translate(modelOG, vec3(0.0f, 0.0f, 0.0f));
		modelOG = scale(modelOG, vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(glGetUniformLocation(ogShader.Program, "model"), 1, GL_FALSE, value_ptr(modelOG));
		myDragon.Draw(ogShader);
		
		ogShader.unbind();
		
		//phongShader
		phongShader.Use();
		mat4 viewPhong;
		viewPhong = camera.GetViewMatrix();
		mat4 projectionPhong;
		projectionPhong = perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f);
		glUniformMatrix4fv(glGetUniformLocation(phongShader.Program, "projection"), 1, GL_FALSE, value_ptr(projectionPhong));
		glUniformMatrix4fv(glGetUniformLocation(phongShader.Program, "view"), 1, GL_FALSE, value_ptr(viewPhong));

		glUniform3f(glGetUniformLocation(phongShader.Program, "light.position"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(phongShader.Program, "viewPos"), camera.Position.x, camera.Position.y, camera.Position.z);

		//light

		glUniform3f(glGetUniformLocation(phongShader.Program, "light.ambient"), ambientColor.x, ambientColor.y, ambientColor.z);
		glUniform3f(glGetUniformLocation(phongShader.Program, "light.diffuse"), diffuseColor.x, diffuseColor.y, diffuseColor.z);
		glUniform3f(glGetUniformLocation(phongShader.Program, "light.specular"), 1.0f, 1.0f, 1.0f);

		//material
		glUniform3f(glGetUniformLocation(phongShader.Program, "material.ambient"), 1.0f, 0.5f, 0.31f);
		glUniform3f(glGetUniformLocation(phongShader.Program, "material.diffuse"), 1.0f, 0.5f, 0.31f);
		glUniform3f(glGetUniformLocation(phongShader.Program, "material.specular"), 0.5f, 0.5f, 0.5f); // Specular doesn't have full effect on this object's material
		glUniform1f(glGetUniformLocation(phongShader.Program, "material.shininess"), 32.0f);

		glUniform1f(glGetUniformLocation(phongShader.Program, "tess"), tessLevel);


		//bind tex
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, heightMap);
		glUniform1i(glGetUniformLocation(phongShader.Program, "texture1"), 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, noiseTex);
		glUniform1i(glGetUniformLocation(phongShader.Program, "texture0"), 1);




		//draw model
		mat4 modelPhong;
		modelPhong = translate(modelPhong, vec3(-4.0f, 0.0f, 0.0f));
		modelPhong = scale(modelPhong, vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(glGetUniformLocation(phongShader.Program, "model"), 1, GL_FALSE, value_ptr(modelPhong));
		myDragon.Draw(phongShader);
		phongShader.unbind();



		//swap
		glfwSwapBuffers(window);



	}
	
	
	
	glfwTerminate();

	return 0;



}


void Do_Movement()
{
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FWD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BWD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RGT, deltaTime);

	if (keys[GLFW_KEY_MINUS])
	{
		if (tessLevel >= 2.0f)
			tessLevel -= 1.0f;
		printf("%f", tessLevel);
	}
	if (keys[GLFW_KEY_EQUAL])
	{
		tessLevel += 1.0f;
		printf("%f", tessLevel);
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);

}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}


void generateNoiseTex(int width, int height)
{
	float *noise = new float[width*height * 4];
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			vec2 xy = glm::circularRand(1.0f);
			float z = glm::linearRand(0.0f, 1.0f);
			float w = glm::linearRand(0.0f, 1.0f);

			int offset = 4 * (y*width + x);
			noise[offset + 0] = xy[0];
			noise[offset + 1] = xy[1];
			noise[offset + 2] = z;
			noise[offset + 3] = w;
		}
	}
	GLint internalFormat = GL_RGBA16F;
	GLint format = GL_RGBA;
	GLint type = GL_FLOAT;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, noise);
}