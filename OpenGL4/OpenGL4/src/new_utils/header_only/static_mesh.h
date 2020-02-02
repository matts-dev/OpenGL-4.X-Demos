#pragma once

//standard libraries
#include <string>
#include <vector>
#include <iostream>

//window library and OpenGL libraries
#include <glad/glad.h> //note: sets up OpenGL headers, so should be before anything that uses those headers (such as GLFW)
#include <GLFW/glfw3.h>

//math libraries
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

//model loading
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//texture loading
#include <stb_image.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// These classes exist to help with demonstrations. They are not optimized in any sense.
// Nor do these classes represent any best practices.
//
// These classes are inspired from the LearnOpenGL model loading and normal mapping tutorials.
// https://learnopengl.com/Model-Loading/Assimp
// https://learnopengl.com/Advanced-Lighting/Normal-Mapping
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace StaticMesh
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 textureCoords;
	};

	struct NormalData
	{
		glm::vec3 tangent;
		glm::vec3 bitangent;
	};

	struct MaterialTexture
	{
		unsigned int id;
		std::string type;
		std::string path;
	};

	/**
	 * Represents a collection of renderable vertices that have been configured for OpenGL.  
	 */
	class Mesh final
	{
	public:
		Mesh(std::vector<Vertex>& vertices,
			std::vector<MaterialTexture>& textures,
			std::vector<unsigned int>& indices,
			std::vector<NormalData>& normalData)
			: vertices(vertices),
			textures(textures),
			indices(indices),
			normalData(normalData)
		{
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &VBO_TANGENTS);
			glGenBuffers(1, &EAO);

			glBindVertexArray(VAO);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EAO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &indices[0], GL_STATIC_DRAW);

			//enable vertex data 
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(0));
			glEnableVertexAttribArray(0);

			//enable normal data
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));
			glEnableVertexAttribArray(1);

			//enable texture coordinate data
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, textureCoords)));
			glEnableVertexAttribArray(2);

			//crashes on glBufferData when all data contained in a single VBO; spliting up by using another VBO
			//enable tangent storage
			glBindBuffer(GL_ARRAY_BUFFER, VBO_TANGENTS);
			glBufferData(GL_ARRAY_BUFFER, sizeof(NormalData) * normalData.size(), &normalData[0], GL_STATIC_DRAW);
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(NormalData), reinterpret_cast<void*>(0));
			glEnableVertexAttribArray(3);

			//enable bitangent
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(NormalData), reinterpret_cast<void*>(offsetof(NormalData, bitangent)));
			glEnableVertexAttribArray(4);
		}

		~Mesh()
		{
			//do not clear opengl resources here unless correct copy/move management is done.
		}

		void cleanup()
		{
			//this is not done in the dtor of mesh because it will cause issues with copy construction
			glDeleteBuffers(1, &VBO);
			glDeleteBuffers(1, &EAO);
			glDeleteBuffers(1, &modelVBO);
			glDeleteVertexArrays(1, &VAO);
		}

		void draw(const GLuint& shaderProg, GLenum draw_primitive = GL_TRIANGLES)
		{
			if (bReleasedGPUResources) { std::cerr << "Attempting to draw after mesh has been cleaned up\n"; }

			unsigned int diffuseTextureNumber = 0;
			unsigned int specularTextureNumber = 0;
			unsigned int normalMapTextureNumber = 0;
			unsigned int currentTextureUnit = GL_TEXTURE0;

			for (unsigned int i = 0; i < textures.size(); ++i)
			{
				std::string uniformName = textures[i].type;
				if (uniformName == "texture_diffuse")
				{
					//naming convention for diffuse is `texture_diffuseN`
					uniformName = std::string("material.") + uniformName + std::to_string(diffuseTextureNumber);
					++diffuseTextureNumber;
				}
				else if (uniformName == "texture_specular")
				{
					uniformName = std::string("material.") + uniformName + std::to_string(specularTextureNumber);
					++specularTextureNumber;
				}
				else if (uniformName == "texture_ambient")
				{
					uniformName = std::string("material.") + uniformName + std::to_string(0);
				}
				else if (uniformName == "texture_normalmap")
				{
					uniformName = std::string("material.") + uniformName + std::to_string(normalMapTextureNumber);
					++normalMapTextureNumber;
				}
				glActiveTexture(currentTextureUnit);
				glBindTexture(GL_TEXTURE_2D, textures[i].id);

				const GLint uniformLoc = glGetUniformLocation(shaderProg, uniformName.c_str());
				glUniform1i(uniformLoc, currentTextureUnit - GL_TEXTURE0); //subtraction makes zero the bottom of this range, but shows association with GL_TEXTURE0
				++currentTextureUnit;
			}

			//draw mesh
			glBindVertexArray(VAO);
			glDrawElements(draw_primitive, indices.size(), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);

		}
		GLuint getVAO() { return VAO; } //todo delete this?
	private:
		std::vector<Vertex> vertices;
		std::vector<MaterialTexture> textures;
		std::vector<unsigned int> indices;
		std::vector<NormalData> normalData;
		GLuint VAO, VBO, VBO_TANGENTS, EAO;
		GLuint modelVBO = 0;
		bool bReleasedGPUResources = false;
	};

	/**
	 *  Represents a collection of meshes.
	 *	This class opens a scene and parses scene into individual models.
	 */
	class Model final
	{
	public:
		/** Special member functions removed to make OpenGL resource management simple */
		Model(const Model& copy) = delete;
		Model(Model&& move) = delete;
		Model& operator=(const Model& copy) = delete;
		Model& operator=(Model&& move) = delete;

		Model(const std::string& executable_relative_file_path)
		{
			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(executable_relative_file_path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			{
				std::cerr << "Assimp: error importing model: ERROR::" << importer.GetErrorString() << std::endl;
				throw std::runtime_error("failed to load required model");
			}
			model_directory = executable_relative_file_path.substr(0, executable_relative_file_path.find_last_of('/'));
			
			processSceneNode(scene->mRootNode, scene);
		}

		~Model()
		{ 
			for (Mesh& mesh : meshes)
			{
				mesh.cleanup();
			}
			for (MaterialTexture& texture : texturesLoaded)
			{
				glDeleteTextures(1, &texture.id);
			}
		}
		void draw(const GLuint& shaderProgram, GLenum draw_primitive)
		{
			for (unsigned int i = 0; i < meshes.size(); ++i)
			{
				meshes[i].draw(shaderProgram, draw_primitive);
			}
		}
	private:
		void processSceneNode(aiNode* node, const aiScene* scene)
		{
			for (unsigned i = 0; i < node->mNumMeshes; ++i)
			{
				aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
				meshes.push_back(processMesh(mesh, scene));
			}
			for (unsigned i = 0; i < node->mNumChildren; ++i)
			{
				processSceneNode(node->mChildren[i], scene);
			}
		}
		Mesh processMesh(aiMesh* mesh, const aiScene* scene)
		{
			std::vector<Vertex> vertices;
			std::vector<unsigned int> indices;
			std::vector<MaterialTexture> textures;
			std::vector<NormalData> normalData;

			for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
			{
				Vertex vertex;
				vertex.position.x = mesh->mVertices[i].x;
				vertex.position.y = mesh->mVertices[i].y;
				vertex.position.z = mesh->mVertices[i].z;

				vertex.normal.x = mesh->mNormals[i].x;
				vertex.normal.y = mesh->mNormals[i].y;
				vertex.normal.z = mesh->mNormals[i].z;

				//check if model has texture coordinates
				if (mesh->mTextureCoords[0])
				{
					vertex.textureCoords.x = mesh->mTextureCoords[0][i].x;
					vertex.textureCoords.y = mesh->mTextureCoords[0][i].y;
				}
				else
				{
					vertex.textureCoords = { 0.f, 0.f };
				}

				NormalData nmData;
				nmData.tangent.x = mesh->mTangents[i].x;
				nmData.tangent.y = mesh->mTangents[i].y;
				nmData.tangent.z = mesh->mTangents[i].z;
				nmData.tangent = glm::normalize(nmData.tangent);

				nmData.bitangent.x = mesh->mBitangents[i].x;
				nmData.bitangent.y = mesh->mBitangents[i].y;
				nmData.bitangent.z = mesh->mBitangents[i].z;
				nmData.bitangent = glm::normalize(nmData.bitangent);

				vertices.push_back(vertex);
				normalData.push_back(nmData);
			}

			for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
			{
				//loader forces faces to be triangles, so these indices should be valid for drawing triangles
				const aiFace& face = mesh->mFaces[i];
				for (unsigned int j = 0; j < face.mNumIndices; ++j)
				{
					indices.push_back(face.mIndices[j]);
				}
			}

			if (mesh->mMaterialIndex >= 0)
			{
				aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

				std::vector<MaterialTexture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
				std::vector<MaterialTexture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
				std::vector<MaterialTexture> ambientMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_ambient");
				std::vector<MaterialTexture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normalmap"); //aiTextureType seems to load normal maps from aiTextureType_HEIGHT, rather than aiTextureType_NORMAL

				textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
				textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
				textures.insert(textures.end(), ambientMaps.begin(), ambientMaps.end());
				textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
			}

			return Mesh(vertices, textures, indices, normalData);
		}
		std::vector<MaterialTexture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
		{
			std::vector<MaterialTexture> textures;

			unsigned int textureCount = mat->GetTextureCount(type);
			for (unsigned int i = 0; i < textureCount; i++)
			{
				aiString str;
				mat->GetTexture(type, i, &str);
				std::string relativePath = std::string(str.C_Str());

				std::string filepath = model_directory + std::string("/") + relativePath;

				bool bShouldSkip = false;
				for (unsigned int j = 0; j < texturesLoaded.size(); ++j)
				{
					if (texturesLoaded[i].path == relativePath)
					{
						//already loaded this texture, just the cached texture information
						textures.push_back(texturesLoaded[j]);
						bShouldSkip = true;
						break;
					}
				}
				if (!bShouldSkip)
				{
					MaterialTexture texture;
					texture.id = loadTexture(filepath.c_str());
					texture.type = typeName;
					texture.path = relativePath;
					textures.push_back(texture);

					//cache for later texture loads
					texturesLoaded.push_back(texture);
				}
			}
			return textures;
		}
		GLuint loadTexture(const char* relative_filepath, int texture_unit = -1, bool useGammaCorrection = false)
		{
			int img_width, img_height, img_nrChannels;
			unsigned char* textureData = stbi_load(relative_filepath, &img_width, &img_height, &img_nrChannels, 0);
			if (!textureData)
			{
				std::cerr << "failed to load texture" << std::endl;
				exit(-1);
			}

			GLuint textureID;
			glGenTextures(1, &textureID);

			if (texture_unit >= 0)
			{
				glActiveTexture(texture_unit);
			}
			glBindTexture(GL_TEXTURE_2D, textureID);

			int mode = -1;
			int dataFormat = -1;
			if (img_nrChannels == 3)
			{
				mode = useGammaCorrection ? GL_SRGB : GL_RGB;
				dataFormat = GL_RGB;
			}
			else if (img_nrChannels == 4)
			{
				mode = useGammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
				dataFormat = GL_RGBA;
			}
			else if (img_nrChannels == 1)
			{
				mode = GL_RED;
				dataFormat = GL_RED;
			}
			else
			{
				std::cerr << "unsupported image format for texture at " << relative_filepath << " there are " << img_nrChannels << "channels" << std::endl;
				exit(-1);
			}

			glTexImage2D(GL_TEXTURE_2D, 0, mode, img_width, img_height, 0, dataFormat, GL_UNSIGNED_BYTE, textureData);
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			stbi_image_free(textureData);

			return textureID;
		}
	private: 
		std::vector<Mesh> meshes;
		std::string model_directory;
		std::vector<MaterialTexture> texturesLoaded;
	};

}