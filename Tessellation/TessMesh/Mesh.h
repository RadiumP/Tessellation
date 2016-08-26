#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
using namespace std;
using namespace glm;
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp\Importer.hpp>
#include "Shader.h"

struct Vertex {
	vec3 Position;
	vec3 Normal;
	vec2 TexCoords;
};

struct Texture {
	GLuint id;
	string type;
	aiString path;
};

class Mesh {
public: 
	vector<Vertex> vertices;

	vector<GLuint> indices;
	vector<Texture> textures;

	vector<vec3> Position;
	vector<vec3> Normal;
	vector<vec2> TexCoord;
	//not working?
	Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures)
	{
		this->vertices = vertices;
		
		this->indices = indices;
		this->textures = textures;

		this->setupMesh();
		printf("here");
	}
	Mesh(vector<vec3> Position, vector<vec3> Normal, vector<vec2> TexCoord, vector<GLuint> indices, vector<Texture> textures)
	{
		this->Position = Position;
		this->Normal = Normal;
		this->TexCoord = TexCoord;

		this->indices = indices;
		this->textures = textures;

		this->setupMesh();

		
	}

	Mesh(vector<vec3> Position, vector<vec3> Normal, vector<vec2> TexCoord,  vector<GLuint> indices)
	{
		this->Position = Position;
		this->Normal = Normal;
		this->TexCoord = TexCoord;

		this->indices = indices;
		//this->textures = textures;

		this->setupMesh();


	}



	void Draw(Shader shader)
	{
		GLuint diffuseNr = 1;
		GLuint specularNr = 1;

		for (GLuint i = 0; i < this->textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);

			stringstream ss;
			string number;
			string name = this->textures[i].type;
			if (name == "texture_diffuse")
				ss << diffuseNr++;
			else if (name == "texture_specular")
				ss << specularNr++;
			number = ss.str();
			glUniform1i(glGetUniformLocation(shader.Program, (name + number).c_str()), i);
			glBindTexture(GL_TEXTURE_2D, this->textures[i].id);

		

		}

		glUniform1f(glGetUniformLocation(shader.Program, "material.shininess"), 16.0f);
		//
		////Draw mesh
		////glBindVertexArray(this->VAO);


		////glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
		//
		//glPatchParameteri(GL_PATCH_VERTICES, 3);
		////glDrawArrays(GL_PATCHES, 0, this->indices.size());

		////wireframe
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//glDrawElements(GL_PATCHES, this->indices.size(), GL_UNSIGNED_INT, 0);
		//
		////glBindVertexArray(0);

		//new Draw
		//v 
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, this->pVBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		
		//vn
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, this->nVBO);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		
		//vt
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, this->tVBO);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glPatchParameteri(GL_PATCH_VERTICES, 3);

		int size;
		glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
		glDrawElements(GL_PATCHES, size/sizeof(GLuint), GL_UNSIGNED_INT, 0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		for (GLuint i = 0; i < this->textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

	}

private:

	GLuint VAO, VBO, EBO;
	GLuint pVBO, nVBO, tVBO;

	void setupMesh()
	{
		
		//glGenVertexArrays(1, &this->VAO);
		//glGenBuffers(1, &this->VBO);
		//glGenBuffers(1, &this->EBO);

		//glBindVertexArray(this->VAO);

		//glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		//glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);

		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);

		////v 
		//glEnableVertexAttribArray(0);
		////glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);

		////vn
		//glEnableVertexAttribArray(1);
		////glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));

		////vt
		//glEnableVertexAttribArray(2);
		////glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords));

		//glBindVertexArray(0);


		//new VBO
		glGenVertexArrays(1, &this->VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &this->pVBO);
		glBindBuffer(GL_ARRAY_BUFFER, this->pVBO);
		glBufferData(GL_ARRAY_BUFFER, this->Position.size() * sizeof(this->Position[0]), &this->Position[0], GL_STATIC_DRAW);


		glGenBuffers(1, &this->nVBO);
		glBindBuffer(GL_ARRAY_BUFFER, this->nVBO);
		glBufferData(GL_ARRAY_BUFFER, this->Normal.size() * sizeof(this->Normal[0]), &this->Normal[0], GL_STATIC_DRAW);

		glGenBuffers(1, &this->tVBO);
		glBindBuffer(GL_ARRAY_BUFFER, this->tVBO);
		glBufferData(GL_ARRAY_BUFFER, this->TexCoord.size() * sizeof(this->TexCoord[0]), &this->TexCoord[0], GL_STATIC_DRAW);

		glGenBuffers(1, &this->EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);




	}

};