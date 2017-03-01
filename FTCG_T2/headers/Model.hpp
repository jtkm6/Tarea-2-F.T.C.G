#pragma once

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include <Mesh.hpp>

#include <GL/glew.h>

#include <Windows.h>

#include <stb_image.h>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

GLint TextureFromFile(const char* path, std::string directory){
	std::string filename = std::string(path);
	filename = directory + '/' + filename;
	GLuint textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);
	int width, height;
	unsigned char* image = stbi_load(filename.c_str(), &width, &height, 0, STBI_rgb);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(image);
	return textureID;
}

class Model{
public:

	// Constructor, expects a filepath to a 3D model.
	Model(GLchar* path){
		loadModel(path);
	}

	// Draws the model, and thus all its meshes
	void Draw(Shader shader){
		for(GLuint i = 0; i < meshes.size(); i++)
			meshes[i].Draw(shader);
	}

	void AddNormalTexture(){

	}

	std::vector<Mesh> meshes;
	std::string directory;
	std::vector<Texture> textures_loaded;
	
	// Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	void loadModel(std::string path){
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace);
		if(!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
			OutputDebugStringA("ERROR::ASSIMP:: ");
			OutputDebugStringA(importer.GetErrorString());
			return;
		}
		directory = path.substr(0, path.find_last_of('/'));
		processNode(scene->mRootNode, scene);
	}

	// Processes a node in a recursive fashion. Processes each individual mesh located at the node
	// and repeats this process on its children nodes (if any).
	void processNode(aiNode* node, const aiScene* scene){
		for(GLuint i = 0; i < node->mNumMeshes; i++){
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}
		for(GLuint i = 0; i < node->mNumChildren; i++){
			processNode(node->mChildren[i], scene);
		}
	}

	Mesh processMesh(aiMesh* mesh, const aiScene* scene){
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;
		std::vector<Texture> textures;
		for(GLuint i = 0; i < mesh->mNumVertices; i++){
			Vertex vertex;
			glm::vec3 vector;
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.Normal = vector;
			if(mesh->HasTangentsAndBitangents()){
				vector.x = mesh->mTangents[i].x;
				vector.y = mesh->mTangents[i].y;
				vector.z = mesh->mTangents[i].z;
				vertex.Tangent = vector;
				vector.x = mesh->mBitangents[i].x;
				vector.y = mesh->mBitangents[i].y;
				vector.z = mesh->mBitangents[i].z;
				vertex.BiTangent = vector;
			}
			if(mesh->mTextureCoords[0]){
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}else{
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);
			}
			vertices.push_back(vertex);
		}
		for(GLuint i = 0; i < mesh->mNumFaces; i++){
			aiFace face = mesh->mFaces[i];
			for(GLuint j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
		if(mesh->mMaterialIndex >= 0){
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
			std::vector<Texture> reflectionMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_reflection");
			textures.insert(textures.end(), reflectionMaps.begin(), reflectionMaps.end());
			std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
			textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		}
		return Mesh(vertices, indices, textures);
	}

	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName){
		std::vector<Texture> textures;
		for(GLuint i = 0; i < mat->GetTextureCount(type); i++){
			aiString str;
			mat->GetTexture(type, i, &str);
			GLboolean skip = false;
			for(GLuint j = 0; j < textures_loaded.size(); j++){
				if(textures_loaded[j].path == str){
					textures.push_back(textures_loaded[j]);
					skip = true;
					break;
				}
			}
			if(!skip){
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), directory);
				texture.type = typeName;
				texture.path = str;
				textures.push_back(texture);
				textures_loaded.push_back(texture);
			}
		}
		return textures;
	}
};
