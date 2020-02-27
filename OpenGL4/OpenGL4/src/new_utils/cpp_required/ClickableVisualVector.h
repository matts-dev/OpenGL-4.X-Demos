#pragma once
#include "VisualVector.h"
#include "../header_only/ray_utils.h"
#include "../header_only/shader.h"


namespace nho
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// A debug utility
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
		void render(glm::mat4& projection_view, glm::mat4& model);
	private:
		GLsizei numVerts = 0;
		GLuint vao, vbo;
		sp<ho::Shader> shader;
	};


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// A triangle list that has extra data for making interaction influence a clickable visual vector
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct VectorCollisionTriangleList : public TriangleList
	{
		VectorCollisionTriangleList();
		virtual void transform(const glm::mat4& model) override;
		glm::mat4 cachedPreviousXform;
		bool bRepresentsTip = false;
		struct ClickableVisualVector* owner = nullptr;
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Visual vector that allows user interaction with its ends
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct ClickableVisualVector : public VisualVector
	{
		ClickableVisualVector();
		ClickableVisualVector(const ClickableVisualVector& copy);
		ClickableVisualVector& operator=(const ClickableVisualVector& copy);
		//ClickableVisualVector(ClickableVisualVector&& move) = delete;
		//ClickableVisualVector& operator=(ClickableVisualVector&& move) = delete;
		virtual void onValuesUpdated(const VisualVector::POD& values) override;
	private:
		void sharedInit();
	public:
		VectorCollisionTriangleList startCollision;
		VectorCollisionTriangleList endCollision;
	};
}