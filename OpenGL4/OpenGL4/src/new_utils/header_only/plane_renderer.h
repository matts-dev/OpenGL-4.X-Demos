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
#include "math_utils.h"

namespace ho
{
	struct PlaneRenderer final
	{
	public:
		PlaneRenderer(const PlaneRenderer& copy) = delete;
		PlaneRenderer(PlaneRenderer&& move) = delete;
		PlaneRenderer& operator=(const PlaneRenderer& copy) = delete;
		PlaneRenderer& operator=(PlaneRenderer&& move) = delete;
		PlaneRenderer()
		{
			using namespace glm;

			const char* const plane_vert_src = R"(
					#version 330 core

					layout (location = 0) in vec3 inPositions;

					out vec4 color;

					uniform vec4 uColor = vec4(1,1,1,1);
					uniform mat4 model;			
					uniform mat4 projection_view;

					void main()
					{
						gl_Position = projection_view * model * vec4(inPositions, 1);
						color = uColor;
					}
				)";
			const char* const plane_frag_src = R"(
					#version 330 core

					in vec4 color;
					out vec4 fragColor;

					uniform bool bScreenDoorEffect = false;

					void main()
					{
						fragColor = color;

						if(bScreenDoorEffect)
						{
							ivec4 clamped = ivec4(gl_FragCoord);

							if( clamped.x % 2 != 0 && clamped.y % 2 != 0 )
							{
								discard;
							} 
						}
					}
				)";

			ho::Shader::ShaderParams in;
			in.vertex_src = plane_vert_src;
			in.fragment_src = plane_frag_src;

			shader = new_sp<ho::Shader>(in);

			glBindVertexArray(0);
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);

			//plane in x-plane so normal matches x-axis -- this lets us easily create rotation matrix that moves plane to match requested normal
			std::vector<vec3> vertPositions =
			{
				vec3(0, -0.5f, -0.5f),
				vec3(0, 0.5f, -0.5f),
				vec3(0, -0.5f, 0.5f),

				vec3(0, 0.5f, -0.5f),
				vec3(0, 0.5f, 0.5f),
				vec3(0, -0.5f, 0.5f)
			};
			glGenBuffers(1, &vboPositions);
			glBindBuffer(GL_ARRAY_BUFFER, vboPositions);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * vertPositions.size(), vertPositions.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), reinterpret_cast<void*>(0));
			glEnableVertexAttribArray(0);

			//prevent other calls from corrupting this VAO state
			glBindVertexArray(0);
		}

		void renderPlane(const glm::vec3& centerPnt, const glm::vec3& normal, const glm::vec3& scale, const glm::vec4& color, const glm::mat4& projection_view)
		{
			using namespace glm;

			vec3 tempUp = vec3(0, 1, 0);
			vec3 u = glm::normalize(normal);
			vec3 tempV = MathUtils::vectorsAreSame(normal, tempUp) ? vec3(-1, 0, 0) : tempUp; 
			vec3 w = glm::normalize(glm::cross(u, tempV));
			vec3 v = glm::normalize(glm::cross(w, u));
			mat4 rot{vec4(u,0), vec4(v,0), vec4(w,0),vec4(0,0,0,1)};

			mat4 model{ 1.f };
			model = glm::translate(model, centerPnt);
			model = model * rot;
			model = glm::scale(model, scale);

			shader->use();
			shader->setUniform4f("uColor", color);
			shader->setMat4("model", model);
			shader->setMat4("projection_view", projection_view);
			shader->setInt("bScreenDoorEffect", bScreenDoorEffect);

			glBindVertexArray(vao);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

	public:
		bool bScreenDoorEffect = false;
		sp<ho::Shader> shader = nullptr;
		GLuint vao = 0;
		GLuint vboPositions = 0;
	};

}