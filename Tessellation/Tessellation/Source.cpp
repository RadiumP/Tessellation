#include <GL/glew.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <windows.h> 

using namespace std;
using namespace glm;
#include "Shader.h"

static void RenderSceneCB()
{

	glClearDepth(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	static bool init = true;
	static Shader triShader("../Shaders/Tess.vs", "../Shaders/Tess.fs", "../Shaders/Tess.tcs", "../Shaders/Tess.tes", "../Shaders/Tess.gs");
	//static Shader triShader("../Shaders/Tess.vs", "../Shaders/Tess.fs");
	static int VBOvert = 0;
	const int num_floats = 4 * 4;

	if (init)
	{
	/*	triShader.attach(GL_VERTEX_SHADER, "../Shaders/Tess.vs");
		triShader.attach(GL_FRAGMENT_SHADER, "../Shaders/Tess.fs");
		triShader.attach(GL_GEOMETRY_SHADER, "../Shaders/Tess.gs");
		triShader.attach(GL_TESS_CONTROL_SHADER, "../Shaders/Tess.tcs");
		triShader.attach(GL_TESS_EVALUATION_SHADER, "../Shaders/Tess.tes");
		triShader.link();*/


		float vert[num_floats] = {
			-0.3 , -0.3 , 0 , 0.3,	// upleft
			0.3 , -0.3 , 0 , 0.6,	// upright
			0.3 ,  0.3 , 0 , 0.4,	// downright
			-0.3 ,  0.3 , 0 , 0.1	// downleft
		};

		glGenBuffers(1, (GLuint *)(&VBOvert));
		glBindBuffer(GL_ARRAY_BUFFER, VBOvert);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*num_floats, vert, GL_DYNAMIC_DRAW_ARB);
		glPointSize(10);

		init = false;

	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();//I

	float Projection[16];
	float Modelview[16];
	glGetFloatv(GL_PROJECTION_MATRIX, Projection);   
	glTranslatef(-0.5, 0, 0);
	glGetFloatv(GL_MODELVIEW_MATRIX, Modelview);

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBOvert);    
	glEnableClientState(GL_VERTEX_ARRAY);  
	glVertexPointer(4, GL_FLOAT, 0, (char*)0);

	triShader.Use();
	glUniformMatrix4fv(glGetUniformLocation(triShader.Program, "projectionMatrix"), 1, GL_FALSE, Projection);
	glUniformMatrix4fv(glGetUniformLocation(triShader.Program, "modelViewMatrix"), 1, GL_FALSE, Modelview);
	/*triShader.setUniformMatrix4fv("projectionMatrix", 1, 0, Projection); 
	triShader.setUniformMatrix4fv("modelViewMatrix", 1, 0, Modelview);*/
	glPatchParameteri(GL_PATCH_VERTICES, 3);
	glDrawArrays(GL_PATCHES, 0, 3);
	//triShader.end();

	glDisableClientState(GL_VERTEX_ARRAY);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	glutSwapBuffers();


}







int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
	glutInitWindowSize(1024, 768);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Tessellation");



	GLenum res = glewInit();
	if (res != GLEW_OK)
	{
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}


	glutDisplayFunc(RenderSceneCB);
	glutIdleFunc(RenderSceneCB);

	glutMainLoop();



	return 0;
}