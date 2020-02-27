#pragma once

#include <glad/glad.h> //note: sets up OpenGL headers, so should be before anything that uses those headers (such as GLFW)
#include <GLFW/glfw3.h>

#include<iostream>
#include<string>
#include<optional>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

namespace ho
{
	struct Shader final
	{
		struct ShaderParams
		{
			std::optional<std::string> vertex_src;
			std::optional<std::string> tessellation_control_src;
			std::optional<std::string> tessellation_evaluation_src;
			std::optional<std::string> geometry_src;
			std::optional<std::string> fragment_src;
			std::optional<std::string> compute_src;
		};
	public:
		Shader(const Shader& copy) = delete;
		Shader(Shader&& move) = delete;
		Shader& operator=(const Shader& copy) = delete;
		Shader& operator=(Shader&& move) = delete;
		Shader(ShaderParams& in) : initParams(in)
		{
			bool bIsPipeline = in.vertex_src.has_value() && in.fragment_src.has_value();
			bool bIsCompute = in.compute_src.has_value();
			if ((bIsPipeline || bIsCompute) && !(bIsCompute && bIsPipeline))
			{
				if (bIsCompute)
				{
					GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
					const char* compute_cstr = in.compute_src->c_str();
					glShaderSource(computeShader, 1, &compute_cstr, nullptr);
					glCompileShader(computeShader);
					shaderCompileCheck("compute shader", computeShader);

					shaderProgram = glCreateProgram();
					glAttachShader(shaderProgram, computeShader);
					glLinkProgram(shaderProgram);
					shaderLinkCheck(shaderProgram);

					glDeleteShader(computeShader);
				}
				else if (bIsPipeline)
				{
					GLuint vertexShader = 0, tessControlShader = 0, tessEvalShader = 0, geometryShader = 0, fragmentShader = 0;

					/////////////////////////////////////////////////////////////////////////////////////
					// create the individual shaders
					/////////////////////////////////////////////////////////////////////////////////////
					if (in.vertex_src.has_value())
					{
						vertexShader = glCreateShader(GL_VERTEX_SHADER);
						const char* vert_cstr = in.vertex_src->c_str();
						glShaderSource(vertexShader, 1, &vert_cstr, nullptr);
						glCompileShader(vertexShader);
						shaderCompileCheck("vertex shader", vertexShader);
					}
					if (in.tessellation_control_src.has_value())
					{
						tessControlShader = glCreateShader(GL_TESS_CONTROL_SHADER);
						const char* tcs_cstr = in.tessellation_control_src->c_str();
						glShaderSource(tessControlShader, 1, &tcs_cstr, nullptr);
						glCompileShader(tessControlShader);
						shaderCompileCheck("tessellation control shader", tessControlShader);
					}
					if (in.tessellation_evaluation_src.has_value())
					{
						tessEvalShader = glCreateShader(GL_TESS_EVALUATION_SHADER);
						const char* tes_cstr = in.tessellation_evaluation_src->c_str();
						glShaderSource(tessEvalShader, 1, &tes_cstr, nullptr);
						glCompileShader(tessEvalShader);
						shaderCompileCheck("tessellation evaluation shader", tessEvalShader);
					}
					if (in.geometry_src.has_value())
					{
						geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
						const char* geo_cstr = in.geometry_src->c_str();
						glShaderSource(geometryShader, 1, &geo_cstr, nullptr);
						glCompileShader(geometryShader);
						shaderCompileCheck("geometry shader", geometryShader);
					}
					if (in.fragment_src.has_value())
					{
						fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
						const char* frag_cstr = in.fragment_src->c_str();
						glShaderSource(fragmentShader, 1, &frag_cstr, nullptr);
						glCompileShader(fragmentShader);
						shaderCompileCheck("fragment shader", fragmentShader);
					}

					/////////////////////////////////////////////////////////////////////////////////////
					// link the shaders into a program
					/////////////////////////////////////////////////////////////////////////////////////
					shaderProgram = glCreateProgram();
					if (vertexShader) { glAttachShader(shaderProgram, vertexShader); }
					if (tessControlShader) { glAttachShader(shaderProgram, tessControlShader); }
					if (tessEvalShader) { glAttachShader(shaderProgram, tessEvalShader); }
					if (geometryShader) { glAttachShader(shaderProgram, geometryShader); }
					if (fragmentShader) { glAttachShader(shaderProgram, fragmentShader); }
					glLinkProgram(shaderProgram);
					shaderLinkCheck(shaderProgram);

					//CLEAN UP
					glDeleteShader(vertexShader);
					glDeleteShader(tessControlShader);
					glDeleteShader(tessEvalShader);
					glDeleteShader(geometryShader);
					glDeleteShader(fragmentShader);
				}
			}
		}
		~Shader()
		{
			glDeleteProgram(shaderProgram);
		}
		void shaderCompileCheck(const char* shadername, GLuint shader)
		{
			GLint success = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				constexpr int size = 4096;
				char infolog[size];

				glGetShaderInfoLog(shader, size, nullptr, infolog);
				std::cerr << "SHADER COMPILE ERROR: " << shadername << infolog;
				std::cerr << std::endl;
			}
		}

