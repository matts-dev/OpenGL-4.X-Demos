#include "TriangleListDebugger.h"
#include "../header_only/opengl_debug_utils.h"
#include <vector>
#include "../header_only/ray_utils.h"
#include "../header_only/shader.h"
#include "../header_only/share_ptr_typedefs.h"

namespace nho
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
// Triangle List Debugger
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	TriangleListDebugger::TriangleListDebugger(const TriangleList& list)
	{
		ec(glGenVertexArrays(1, &vao));
		ec(glBindVertexArray(vao));

		//convert the triangle list into a series of vertices
		std::vector<glm::vec3> verts;
		for (Triangle tri : list.getLocalTriangles())
		{
			verts.push_back(tri.pntA);
			verts.push_back(tri.pntB);
			verts.push_back(tri.pntC);
		}
		numVerts = verts.size(); //3 elements in a vec3

		ec(glGenBuffers(1, &vbo));
		ec(glBindBuffer(GL_ARRAY_BUFFER, vbo));
		glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(glm::vec3), &verts[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(0));
		glEnableVertexAttribArray(0);

		ho::Shader::ShaderParams shaderInit;
		shaderInit.vertex_src = R"(
						#version 410 core
						layout (location = 0) in vec3 position;				
						uniform mat4 model = mat4(1.0f);
						uniform mat4 projection_view = mat4(1.0f);
						void main(){
							gl_Position = projection_view * model * vec4(position,1.f);
						}
					)";
		shaderInit.fragment_src = R"(
					#version 410 core
					out vec4 fragmentColor;
					uniform vec3 solidColor = vec3(0,1,0);
					void main()
					{
						fragmentColor = vec4(solidColor.rgb, 1.0f);
					}
				)";

		shader = new_sp<ho::Shader>(shaderInit);
	}

	void TriangleListDebugger::render(const glm::mat4& projection_view, const glm::mat4& model)
	{
		/** PSA:: if you get nullptr here, make sure you called Super::init.*/

		shader->use();
		shader->setMat4("projection_view", projection_view);
		shader->setMat4("model", model);

		glBindVertexArray(vao);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		ec(glDrawArrays(GL_TRIANGLES, 0, numVerts));
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBindVertexArray(0);
	}

	TriangleListDebugger::~TriangleListDebugger()
	{
		ec(glDeleteVertexArrays(1, &vao));
		ec(glDeleteBuffers(1, &vbo));
		shader = nullptr;
	}

}
