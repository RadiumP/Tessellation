#pragma once
#ifndef LOADMESH_H
#define LOADMESH_H

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <assimp\scene.h>
#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assert.h>
#include <SOIL\SOIL.h>
#include <map>

#include "DrawMesh.h"

#include "Shader.h"



using namespace glm;
using namespace std;


class Model
{
public:
	Model(GLchar* path)
	{
		//this->loadModel(path);
		this->loadObj(path);
	}
	void Draw(Shader shader)
	{
		for (GLuint i = 0; i < this->meshes.size(); i++)
			this->meshes[i].Draw(shader);
	}

private:
	vector<Mesh> meshes;
	string directory;
	vector<Texture> textures_loaded;
	
	vector<Vertex> vertex;
	
	vector<Texture> textures;
	vector<vec3> vertices;
	vector<vec3> normals;
	vector<vec2> texcoords;
	vector<GLuint> indices;
	//Mesh mesh();

	void loadObj(string path)
	{
		vertices.clear();
		normals.clear();
		texcoords.clear();
		indices.clear();
		
		ifstream inpfile(path.c_str());
		if (!inpfile.is_open())
		{
			cout << "Unable to open file" << endl;
			return;
		}

		string line;
		while (!getline(inpfile, line).eof())
		{
			vector<string> splitline;
			string buf;

			stringstream ss(line);
			while (ss >> buf)
			{
				splitline.push_back(buf);
			}
			//Ignore blank lines
			if (splitline.size() == 0)
			{
				continue;
			}
			//Vertex
			if (splitline[0][0] == 'v')
			{
				if (splitline[0].length() > 1 && splitline[0][1] == 'n')
				{

				}
				else if (splitline[0].length() > 1 && splitline[0][1] == 't')
				{
					texcoords.push_back(vec2(atof(splitline[1].c_str()), atof(splitline[2].c_str())));
				}
				else
				{
					vertices.push_back(vec3(atof(splitline[1].c_str()), atof(splitline[2].c_str()), atof(splitline[3].c_str())));
				}
			}

			//Face
			else if (splitline[0][0] == 'f')
			{
				int v1, v2, v3;
				int n1, n2, n3;
				int t1, t2, t3;

				int num_slash = 0;
				bool double_slash = false;
				for (unsigned int i = 0; i < splitline[1].length(); i++)
				{
					if (splitline[1][i] == '/')
					{
						num_slash++;
						if (i + 1 < splitline[1].length() && splitline[1][i + 1] == '/')
						{
							double_slash = true;
						}
					}
				}
				if (num_slash == 0)
				{
					sscanf(line.c_str(), "f %d %d %d", &v1, &v2, &v3);
				}
				else if (num_slash == 1)
				{
					sscanf(line.c_str(), "f %d/%*d %d/%*d %d/%*d", &v1, &v2, &v3);
				}
				else if (num_slash == 2)
				{
					if (double_slash)
					{
						sscanf(line.c_str(), "f %d//%d %d//%d %d//%d", &v1, &n1, &v2, &n2, &v3, &n3);
					}
					else
					{
						sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d", &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3);
					}
				}
				else
				{
					cout << "Too many slashes" << endl;
					continue;
				}
				indices.push_back(v1 - 1);
				indices.push_back(v2 - 1);
				indices.push_back(v3 - 1);
		
			}


		}
		inpfile.close();
		//geometric averaged normal
		vector<int> nb_seen;
		normals.resize(vertices.size(), vec3(0.0, 0.0, 0.0));
		nb_seen.resize(vertices.size(), 0);
		for (int i = 0; i < indices.size(); i += 3)
		{
			GLuint ia = indices[i];
			GLuint ib = indices[i + 1];
			GLuint ic = indices[i + 2];

			vec3 normal = normalize(cross(vec3(vertices[ib]) - vec3(vertices[ia]), vec3(vertices[ic]) - vec3(vertices[ia])));

			int v[3];
			v[0] = ia; 
			v[1] = ib;
			v[2] = ic;

			for (int j = 0; j < 3; j++)
			{
				GLuint cur_v = v[j];
				++nb_seen[cur_v];
				if (nb_seen[cur_v] == 1)
				
				{
					normals[cur_v] = normal;
				}
				else
				{
					normals[cur_v].x = normals[cur_v].x * (1.0 - 1.0 / nb_seen[cur_v]) + normal.x * 1.0 / nb_seen[cur_v];
					normals[cur_v].y = normals[cur_v].y * (1.0 - 1.0 / nb_seen[cur_v]) + normal.y * 1.0 / nb_seen[cur_v];
					normals[cur_v].z = normals[cur_v].z * (1.0 - 1.0 / nb_seen[cur_v]) + normal.z * 1.0 / nb_seen[cur_v];
					normals[cur_v] = normalize(normals[cur_v]);
				}
			}

		}
		this->meshes.push_back(Mesh(vertices,normals,texcoords, indices));
		//Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures)
		//this->meshes.push_back(Mesh(vertex, indices, textures));
	}



