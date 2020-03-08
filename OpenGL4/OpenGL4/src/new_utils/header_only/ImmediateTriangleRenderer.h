#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "shader.h"
#include "share_ptr_typedefs.h"

#include<glad/glad.h> //include opengl headers, so should be before anything that uses those headers (such as GLFW)
#include<GLFW/glfw3.h>
#include <vector>

namespace ho
{
	struct ImmediateTriangle final
	{
	public:
		ImmediateTriangle(const ImmediateTriangle& copy) = delete;
		ImmediateTriangle(ImmediateTriangle&& move) = delete;
		ImmediateTriangle& operator=(const ImmediateTriangle& copy) = delete;
		ImmediateTriangle& operator=(ImmediateTriangle&& move) = delete;
		ImmediateTriangle()
		{
			using namespace glm;

			const char* const triShader_vs_src = R"(
				#version 330 core

				layout (location = 0) in vec3 basis_vector;

				out vec4 color;

				uniform mat4 shearMat;			
				uniform mat4 projection_view;
				uniform vec3 u_color = vec3(1,1,1);

				void main()
				{
					gl_Position = projection_view * shearMat * vec4(basis_vector, 1);
					color = vec4(u_color,1.f);
				}
			)";
			const char* const triShader_fs_src = R"(
				#version 330 core

				in vec4 color;
				out vec4 fragColor;

				void main()
				{
					fragColor = color;
				}
			)";

			ho::Shader::ShaderParams in;
			in.vertex_src = triShader_vs_src;
			in.fragment_src = triShader_fs_src;

			shader = new_sp<ho::Shader>(in);

			glBindVertexArray(0);
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);

			std::vector<vec3> vertPositions =
			{
				vec3(1,0,0),
				vec3(0,1,0),
				vec3(0,0,1)
			};
			glGenBuffers(1, &vboPositions);
			glBindBuffer(GL_ARRAY_BUFFER, vboPositions);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * vertPositions.size(), vertPositions.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), reinterpret_cast<void*>(0));
			glEnableVertexAttribArray(0);

			//prevent other calls from corrupting this VAO state
			glBindVertexArray(0);
		}

		void renderTriangle(const glm::vec3& pntA, const glm::vec3& pntB, const glm::vec3& pntC, const glm::vec3& color, const glm::mat4& projection_view)
		{
			using namespace glm;

			//use a shear matrix trick to package all this data into a single mat4.
			// shear matrix trick transforms basis vectors into the columns of the provided matrix.
			mat4 shearMatrix = mat4(vec4(pntA, 0), vec4(pntB, 0), vec4(pntC, 0), vec4(0.f, 0.f, 0.f, 1.f));

			shader->use();
			shader->setMat4("shearMat", shearMatrix);
			shader->setMat4("projection_view", projection_view);
			shader->setUniform3f("u_color", color);

			glBindVertexArray(vao);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}

	public:
		sp<ho::Shader> shader = nullptr;
		GLuint vao = 0;
		GLuint vboPositions = 0;
	};

}