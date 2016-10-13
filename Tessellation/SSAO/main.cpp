#include <GL/glew.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

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
#include "Model.h"

static const int  HBAO_RANDOM_SIZE = 4;
static const int  HBAO_RANDOM_ELEMENTS = HBAO_RANDOM_SIZE*HBAO_RANDOM_SIZE;
static const int MAX_SAMPLES = 8;

static const int NUM_MRT = 8;

GLuint screenWidth = 1920, screenHeight = 1080;

//  Number of frames per second
float fps = 0;
int currentTime = 0, previousTime = 0;
int frameCount = 0;
int type = 0;

//camera control
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();

void RenderCube();
void RenderQuad();


//noise texture
void generateNoiseTex(int width, int height);
vec3 lightPos(1.2f, 1.0f, 2.0f);
Camera camera(vec3(-4.0f, 0.0f, -4.0f));
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

float tessLevel = 2.0f;
// Options
GLuint draw_mode = 1;

GLfloat lerp(GLfloat a, GLfloat b, GLfloat f)
{
	return a + f * (b - a);
}


void generateNoiseTex(int width, int height)
{
	//float *noise = new float[width*height ];
	float *noise = new float[width*height * 4];
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			vec2 xy = glm::circularRand(1.0f);
			float z = glm::linearRand(0.0f, 1.0f);
			float w = glm::linearRand(0.0f, 1.0f);

			int offset = 4 *(y*width + x);
			//int offset = 4 * (y*width + x);
			noise[offset + 0] = xy[0] ;
			noise[offset + 1] = xy[1] ;
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

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "SSAO", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewExperimental = GL_TRUE;
	glewInit();
	glGetError();

	glViewport(0, 0, screenWidth, screenHeight);

	glEnable(GL_DEPTH_TEST);


	// Setup and compile our shaders
	Shader shaderGeometryPass("../Shaders/Dragon/SSAO/tessgeoSSAO.vs", "../Shaders/Dragon/SSAO/tessgeoSSAO.fs", "../Shaders/Dragon/SSAO/geoSSAO.tcs", "../Shaders/Dragon/SSAO/geoSSAO.tes");
	//Shader shaderGeometryPass("../Shaders/Dragon/SSAO/geoSSAO.vs", "../Shaders/Dragon/SSAO/geoSSAO.fs");
	//Shader hbaoGeometryPass("../Shaders/Dragon/HBAO/tessgeoSSAO.vs", "../Shaders/Dragon/HBAO/tessgeoSSAO.fs", "../Shaders/Dragon/HBAO/geoSSAO.tcs", "../Shaders/Dragon/HBAO/geoSSAO.tes");

	//Shader shaderLightingPass("../Shaders/Dragon/SSAO/SSAO.vs", "../Shaders/Dragon/SSAO/ssaoLight.fs");
	Shader shaderSSAO("../Shaders/Dragon/SSAO/SSAO.vs", "../Shaders/Dragon/SSAO/SSAO.fs");
	Shader shaderHBAO("../Shaders/Dragon/HBAO/hbao.vs", "../Shaders/Dragon/HBAO/hbao1.fs");
	
	Shader shaderViewnormal("../Shaders/Dragon/HBAO+/fullscreenquad.vs", "../Shaders/Dragon/HBAO+/viewnormal.fs");
	Shader shaderHBAOPlus("../Shaders/Dragon/HBAO+/fullscreenquad.vs", "../Shaders/Dragon/HBAO/hbaoplus.fs");
	Shader shaderDeinter("../Shaders/Dragon/HBAO+/fullscreenquad.vs", "../Shaders/Dragon/HBAO+/deinter.fs");
	Shader shaderReinter("../Shaders/Dragon/HBAO+/fullscreenquad.vs", "../Shaders/Dragon/HBAO+/reinter.fs");
	//Shader shaderPlusBlur
	
	
	Shader shaderSSAOBlur("../Shaders/Dragon/SSAO/SSAO.vs", "../Shaders/Dragon/SSAO/ssaoBlur.fs");

	//// Set samplers
	//shaderLightingPass.Use();
	//glUniform1i(glGetUniformLocation(shaderLightingPass.Program, "gPositionDepth"), 0);
	//glUniform1i(glGetUniformLocation(shaderLightingPass.Program, "gNormal"), 1);
	//glUniform1i(glGetUniformLocation(shaderLightingPass.Program, "gAlbedo"), 2);
	//glUniform1i(glGetUniformLocation(shaderLightingPass.Program, "ssao"), 3);
	//
		
	
	Model myObj("../Obj/nanosuit/nanosuit.obj");
	//Model myObj("../Obj/si.obj");
	//Model myObj("../Obj/dragon.obj");
	

	


	/*vec3 lightPos = vec3(2.0, 4.0, -2.0);
	vec3 lightColor = vec3(1.0, 1.0, 1.0);
*/
	


	//init framebuffer
	GLuint gBuffer;
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	GLuint gPositionDepth, gNormal, gAlbedo, gDepth;
	//pos
	glGenTextures(1, &gPositionDepth);
	glBindTexture(GL_TEXTURE_2D, gPositionDepth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPositionDepth, 0);

	//norm
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	//albedo
	glGenTextures(1, &gAlbedo);
	glBindTexture(GL_TEXTURE_2D, gAlbedo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);



	// - Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	// - Create and attach depth buffer (renderbuffer)
	GLuint rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	// - Finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "GBuffer Framebuffer not complete!" << std::endl;

	
	// Also create framebuffer to hold SSAO processing stage 
	GLuint ssaoFBO, ssaoBlurFBO;
	glGenFramebuffers(1, &ssaoFBO);  
	glGenFramebuffers(1, &ssaoBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

	GLuint ssaoColorBuffer, ssaoColorBufferBlur;
	
	// - SSAO color buffer
	glGenTextures(1, &ssaoColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "SSAO Framebuffer not complete!" << std::endl;
	
	
	//viewnormal
	GLuint viewnormal;
	glGenTextures(1, &viewnormal);
	glBindTexture(GL_TEXTURE_2D, viewnormal);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, screenWidth, screenHeight);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint viewnormalFBO;
	//newFramebuffer(fbos.viewnormal);
	glGenFramebuffers(1, &viewnormalFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, viewnormalFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, viewnormal, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "viewnorm Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	//- interleaved buffer
	int quarterWidth = ((screenWidth + 3) / 4);
	int quarterHeight = ((screenHeight + 3) / 4);

	GLenum formatAO = GL_R8;
	GLint swizzle[4] = { GL_RED,GL_RED,GL_RED,GL_RED };

	GLuint hbaoplusDepthArray;
	glGenTextures(1, &hbaoplusDepthArray);
	glBindTexture(GL_TEXTURE_2D_ARRAY, hbaoplusDepthArray);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, formatAO, quarterWidth, quarterHeight, HBAO_RANDOM_ELEMENTS);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	GLuint hbaoplusDepthView[HBAO_RANDOM_ELEMENTS];
	for (int i = 0; i < HBAO_RANDOM_ELEMENTS; i++) {

		glGenTextures(1, &hbaoplusDepthView[i]);		
		glTextureView(hbaoplusDepthView[i], GL_TEXTURE_2D, hbaoplusDepthArray, GL_R32F, 0, 1, i, 1);
		glBindTexture(GL_TEXTURE_2D, hbaoplusDepthView[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	GLuint hbaoplusResultArray;
	glGenTextures(1, &hbaoplusResultArray);
	glBindTexture(GL_TEXTURE_2D_ARRAY, hbaoplusResultArray);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, formatAO, quarterWidth, quarterHeight, HBAO_RANDOM_ELEMENTS);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);


	GLenum drawbuffers[NUM_MRT];
	for (int layer = 0; layer < NUM_MRT; layer++) {
		drawbuffers[layer] = GL_COLOR_ATTACHMENT0 + layer;
	}

	GLuint deinterFBO;
	glGenFramebuffers(1, &deinterFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, deinterFBO);
	glDrawBuffers(NUM_MRT, drawbuffers);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLuint hbaoplusCalc;
	glGenFramebuffers(1, &hbaoplusCalc);
	glBindFramebuffer(GL_FRAMEBUFFER, hbaoplusCalc);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// - and blur stage
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	glGenTextures(1, &ssaoColorBufferBlur);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Sample kernel
	std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
	std::default_random_engine generator;
	std::vector<glm::vec3> ssaoKernel;
	for (GLuint i = 0; i < 64; ++i)
	{
		glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		GLfloat scale = GLfloat(i) / 64.0;

		// Scale samples s.t. they're more aligned to center of kernel
		scale = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		ssaoKernel.push_back(sample);
	}

	// Noise texture
	std::vector<glm::vec3> ssaoNoise;
	for (GLuint i = 0; i < 16; i++)
	{
		//glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f);//ssao
		glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator)); // rotate around z-axis (in tangent space)
		ssaoNoise.push_back(noise);
	}

	GLuint noiseTexture;
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	//generateNoiseTex(4, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


	//hbao jitter

	vec4 hbaoRandom[HBAO_RANDOM_ELEMENTS * MAX_SAMPLES];
	float numDir = 8;

	signed short hbaoRandomShort[HBAO_RANDOM_ELEMENTS*MAX_SAMPLES * 4];

	for (int i = 0; i < HBAO_RANDOM_ELEMENTS*MAX_SAMPLES; i++)
	{
		float Rand1 = randomFloats(generator);
		float Rand2 = randomFloats(generator);

		float Angle = 2.0 * glm::pi<float>() * Rand1 / numDir;
		hbaoRandom[i].x = cosf(Angle);
		hbaoRandom[i].y = sinf(Angle);
		hbaoRandom[i].z = Rand2;
		hbaoRandom[i].w = 0;

#define SCALE ((1<<15)) // ?
		hbaoRandomShort[i * 4 + 0] = (signed short)(SCALE*hbaoRandom[i].x);
		hbaoRandomShort[i * 4 + 1] = (signed short)(SCALE*hbaoRandom[i].y);
		hbaoRandomShort[i * 4 + 2] = (signed short)(SCALE*hbaoRandom[i].z);
		hbaoRandomShort[i * 4 + 3] = (signed short)(SCALE*hbaoRandom[i].w);
#undef SCALE

	}

	GLuint hbaoplusRandom;
	glGenTextures(1, &hbaoplusRandom);
	glBindTexture(GL_TEXTURE_2D_ARRAY, hbaoplusRandom);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA16_SNORM, HBAO_RANDOM_SIZE, HBAO_RANDOM_SIZE, MAX_SAMPLES);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, HBAO_RANDOM_SIZE, HBAO_RANDOM_SIZE, MAX_SAMPLES, GL_RGBA, GL_SHORT, hbaoRandomShort);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	GLuint hbaopluRandomview[MAX_SAMPLES];
	for (int i = 0; i < MAX_SAMPLES; i++)
	{
		//newTexture(textures.hbao_randomview[i]);

		glGenTextures(1, &hbaopluRandomview[i]);
		glTextureView(hbaopluRandomview[i], GL_TEXTURE_2D, hbaoplusRandom, GL_RGBA16_SNORM, 0, 1, i, 1);
		glBindTexture(GL_TEXTURE_2D, hbaopluRandomview[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
	}


	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	


	
	GLfloat lastTime = glfwGetTime();
	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Set frame time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		frameCount++;

		//printf("%f\n", deltaTime);
		if (currentFrame - lastTime >= 1.0)
		{
			printf("%f ms/frame\n", 1000.0 / double(frameCount));
			frameCount = 0;
			lastTime += 1.0;
		}


		// Check and call events
		glfwPollEvents();
		Do_Movement();


		// 1. Geometry Pass: render scene's geometry/color data into gbuffer
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

		//vec4  bgColor(0.0, 0.0, 0.0, 0.0);
		//glClearBufferfv(GL_COLOR, 0, &bgColor.x);
		//glClearDepth(1.0);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		mat4 projection = perspective(radians(camera.Zoom), (GLfloat)screenWidth / (GLfloat)screenHeight, 1.0f, 2000.0f);
		
		mat4 view = camera.GetViewMatrix();
		mat4 model;
		shaderGeometryPass.Use();
		glUniformMatrix4fv(glGetUniformLocation(shaderGeometryPass.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(shaderGeometryPass.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniform1f(glGetUniformLocation(shaderGeometryPass.Program, "tess"), tessLevel);
		


		model = glm::mat4();
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -4.0));
		model = glm::rotate(model, 0.0f, glm::vec3(1.0, 0.0, 0.0));
		model = glm::scale(model, glm::vec3(1.0f));
		glUniformMatrix4fv(glGetUniformLocation(shaderGeometryPass.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		//myObj1.Draw(shaderGeometryPass);

		
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0));
		model = glm::rotate(model, 0.0f, glm::vec3(1.0, 0.0, 0.0));
		model = glm::scale(model, glm::vec3(1.0f));
		glUniformMatrix4fv(glGetUniformLocation(shaderGeometryPass.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		myObj.Draw(shaderGeometryPass);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		// 2. Create SSAO texture
		
		//hbao uniforms
		vec4 projInfo = vec4(2.0f / projection[0][0], 2.0f / projection[1][1], -1.0f / projection[0][0], -1.0f / projection[1][1]);
		vec2 InvFullResolution = vec2(1.0f / float(screenWidth), 1.0f / float(screenHeight));
		
		vec2 InvQuarterResolution = vec2(1.0f / float(quarterWidth), 1.0f / float(quarterHeight));

		float R = 1.0f;
		float NegInvR2 = -1.0f / (R * R);
		float RadiusToScreen = R * 0.5f * screenHeight / (tanf(radians(camera.Zoom) * 0.5f) * 2.0f);
		float PowExponent = 2.0f;
		float NDotVBias = 0.1f;
		float AOMultiplier = 1.0f / (1.0f - NDotVBias);

		if (type != 3)
		{

			glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
			glClear(GL_COLOR_BUFFER_BIT);

			//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gPositionDepth);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, gNormal);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, noiseTexture);

			//basic type
			if (type == 0)
			{
				//shaderSSAO.Use();
				shaderSSAO.Use();
				glUniform1i(glGetUniformLocation(shaderSSAO.Program, "gPositionDepth"), 0);
				glUniform1i(glGetUniformLocation(shaderSSAO.Program, "gNormal"), 1);
				glUniform1i(glGetUniformLocation(shaderSSAO.Program, "texNoise"), 2);


				// Send kernel + rotation 
				for (GLuint i = 0; i < 64; ++i)
					glUniform3fv(glGetUniformLocation(shaderSSAO.Program, ("samples[" + std::to_string(i) + "]").c_str()), 1, &ssaoKernel[i][0]);
				glUniformMatrix4fv(glGetUniformLocation(shaderSSAO.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
				glUniformMatrix4fv(glGetUniformLocation(shaderSSAO.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(glGetUniformLocation(shaderSSAO.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));



				

				RenderQuad();
			}

			else if (type == 1)
			{
				shaderHBAO.Use();

				
				glUniform1i(glGetUniformLocation(shaderHBAO.Program, "gPositionDepth"), 0);
				glUniform1i(glGetUniformLocation(shaderHBAO.Program, "gNormal"), 1);
				glUniform1i(glGetUniformLocation(shaderHBAO.Program, "texNoise"), 2);
				glUniformMatrix4fv(glGetUniformLocation(shaderHBAO.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
				RenderQuad();
			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			
		}
		else if (type == 2)
		{
			//no ao

			RenderQuad();
		}
		//hbaoplus
		else if (type == 3)
		{
			
			{
				//viewnormal

				glBindFramebuffer(GL_FRAMEBUFFER, viewnormalFBO);
				
				//glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
				shaderViewnormal.Use();

				glUniform4fv(glGetUniformLocation(shaderViewnormal.Program, "projInfo"), 1, &projInfo[0]);
				glUniform2fv(glGetUniformLocation(shaderViewnormal.Program, "InvFullResolution"), 1, &InvFullResolution[0]);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, gPositionDepth);
				RenderQuad();
				
				glBindTexture(GL_TEXTURE_2D, 0);
				

			}
			{
				//deinter
				glBindFramebuffer(GL_FRAMEBUFFER, deinterFBO);
				//glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
				glViewport(0, 0, quarterWidth, quarterHeight);
							
				shaderDeinter.Use();

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, gPositionDepth);

				for (int i = 0; i < HBAO_RANDOM_ELEMENTS; i += NUM_MRT) {
					glUniform4f(0, float(i % 4) + 0.5f, float(i / 4) + 0.5f, InvFullResolution[0], InvFullResolution[1]);//info

					for (int layer = 0; layer < NUM_MRT; layer++) {
						glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + layer, hbaoplusDepthView[i + layer], 0);
					}

					//RenderQuad();
					glDrawArrays(GL_TRIANGLES, 0, 3);
				}

			}
			//{
			//	//hbaoplus
			//	glBindFramebuffer(GL_FRAMEBUFFER, hbaoplusCalc);
			//	glViewport(0, 0, quarterWidth, quarterHeight);

			//	shaderHBAOPlus.Use();
			//	
			//	glActiveTexture(GL_TEXTURE1);
			//	glBindTexture(GL_TEXTURE_2D, viewnormal);

			//	glUniform4fv(glGetUniformLocation(shaderHBAOPlus.Program, "projInfo"), 1, &projInfo[0]);
			//	glUniform2fv(glGetUniformLocation(shaderHBAOPlus.Program, "InvQuarterResolution"), 1, &InvQuarterResolution[0]);
			//	
			//	
			//	glUniform1f(glGetUniformLocation(shaderHBAOPlus.Program, "NegInvR2"), NegInvR2);
			//	glUniform1f(glGetUniformLocation(shaderHBAOPlus.Program, "NDotVBias"), NDotVBias);
			//	glUniform1f(glGetUniformLocation(shaderHBAOPlus.Program, "AOMultiplier"), AOMultiplier);
			//	glUniform1f(glGetUniformLocation(shaderHBAOPlus.Program, "RadiusToScreen"), RadiusToScreen);
			//	glUniform1f(glGetUniformLocation(shaderHBAOPlus.Program, "PowExponent"), PowExponent);

			//	
			//	//uniform vec4 projInfo;
			//	//uniform vec2 InvFullResolution;
			//	//uniform float NegInvR2;
			//	//uniform float NDotVBias;
			//	//uniform float AOMultiplier;
			//	//uniform float RadiusToScreen;
			//	//uniform float PowExponent;

			//	////not layered
			//	//uniform vec2 g_Float2Offset;
			//	//uniform vec4 g_Jitter;


			//	for (int i = 0; i < HBAO_RANDOM_ELEMENTS; i++) {
			//		glUniform2f(glGetUniformLocation(shaderHBAOPlus.Program, "g_Float2Offset"), float(i % 4) + 0.5f, float(i / 4) + 0.5f);//
			//		glUniform4fv(glGetUniformLocation(shaderHBAOPlus.Program, "g_Jitter"), 1, &hbaoRandom[i][0]);//jitter
			//		
			//		
			//		glActiveTexture(GL_TEXTURE0);
			//		glBindTexture(GL_TEXTURE_2D, hbaoplusDepthView[i]);

			//		//glBindMultiTextureEXT(GL_TEXTURE0, GL_TEXTURE_2D, hbaoplusDepthview[i]);
			//		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, hbaoplusResultArray, 0, i);

			//		RenderQuad();

			//		//glDrawArrays(GL_TRIANGLES, 0, 3);
			//	}

			//}
			//{
			//	//reinter

			//	
			//	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
			//	glClear(GL_COLOR_BUFFER_BIT);

			//	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			//	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//	//glBindFramebuffer(GL_FRAMEBUFFER, fbos.hbao_calc);
			//	//glDrawBuffer(GL_COLOR_ATTACHMENT0); //??

			//	glViewport(0, 0, screenWidth, screenHeight);

			//	shaderReinter.Use();
			//	
			//	glActiveTexture(GL_TEXTURE0);
			//	glBindTexture(GL_TEXTURE_2D_ARRAY, hbaoplusResultArray);
			//	
			//	glActiveTexture(GL_TEXTURE1);
			//	glBindTexture(GL_TEXTURE_2D, gPositionDepth);
			//	
			//	RenderQuad();

			//}
		
			
		}


		


		// 3. Blur SSAO texture to remove noise
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
		//glClear(GL_COLOR_BUFFER_BIT);
		
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);

		shaderSSAOBlur.Use();
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		RenderQuad();

		// 4. Lighting Pass: traditional deferred Blinn-Phong lighting now with added screen-space ambient occlusion
		
		//shaderLightingPass.Use();
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, gPositionDepth);
		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, gNormal);
		//glActiveTexture(GL_TEXTURE2);
		//glBindTexture(GL_TEXTURE_2D, gAlbedo);
		//glActiveTexture(GL_TEXTURE3); // Add extra SSAO texture to lighting pass
		//glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
		//// Also send light relevant uniforms
		//glm::vec3 lightPosView = glm::vec3(camera.GetViewMatrix() * glm::vec4(lightPos, 1.0));
		//glUniform3fv(glGetUniformLocation(shaderLightingPass.Program, "light.Position"), 1, &lightPosView[0]);
		//glUniform3fv(glGetUniformLocation(shaderLightingPass.Program, "light.Color"), 1, &lightColor[0]);
		//// Update attenuation parameters
		//const GLfloat constant = 1.0; // Note that we don't send this to the shader, we assume it is always 1.0 (in our case)
		//const GLfloat linear = 0.09;
		//const GLfloat quadratic = 0.032;
		//glUniform1f(glGetUniformLocation(shaderLightingPass.Program, "light.Linear"), linear);
		//glUniform1f(glGetUniformLocation(shaderLightingPass.Program, "light.Quadratic"), quadratic);
		//glUniform1i(glGetUniformLocation(shaderLightingPass.Program, "draw_mode"), draw_mode);
		//RenderQuad();


		// Swap the buffers
		glfwSwapBuffers(window);
	}
	
	glfwTerminate();
	return 0;
}

// RenderQuad() Renders a 1x1 quad in NDC, best used for framebuffer color targets
// and post-processing effects.
GLuint quadVAO = 0;
GLuint quadVBO;
GLuint quadEBO;
void RenderQuad()
{
	if (quadVAO == 0)
	{
		GLfloat quadVertices[] = {
			// Positions        // Texture Coords
			-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};

		
		// Setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

// RenderCube() Renders a 1x1 3D cube in NDC.
GLuint cubeVAO = 0;
GLuint cubeVBO = 0;
void RenderCube()
{
	// Initialize (if necessary)
	if (cubeVAO == 0)
	{
		GLfloat vertices[] = {
			// Back face
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
			0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
			0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,  // top-right
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,  // bottom-left
			-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,// top-left
															  // Front face
			-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
			0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // bottom-right
			0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,  // top-right
			0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
			-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // top-left
			-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom-left
															   // Left face
			-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
			-0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
			-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-left
			-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
			-0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
			-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
															  // Right face
			0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
			0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
			0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right         
			0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-right
			0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // top-left
			0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left     
															 // Bottom face
			-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
			0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
			0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,// bottom-left
			0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
			-0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
			-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
																// Top face
			-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,// top-left
			0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
			0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-right     
			0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
			-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,// top-left
			-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// Fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// Link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// Render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}




bool keys[1024];
bool keysPressed[1024];
// Moves/alters the camera positions based on user input
void Do_Movement()
{
	// Camera controls
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FWD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BWD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RGT, deltaTime);
	if (keys[GLFW_KEY_H])
	{
		type = 1;
	}
	if (keys[GLFW_KEY_P])
	{
		type = 3;
	}
	if (keys[GLFW_KEY_O])
	{
		type = 0;
	}
	if (keys[GLFW_KEY_N])
	{
		type = 2;
	}
	if (keys[GLFW_KEY_1])
		draw_mode = 1;
	if (keys[GLFW_KEY_2])
		draw_mode = 2;
	if (keys[GLFW_KEY_3])
		draw_mode = 3;
	if (keys[GLFW_KEY_4])
		draw_mode = 4;
	if (keys[GLFW_KEY_5])
		draw_mode = 5;
	if (keys[GLFW_KEY_MINUS])
	{
		if (tessLevel > 1.0f)
			tessLevel -= 0.5f;
		//printf("%f", tessLevel);
	}
	if (keys[GLFW_KEY_EQUAL])
	{
		tessLevel += 0.5f;
		//printf("%f", tessLevel);
	}
}


// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key <= 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
			keysPressed[key] = false;
		}
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

void drawPFS()
{
	glLoadIdentity();
	printf("FPS: %4.2f", fps);
}

void calculateFPS()
{
	//  Increase frame count
	frameCount++;

	//  Get the number of milliseconds since glutInit called 
	//  (or first call to glutGet(GLUT ELAPSED TIME)).
	currentTime = glutGet(GLUT_ELAPSED_TIME);

	//  Calculate time passed
	int timeInterval = currentTime - previousTime;

	if (timeInterval > 1000)
	{
		//  calculate the number of frames per second
		fps = frameCount / (timeInterval / 1000.0f);

		//  Set time
		previousTime = currentTime;

		//  Reset frame count
		frameCount = 0;
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}