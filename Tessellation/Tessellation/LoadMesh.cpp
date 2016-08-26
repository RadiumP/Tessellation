////http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/
//
//
//#include "LoadMesh.h"
//
//bool loadAssImp(
//
//	const char* path,
//	vector<unsigned short> & indices,
//	vector<vec3> & vertices,
//	vector<vec2> & uvs,
//	vector<vec3> & normals
//
//	)
//{
//	Assimp::Importer importer;
//	const aiScene * scene = importer.ReadFile(path, 0);
//
//	if (!scene) {
//		fprintf(stderr, importer.GetErrorString());
//		getchar();
//		return false;
//	}
//
//	const aiMesh * mesh = scene->mMeshes[0];
//
//	//get v pos
//	vertices.reserve(mesh->mNumVertices);
//	for (unsigned int i = 0; i<mesh->mNumVertices; i++) {
//		aiVector3D pos = mesh->mVertices[i];
//		vertices.push_back(vec3(pos.x, pos.y, pos.z));
//	}
//
//	////get v tex
//	//uvs.reserve(mesh->mNumVertices);
//	//for (unsigned int i = 0; i < mesh->mNumVertices; i++)
//	//{
//	//	aiVector3D UVW = mesh->mTextureCoords[0][i];
//	//	uvs.push_back(vec2(UVW.x, UVW.y));
//	//}
//
//	
//
//	////fill vn
//	//normals.reserve(mesh->mNumVertices);
//	//for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
//	//	aiVector3D n = mesh->mNormals[i];
//	//	normals.push_back(vec3(n.x, n.y, n.z));
//	//}
//
//	//fill f indices
//	indices.reserve(3 * mesh->mNumFaces);
//	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
//		indices.push_back(mesh->mFaces[i].mIndices[0]);
//		indices.push_back(mesh->mFaces[i].mIndices[1]);
//		indices.push_back(mesh->mFaces[i].mIndices[2]);
//	}
//
//
//
//
//
//}
