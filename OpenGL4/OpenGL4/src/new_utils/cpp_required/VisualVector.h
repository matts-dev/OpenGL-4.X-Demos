#pragma once
#include "../header_only/Transform.h"
#include "../header_only/share_ptr_typedefs.h"

//forward declarations
namespace ho
{
	struct Shader;
	struct LineRenderer;
}
namespace StaticMesh
{
	class Model;
}

namespace nho
{

	class VisualVector : public Entity
	{
	public:
		VisualVector();
		~VisualVector();

		/** Converting to entity so move/copy semantics cacn probably be disregarded*/
		VisualVector(const VisualVector& copy);
		VisualVector& operator=(const VisualVector& copy);
		VisualVector(VisualVector&& move);
		VisualVector& operator=(VisualVector&& move);
	public:
		void render(glm::mat4 projectin_view) const;
		void setVector(glm::vec3 newVec);
		void setStart(glm::vec3 newStart);
		void setEnd(glm::vec3 newEnd);

		glm::vec3 getVec() { return pod.dir; }
		glm::vec3 getStart() { return pod.startPos; }
	private:
		void updateCache();
	public:
		glm::vec3 color{ 1.f };
		struct POD
		{
			glm::vec3 startPos{ 0.f };
			glm::vec3 dir_n{ 1.f,0.f,0.f };
			glm::vec3 dir{ 1.f,0.f,0.f};
			glm::mat4 cachedTipXform{ 1.f };
		};
	private:
		POD pod;
		virtual void onValuesUpdated(const struct POD& values) {};
	private:
		static int numInstances;
		static sp<StaticMesh::Model> tipMesh;
		static sp<ho::Shader> tipShader;
		static sp<ho::LineRenderer> lineRenderer;
	};


}