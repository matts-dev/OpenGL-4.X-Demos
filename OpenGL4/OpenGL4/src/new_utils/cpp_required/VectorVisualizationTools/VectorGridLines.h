#pragma once
#include "../../header_only/share_ptr_typedefs.h"
#include "../curves/CurveSystem.h"
#include "../../header_only/Transform.h"
#include <optional>

namespace ho
{
	struct LineRenderer;
}

namespace nho //not header only
{
	class VisualVector;

	

	class VectorGridLines : public Entity	
	{
		using Parent = Entity;
	public:
		/** Used to help figure out how to render grid lines relative to a vector; one may want to change these to camera basis*/
		struct BasisVectors
		{
			glm::vec3 right_n = glm::vec3(1, 0, 0);
			glm::vec3 up_n = glm::vec3(0, 1, 0);
			glm::vec3 forward_n = glm::vec3(0, 0, 1);
		};
	public:
		void setVector(const sp<VisualVector>& vector);
		void tick(float dt_sec);
		void render(const glm::mat4& projection_view, std::optional<glm::vec3> cameraPos);
		void resetAnim() { animTimeSec = 0.f; }
	protected:
		virtual void postConstruct() override; 
	public:
		float animationDurationSec = 1.5f;
		Curve<200> animCurve = CurveSystem::get().generateSigmoid_medp(100.f);
		glm::vec3 upTickColor{ 0.5f,1.f,0.5f };
		glm::vec3 depthTickColor{ 0.5f,0.5f,1.f };
		glm::vec3 gridBaseColor = glm::vec3(0.5f);
	private:
		float animTimeSec = 0.f;
		float tickHalfLength = 0.5f;
		size_t numTicks = 10;
		BasisVectors basis;
	private:
		sp<VisualVector> trackedVector;
		static sp<ho::LineRenderer> lineRenderer;
	};
}
