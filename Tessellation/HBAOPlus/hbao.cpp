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
//#include "Shader.h"
#include "Shader.h"
#include "Camera.h"
#include "LoadMesh.h"
#include "Model.h"


static const int  NUM_MRT = 8;
static const int  HBAO_RANDOM_SIZE = 4;
static const int  HBAO_RANDOM_ELEMENTS = HBAO_RANDOM_SIZE*HBAO_RANDOM_SIZE;
static const int  MAX_SAMPLES = 8;


//texture
struct {
	GLuint
		scene_color,
		scene_depthstencil,
		scene_depthlinear,
		scene_viewnormal,
		hbao_result,
		hbao_blur,
		hbao_random,
		hbao_randomview[MAX_SAMPLES],
		hbao2_deptharray,
		hbao2_depthview[HBAO_RANDOM_ELEMENTS],
		hbao2_resultarray;
} textures;


//FBO
struct {
	GLuint
		scene,
		depthlinear,
		viewnormal,
		hbao_calc,
		hbao2_deinterleave,
		hbao2_calc;

}fbos;

//uniforms
struct hbao{
	vec4 projInfo;
	vec2 InvFullResolution;

	vec2 InvQuarterResolution;

	float R;
	float NegInvR2 ;
	float RadiusToScreen;
	float PowExponent;
	float NDotVBias;
	float AOMultiplier;
}hbaoData;


// Properties
const GLuint SCR_WIDTH = 1920, SCR_HEIGHT = 1080;

int quarterWidth = ((SCR_WIDTH + 3) / 4);
int quarterHeight = ((SCR_HEIGHT + 3) / 4);

glm::vec4  hbaoRandom[HBAO_RANDOM_ELEMENTS * MAX_SAMPLES];
signed short hbaoRandomShort[HBAO_RANDOM_ELEMENTS*MAX_SAMPLES * 4];


//  Number of frames per second
float fps = 0;
int currentTime = 0, previousTime = 0;
int frameCount = 0;
int type = 0;
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;


float tessLevel = 2.0f;






// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));

// Delta
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// Options
GLuint draw_mode = 1;
//GLuint ao_mode = 0;

GLfloat lerp(GLfloat a, GLfloat b, GLfloat f)
{
	return a + f * (b - a);
}




void RenderQuad();


// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();