	void loadModel(string path)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcessPreset_TargetRealtime_Quality | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals);//assimp cannot differ vertex with different normal.
		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			cout << "Error:: ASSIMP:: " << importer.GetErrorString() << endl;
			return;
		}

		this->directory = path.substr(0, path.find_last_of('/'));

		this->processNode(scene->mRootNode, scene);

	}


	void processNode(aiNode* node, const aiScene* scene)
	{
		for (GLuint i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			this->meshes.push_back(this->processMesh(mesh, scene));
		}

		for (GLuint i = 0; i < node->mNumChildren; i++)
		{
			this->processNode(node->mChildren[i], scene);
		}


	}
	Mesh processMesh(aiMesh* mesh, const aiScene* scene) 
	{
		vector<Vertex> vertices;
	
		vector<GLuint> indices;
		vector<Texture> textures;

		vector<vec3> Position;
		vector<vec3> Normal;
		vector<vec2> TexCoord;

		for (GLuint i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			vec3 vector;
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;
			Position.push_back(vector);
			
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.Normal = vector;
			Normal.push_back(vector);

			//loading is ok
			//printf("%f,%f,%f", vertex.Normal.x, vertex.Normal.y, vertex.Normal.z);
			if (mesh->mTextureCoords[0])
			{
				vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
				
			}
			else
			{
				vertex.TexCoords = vec2(0.0f, 0.0f);
				
			}
			
			vertices.push_back(vertex);
			//Position.push_back(vertex.Position);
			//Normal.push_back(vertex.Normal);
			TexCoord.push_back(vertex.TexCoords);
		}

		for (GLuint i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (GLuint j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			//diffuse
			vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

			//specular
			vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_supecular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		
		
		}



		//return Mesh(vertices, indices, textures);
		vector<vec3> nPosition;
		vector<vec3> nNormal;
		vector<vec2> nTexCoord;
		vector<GLuint> nindices;
		indexVBO(Position, Normal, TexCoord, nindices, nPosition, nNormal, nTexCoord);
		//return Mesh(nPosition, nNormal, nTexCoord, nindices, textures);
		
		//return Mesh(Position, Normal, TexCoord, indices, textures);
		return Mesh(vertices, indices, textures);

	}


	
	vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
	{
		vector <Texture> textures;
		for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);

			GLboolean skip = false;
			for (GLuint j = 0; j < textures_loaded.size(); j++)
			{
				if (textures_loaded[j].path == str)
				{
					textures.push_back(textures_loaded[j]);
					skip = true;
					break;
				}
			}
			if (!skip)
			{
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), this->directory);
				texture.type = typeName;
				texture.path = str;
				textures.push_back(texture);
				this->textures_loaded.push_back(texture);
			}

		}
		return textures;

	}
	GLint TextureFromFile(const char* path, string directory)
	{
		string filename = string(path);
		filename = directory + '/' + filename;
		GLuint textureID;
		glGenTextures(1, &textureID);
		int width, height;
		unsigned char* image = SOIL_load_image(filename.c_str(), &width, &height, 0, SOIL_LOAD_RGB);

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		SOIL_free_image_data(image);


		return textureID;


	}

	struct PackedVertex {
		vec3 position;
		vec2 uv;
		vec3 normal;
		bool operator<(const PackedVertex that) const {
			return memcmp((void*)this, (void*)&that, sizeof(PackedVertex))>0;
		};
	};

	bool getSimilarVertexIndex_fast(
		PackedVertex & packed,
		std::map<PackedVertex, GLuint> & VertexToOutIndex,
		GLuint & result
	) {
		std::map<PackedVertex, GLuint>::iterator it = VertexToOutIndex.find(packed);
		if (it == VertexToOutIndex.end()) {
			return false;
		}
		else {
			result = it->second;
			return true;
		}
	}

	void indexVBO(vector<vec3> &position, vector<vec3> &normal, vector<vec2> &texcoord, vector<GLuint> &out_indices, vector<vec3> &newPosition, vector<vec3> &newNormal, vector<vec2> &newTexCoord)
	{
		std::map<PackedVertex, GLuint> VertexToOutIndex;

		// For each input vertex
		for (unsigned int i = 0; i<position.size(); i++) {

			PackedVertex packed = { position[i], texcoord[i], normal[i] };


			// Try to find a similar vertex in out_XXXX
			GLuint index;
			bool found = getSimilarVertexIndex_fast(packed, VertexToOutIndex, index);

			if (found) { // A similar vertex is already in the VBO, use it instead !
				out_indices.push_back(index);
			}
			else { // If not, it needs to be added in the output data.
				newPosition.push_back(position[i]);
				newTexCoord.push_back(texcoord[i]);
				newNormal.push_back(normal[i]);
				GLuint newindex = (GLuint)newPosition.size() - 1;
				out_indices.push_back(newindex);
				VertexToOutIndex[packed] = newindex;
			}
		}
	}


};



//bool loadassimp(
//	
//	const char* path,
//	vector<unsigned short> & indices,
//	vector<vec3> & vertices,
//	vector<vec2> & uvs,
//	vector<vec3> & normals
//
//);


#endif // !LOADMESH_H
