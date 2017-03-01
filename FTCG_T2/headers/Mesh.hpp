#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#include <GL/glew.h>

#include <Shader.hpp>

#include <assimp/Importer.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


struct Vertex{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;
	glm::vec3 BiTangent;
};

struct Texture{
	GLuint id;
	std::string type;
	aiString path;
};

class Mesh{
public:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;

	Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures){
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;
		setupMesh();
	}

	// Render the mesh
	void Draw(Shader shader){
		GLuint diffuseNr = 1;
		GLuint specularNr = 1;
		GLuint reflectionNr = 1;
		GLuint normalNr = 1;
		for(GLuint i = 0; i < textures.size(); i++){
			glActiveTexture(GL_TEXTURE0 + i);
			std::stringstream ss;
			std::string number;
			std::string name = textures[i].type;
			if(name == "texture_diffuse")
				ss << diffuseNr++;
			else if(name == "texture_specular")
				ss << specularNr++;
			else if(name == "texture_reflection")
				ss << reflectionNr++;
			else if(name == "texture_normal")
				ss << normalNr++;
			number = ss.str();
			shader.SetglUniformValue((GLchar*)((name + number).c_str()), i);
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	GLuint VAO, VBO, EBO;

	// Initializes all the buffer objects/arrays
	void setupMesh(){
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Tangent));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, BiTangent));
		glBindVertexArray(0);
	}
};



