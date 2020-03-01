#pragma once
#include "../header_only/Transform.h"
#include "../header_only/shader.h"
#include "../header_only/share_ptr_typedefs.h"
#include "../header_only/ray_utils.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// A debug utility
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace nho
{
	class TriangleListDebugger
	{
	public:
		TriangleListDebugger(const TriangleList& list);
		~TriangleListDebugger();

		TriangleListDebugger(const TriangleListDebugger& copy) = delete;
		TriangleListDebugger(TriangleListDebugger&& move) = delete;
		TriangleListDebugger& operator=(const TriangleListDebugger& copy) = delete;
		TriangleListDebugger& operator=(TriangleListDebugger&& move) = delete;
	public:
		void render(const glm::mat4& projection_view, const glm::mat4& model);
	private:
		GLsizei numVerts = 0;
		GLuint vao, vbo;
		sp<ho::Shader> shader;
	};
}