		void shaderLinkCheck(GLuint shaderProgram)
		{
			GLint success = 0;
			glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
			if (!success)
			{
				constexpr int size = 4096;
				char infolog[size];

				glGetProgramInfoLog(shaderProgram, size, nullptr, infolog);
				std::cerr << "SHADER LINK ERROR: " << infolog;
				std::cerr << std::endl;
			}
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// API
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void use() { glUseProgram(shaderProgram); }

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Uniform Functions
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void setMat4(const char* uniformName, const glm::mat4& matrix4)
		{
			setUniformMatrix4fv(uniformName, 1, GL_FALSE, glm::value_ptr(matrix4));
		}
		void setInt(const char* uniformName, int value)
		{
			setUniform1i(uniformName, value);
		}
		void setFloat(const char* uniformName, float value)
		{
			setUniform1f(uniformName, value);
		}

		void setUniform4f(const char* uniform, float red, float green, float blue, float alpha)
		{
			int uniformLocation = glGetUniformLocation(shaderProgram, uniform);

			//must be using the shader to update uniform value
			glUseProgram(shaderProgram);
			glUniform4f(uniformLocation, red, green, blue, alpha);
		}

		void setUniform4f(const char* uniform, const glm::vec4& values)
		{
			setUniform4f(uniform, values.r, values.g, values.b, values.a);
		}

		void setUniform3f(const char* uniform, float red, float green, float blue)
		{
			int uniformLocation = glGetUniformLocation(shaderProgram, uniform);
			glUseProgram(shaderProgram);
			glUniform3f(uniformLocation, red, green, blue);
		}

		void setUniform3f(const char* uniform, const glm::vec3& vals)
		{
			int uniformLocation = glGetUniformLocation(shaderProgram, uniform);
			glUseProgram(shaderProgram);
			glUniform3f(uniformLocation, vals.r, vals.g, vals.b);
		}

		void setUniform1i(const char* uniform, int newValue)
		{
			int uniformLocation = glGetUniformLocation(shaderProgram, uniform);
			glUseProgram(shaderProgram);
			glUniform1i(uniformLocation, newValue);
		}


		void setUniformMatrix4fv(const char* uniform, int numberMatrices, GLuint transpose, const float* data)
		{
			int uniformLocation = glGetUniformLocation(shaderProgram, uniform);
			glUseProgram(shaderProgram);
			glUniformMatrix4fv(uniformLocation, numberMatrices, transpose, data);
		}

		void setUniform1f(const char* uniformName, float value)
		{
			GLuint uniformLocationInShader = glGetUniformLocation(shaderProgram, uniformName);
			glUseProgram(shaderProgram);
			glUniform1f(uniformLocationInShader, value);
		}


	public:
		ShaderParams initParams;
		GLuint shaderProgram = 0;
	};

}