int main()
{
	// Init GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", nullptr, nullptr); // Windowed
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Initialize GLEW to setup the OpenGL Function pointers
	glewExperimental = GL_TRUE;
	glewInit();
	glGetError();

	// Define the viewport dimensions
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	// Setup some OpenGL options
	glEnable(GL_DEPTH_TEST);

	// Setup and compile our shaders
	Shader shaderDrawScene("../Shaders/AO/HBAO+/scene.vs", "../Shaders/AO/HBAO+/scene.fs");
	//Shader shaderDrawScene("../Shaders/AO/HBAO+/tessgeoSSAO.vs", "../Shaders/AO/HBAO+/tessgeoSSAO.fs", "../Shaders/AO/HBAO+/geoSSAO.tcs", "../Shaders/AO/HBAO+/geoSSAO.tes");

	Shader shaderLinearDepth("../Shaders/AO/HBAO+/fullscreenquad.vs", "../Shaders/AO/HBAO+/depthlinearize.fs");
	Shader shaderViewNormal("../Shaders/AO/HBAO+/fullscreenquad.vs", "../Shaders/AO/HBAO+/viewnormal.fs");
	Shader shaderHBAO("../Shaders/AO/HBAO+/fullscreenquad.vs", "../Shaders/AO/HBAO+/hbao.fs");
	Shader shaderBlur("../Shaders/AO/HBAO+/fullscreenquad.vs", "../Shaders/AO/HBAO+/blur.fs");
	Shader shaderSSAO("../Shaders/AO/HBAO+/fullscreenquad.vs", "../Shaders/AO/HBAO+/ssao.fs");
	//Shader shaderHBAOPlus("../Shaders/AO/fullscreenquad.vs", "../Shaders/AO/hbaoplus.fs", "../Shaders/AO/fullscreenquad.gs"); //SINGLEPASS
	Shader shaderHBAOPlus("../Shaders/AO/HBAO+/fullscreenquad.vs", "../Shaders/AO/HBAO+/hbaoplus.fs");
	Shader shaderDeinterleave("../Shaders/AO/HBAO+/fullscreenquad.vs", "../Shaders/AO/HBAO+/deinter.fs");
	Shader shaderReinterleave("../Shaders/AO/HBAO+/fullscreenquad.vs", "../Shaders/AO/HBAO+/reinter.fs");




	//initMisc
	std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
	std::default_random_engine generator;


	float numDir = 8; // keep in sync to glsl
	for (int i = 0; i < HBAO_RANDOM_ELEMENTS * MAX_SAMPLES; i++)
	{
		float Rand1 = randomFloats(generator);
		float Rand2 = randomFloats(generator);

		float Angle = 2.0 * glm::pi<float>() * Rand1 / numDir;
		hbaoRandom[i].x = cosf(Angle);
		hbaoRandom[i].y = sinf(Angle);
		hbaoRandom[i].z = Rand2;
		hbaoRandom[i].w = 0;
		//???
#define SCALE ((1<<15))
		hbaoRandomShort[i * 4 + 0] = (signed short)(SCALE*hbaoRandom[i].x);
		hbaoRandomShort[i * 4 + 1] = (signed short)(SCALE*hbaoRandom[i].y);
		hbaoRandomShort[i * 4 + 2] = (signed short)(SCALE*hbaoRandom[i].z);
		hbaoRandomShort[i * 4 + 3] = (signed short)(SCALE*hbaoRandom[i].w);
#undef SCALE

	}

	//noise Tex
	
	glGenTextures(1, &textures.hbao_random);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textures.hbao_random);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA16_SNORM, HBAO_RANDOM_SIZE, HBAO_RANDOM_SIZE, MAX_SAMPLES);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, HBAO_RANDOM_SIZE, HBAO_RANDOM_SIZE, MAX_SAMPLES, GL_RGBA, GL_SHORT, hbaoRandomShort);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	for (int i = 0; i < MAX_SAMPLES; i++)
	{
		glGenTextures(1, &textures.hbao_randomview[i]);
		glTextureView(textures.hbao_randomview[i], GL_TEXTURE_2D, textures.hbao_random, GL_RGBA16_SNORM, 0, 1, i, 1);
		glBindTexture(GL_TEXTURE_2D, textures.hbao_randomview[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
	}


	//init obj
	//Model scene("../Obj/si.obj");
	Model scene("../Obj/nanosuit/nanosuit.obj");

	//init FrameBuffer
	
	glGenTextures(1, &textures.scene_color);
	glBindTexture(GL_TEXTURE_2D, textures.scene_color);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, SCR_WIDTH, SCR_HEIGHT);
	glBindTexture(GL_TEXTURE_2D, 0);

	
	glGenTextures(1, &textures.scene_depthstencil);
	glBindTexture(GL_TEXTURE_2D, textures.scene_depthstencil);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glBindTexture(GL_TEXTURE_2D, 0);

	
	glGenFramebuffers(1, &fbos.scene);
	glBindFramebuffer(GL_FRAMEBUFFER, fbos.scene);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textures.scene_color, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, textures.scene_depthstencil, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	
	glGenTextures(1, &textures.scene_depthlinear);
	glBindTexture(GL_TEXTURE_2D, textures.scene_depthlinear);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, SCR_WIDTH, SCR_HEIGHT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	
	
	glGenFramebuffers(1, &fbos.depthlinear);
	glBindFramebuffer(GL_FRAMEBUFFER, fbos.depthlinear);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textures.scene_depthlinear, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	
	glGenTextures(1, &textures.scene_viewnormal);
	glBindTexture(GL_TEXTURE_2D, textures.scene_viewnormal);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, SCR_WIDTH, SCR_HEIGHT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	
	glGenFramebuffers(1, &fbos.viewnormal);
	glBindFramebuffer(GL_FRAMEBUFFER, fbos.viewnormal);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textures.scene_viewnormal, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//blur
	/*GLenum formatAO = GL_RG16F;
	GLint swizzle[4] = { GL_RED,GL_GREEN,GL_ZERO,GL_ZERO };*/

	GLenum formatAO = GL_R8;
	GLint swizzle[4] = { GL_RED,GL_RED,GL_RED,GL_RED };
	
	
	
	glGenTextures(1, &textures.hbao_result);
	glBindTexture(GL_TEXTURE_2D, textures.hbao_result);
	glTexStorage2D(GL_TEXTURE_2D, 1, formatAO, SCR_WIDTH, SCR_HEIGHT);
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	
	glGenTextures(1, &textures.hbao_blur);
	glBindTexture(GL_TEXTURE_2D, textures.hbao_blur);
	glTexStorage2D(GL_TEXTURE_2D, 1, formatAO, SCR_WIDTH, SCR_HEIGHT);
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	
	glGenFramebuffers(1, &fbos.hbao_calc);
	glBindFramebuffer(GL_FRAMEBUFFER, fbos.hbao_calc);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textures.hbao_result, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, textures.hbao_blur, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//interleaved 

	
	glGenTextures(1, &textures.hbao2_deptharray);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textures.hbao2_deptharray);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R32F, quarterWidth, quarterHeight, HBAO_RANDOM_ELEMENTS);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	
	for (int i = 0; i < HBAO_RANDOM_ELEMENTS; i++) {
		glGenTextures(1, &textures.hbao2_depthview[i]);
		glTextureView(textures.hbao2_depthview[i], GL_TEXTURE_2D, textures.hbao2_deptharray, GL_R32F, 0, 1, i, 1);
		glBindTexture(GL_TEXTURE_2D, textures.hbao2_depthview[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	
	glGenTextures(1, &textures.hbao2_resultarray);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textures.hbao2_resultarray);
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

	
	glGenFramebuffers(1, &fbos.hbao2_deinterleave);
	glBindFramebuffer(GL_FRAMEBUFFER, fbos.hbao2_deinterleave);
	glDrawBuffers(NUM_MRT, drawbuffers);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	
	glGenFramebuffers(1, &fbos.hbao2_calc);
	glBindFramebuffer(GL_FRAMEBUFFER, fbos.hbao2_calc);
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textures.hbao2_resultarray, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	GLfloat lastTime = glfwGetTime();
	// Main Game loop
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


		
		mat4 projection = perspective(radians(camera.Zoom), (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 1.0f, 2000.0f);
		
		mat4 view = camera.GetViewMatrix();
		mat4 model;

		//hbao uniforms
		hbaoData.projInfo = vec4(2.0f / projection[0][0], 2.0f / projection[1][1], -1.0f / projection[0][0], -1.0f / projection[1][1]);
		hbaoData.InvFullResolution = vec2(1.0f / float(SCR_WIDTH), 1.0f / float(SCR_HEIGHT));

		hbaoData.InvQuarterResolution = vec2(1.0f / float(quarterWidth), 1.0f / float(quarterHeight));

		hbaoData.R = 1.0f;
		hbaoData.NegInvR2 = -1.0f / (hbaoData.R * hbaoData.R);
		hbaoData.RadiusToScreen = hbaoData.R * 0.5f * SCR_HEIGHT / (tanf(radians(camera.Zoom) * 0.5f) * 2.0f);
		hbaoData.PowExponent = 2.0f;
		hbaoData.NDotVBias = 0.1f;
		hbaoData.AOMultiplier = 1.0f / (1.0f - hbaoData.NDotVBias);
		
		//bind scene buffer
		{
			glBindFramebuffer(GL_FRAMEBUFFER, fbos.scene);

			vec4  bgColor(0.0, 0.0, 0.0, 0.0);
			glClearBufferfv(GL_COLOR, 0, &bgColor.x);


			glClearDepth(1.0);
			glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);

			model = glm::mat4();
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, -4.0));
			model = glm::rotate(model, 0.0f, glm::vec3(1.0, 0.0, 0.0));
			model = glm::scale(model, glm::vec3(1.0f));

			shaderDrawScene.Use();

			glUniformMatrix4fv(glGetUniformLocation(shaderDrawScene.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
			glUniformMatrix4fv(glGetUniformLocation(shaderDrawScene.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(shaderDrawScene.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			
			scene.Draw(shaderDrawScene);
			
		
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		
		//bind lineardepth buffer
		{
			glBindFramebuffer(GL_FRAMEBUFFER, fbos.depthlinear);
			shaderLinearDepth.Use();
			
			//glUniform4f(0, projection.nearplane * projection.farplane, projection.nearplane - projection.farplane, projection.farplane, projection.ortho ? 0.0f : 1.0f);
			//far near 
			glUniform1f(glGetUniformLocation(shaderLinearDepth.Program, "near"), 1.0);
			glUniform1f(glGetUniformLocation(shaderLinearDepth.Program, "far"), 2000.0);
			glBindMultiTextureEXT(GL_TEXTURE0, GL_TEXTURE_2D, textures.scene_depthstencil);
			
			RenderQuad();
			//glDrawArrays(GL_TRIANGLES, 0, 3);
			glBindMultiTextureEXT(GL_TEXTURE0, GL_TEXTURE_2D, 0);
		}

		
	
		//ssao
		if (type == 0) {

			{
				glBindFramebuffer(GL_FRAMEBUFFER, fbos.hbao_calc);

				glDrawBuffer(GL_COLOR_ATTACHMENT0);
			}
			shaderSSAO.Use();

			// Sample kernel
			std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
			std::default_random_engine generator;
			std::vector<glm::vec3> ssaoKernel;
			for (GLuint i = 0; i < 64; ++i)
			{
				glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0);
				sample = glm::normalize(sample);
				sample *= randomFloats(generator);
				GLfloat scale = GLfloat(i) / 64.0;

				// Scale samples s.t. they're more aligned to center of kernel
				scale = lerp(0.1f, 1.0f, scale * scale);
				sample *= scale;
				ssaoKernel.push_back(sample);
			}

			for (GLuint i = 0; i < 64; ++i)
				glUniform3fv(glGetUniformLocation(shaderSSAO.Program, ("samples[" + std::to_string(i) + "]").c_str()), 1, &ssaoKernel[i][0]);
			glUniformMatrix4fv(glGetUniformLocation(shaderSSAO.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

			glUniform4fv(glGetUniformLocation(shaderSSAO.Program, "projInfo"), 1, &hbaoData.projInfo[0]);			
			glUniform2fv(glGetUniformLocation(shaderSSAO.Program, "InvFullResolution"), 1, &hbaoData.InvFullResolution[0]);

			glBindMultiTextureEXT(GL_TEXTURE0, GL_TEXTURE_2D, textures.scene_depthlinear);
			glBindMultiTextureEXT(GL_TEXTURE1, GL_TEXTURE_2D, textures.hbao_randomview[0]);

			RenderQuad();

		}

		//hbao
		if (type == 1) {
			{
				glBindFramebuffer(GL_FRAMEBUFFER, fbos.hbao_calc);

				glDrawBuffer(GL_COLOR_ATTACHMENT0); 
			}

			shaderHBAO.Use();


			glUniform4fv(glGetUniformLocation(shaderHBAO.Program, "projInfo"), 1, &hbaoData.projInfo[0]);
			//glUniform2fv(glGetUniformLocation(shaderHBAO.Program, "InvQuarterResolution"), 1, &hbaoData.InvQuarterResolution[0]);
			glUniform2fv(glGetUniformLocation(shaderHBAO.Program, "InvFullResolution"), 1, &hbaoData.InvFullResolution[0]);

			glUniform1f(glGetUniformLocation(shaderHBAO.Program, "NegInvR2"), hbaoData.NegInvR2);
			glUniform1f(glGetUniformLocation(shaderHBAO.Program, "NDotVBias"), hbaoData.NDotVBias);
			glUniform1f(glGetUniformLocation(shaderHBAO.Program, "AOMultiplier"), hbaoData.AOMultiplier);
			glUniform1f(glGetUniformLocation(shaderHBAO.Program, "RadiusToScreen"), hbaoData.RadiusToScreen);
			glUniform1f(glGetUniformLocation(shaderHBAO.Program, "PowExponent"), hbaoData.PowExponent);

			glBindMultiTextureEXT(GL_TEXTURE0, GL_TEXTURE_2D, textures.scene_depthlinear);
			glBindMultiTextureEXT(GL_TEXTURE1, GL_TEXTURE_2D, textures.hbao_randomview[0]);

			RenderQuad();

		}

		//hbao +
		if (type == 3)
		{
			//hbao ++ 

			{
				//viewnormal

				glBindFramebuffer(GL_FRAMEBUFFER, fbos.viewnormal);

				//glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
				shaderViewNormal.Use();

				glUniform4fv(glGetUniformLocation(shaderViewNormal.Program, "projInfo"), 1, &hbaoData.projInfo[0]);
				glUniform2fv(glGetUniformLocation(shaderViewNormal.Program, "InvFullResolution"), 1, &hbaoData.InvFullResolution[0]);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, textures.scene_depthlinear);



				RenderQuad();


				glBindTexture(GL_TEXTURE_2D, 0);


			}

			{

				//deinter
				glBindFramebuffer(GL_FRAMEBUFFER, fbos.hbao2_deinterleave);
				//glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
				glViewport(0, 0, quarterWidth, quarterHeight);

				shaderDeinterleave.Use();

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, textures.scene_depthlinear);

				for (int i = 0; i < HBAO_RANDOM_ELEMENTS; i += NUM_MRT) {
					glUniform4f(0, float(i % 4) + 0.5f, float(i / 4) + 0.5f, hbaoData.InvFullResolution[0], hbaoData.InvFullResolution[1]);//info

					for (int layer = 0; layer < NUM_MRT; layer++) {
						glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + layer, textures.hbao2_depthview[i + layer], 0);
					}

					RenderQuad();
					//glDrawArrays(GL_TRIANGLES, 0, 3);
				}

			}
			{
				//hbaoplus
				glBindFramebuffer(GL_FRAMEBUFFER, fbos.hbao2_calc);
				glViewport(0, 0, quarterWidth, quarterHeight);

				shaderHBAOPlus.Use();

				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, textures.scene_viewnormal);

				glUniform4fv(glGetUniformLocation(shaderHBAOPlus.Program, "projInfo"), 1, &hbaoData.projInfo[0]);
				glUniform2fv(glGetUniformLocation(shaderHBAOPlus.Program, "InvQuarterResolution"), 1, &hbaoData.InvQuarterResolution[0]);


				glUniform1f(glGetUniformLocation(shaderHBAOPlus.Program, "NegInvR2"), hbaoData.NegInvR2);
				glUniform1f(glGetUniformLocation(shaderHBAOPlus.Program, "NDotVBias"), hbaoData.NDotVBias);
				glUniform1f(glGetUniformLocation(shaderHBAOPlus.Program, "AOMultiplier"), hbaoData.AOMultiplier);
				glUniform1f(glGetUniformLocation(shaderHBAOPlus.Program, "RadiusToScreen"), hbaoData.RadiusToScreen);
				glUniform1f(glGetUniformLocation(shaderHBAOPlus.Program, "PowExponent"), hbaoData.PowExponent);


				//uniform vec4 projInfo;
				//uniform vec2 InvFullResolution;
				//uniform float NegInvR2;
				//uniform float NDotVBias;
				//uniform float AOMultiplier;
				//uniform float RadiusToScreen;
				//uniform float PowExponent;

				////not layered
				//uniform vec2 g_Float2Offset;
				//uniform vec4 g_Jitter;


				for (int i = 0; i < HBAO_RANDOM_ELEMENTS; i++) {
					glUniform2f(glGetUniformLocation(shaderHBAOPlus.Program, "g_Float2Offset"), float(i % 4) + 0.5f, float(i / 4) + 0.5f);//
					glUniform4fv(glGetUniformLocation(shaderHBAOPlus.Program, "g_Jitter"), 1, &hbaoRandom[i][0]);//jitter
																												 //glActiveTexture(GL_TEXTURE0);
																												 //glBindTexture(GL_TEXTURE_2D, hbaoplusDepthView[i]);

					glBindMultiTextureEXT(GL_TEXTURE0, GL_TEXTURE_2D, textures.hbao2_depthview[i]);
					glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textures.hbao2_resultarray, 0, i);

					RenderQuad();


				}


			}
			{
				//reinter

				{
					glBindFramebuffer(GL_FRAMEBUFFER, fbos.hbao_calc);

					glDrawBuffer(GL_COLOR_ATTACHMENT0); //??
				}

				
				glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

				shaderReinterleave.Use();

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D_ARRAY, textures.hbao2_resultarray);

				RenderQuad();

				glBindMultiTextureEXT(GL_TEXTURE0, GL_TEXTURE_2D_ARRAY, 0);
			}



			
		}



		//blur bilateral
			{
				float meters2viewspace = 1.0f;
				shaderBlur.Use();
				glBindMultiTextureEXT(GL_TEXTURE1, GL_TEXTURE_2D, textures.scene_depthlinear);
				float blurSharpness = 4.0f;
				glUniform1f(0, blurSharpness / meters2viewspace);

				glDrawBuffer(GL_COLOR_ATTACHMENT1);
				glBindMultiTextureEXT(GL_TEXTURE0, GL_TEXTURE_2D, textures.hbao_result);

				glUniform2f(1, 1.0f / float(SCR_WIDTH), 0);
				RenderQuad();
				//glDrawArrays(GL_TRIANGLES, 0, 3);

				// final output to main fbo
				glBindFramebuffer(GL_FRAMEBUFFER, fbos.scene);
				glDisable(GL_DEPTH_TEST);
				glEnable(GL_BLEND);
				glBlendFunc(GL_ZERO, GL_SRC_COLOR);

				glBindMultiTextureEXT(GL_TEXTURE0, GL_TEXTURE_2D, textures.hbao_blur);
				glUniform2f(1, 0, 1.0f / float(SCR_HEIGHT));
				RenderQuad();
				//glDrawArrays(GL_TRIANGLES, 0, 3);
				glDisable(GL_BLEND);
				glEnable(GL_DEPTH_TEST);
				glDisable(GL_SAMPLE_MASK);
				glSampleMaski(0, ~0);

				glBindMultiTextureEXT(GL_TEXTURE0, GL_TEXTURE_2D, 0);
				glBindMultiTextureEXT(GL_TEXTURE1, GL_TEXTURE_2D, 0);


			}


		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glDisable(GL_SAMPLE_MASK);
		glSampleMaski(0, ~0);

		glBindMultiTextureEXT(GL_TEXTURE0, GL_TEXTURE_2D, 0);
		glBindMultiTextureEXT(GL_TEXTURE1, GL_TEXTURE_2D, 0);

		glUseProgram(0);

		{
			
			// blit to background
			glBindFramebuffer(GL_READ_FRAMEBUFFER, fbos.scene);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT,
				0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}


		

		// Swap the buffers
		glfwSwapBuffers(window);

	}

	glfwTerminate();
	return 0;
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
	if (keys[GLFW_KEY_O])
	{
		type = 0;
	}
	if (keys[GLFW_KEY_N])
	{
		type = 2;
	}

	if (keys[GLFW_KEY_P])
	{
		type = 3;
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
	//if (keys[GLFW_KEY_MINUS])
	//{
	//	if (tessLevel > 1.0f)
	//		tessLevel -= 0.5f;
	//	//printf("%f", tessLevel);
	//}
	//if (keys[GLFW_KEY_EQUAL])
	//{
	//	tessLevel += 0.5f;
	//	//printf("%f", tessLevel);
	//}
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