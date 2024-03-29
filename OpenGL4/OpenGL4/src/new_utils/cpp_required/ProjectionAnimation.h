#pragma once
#include <vector>
#include <optional>

#include "../header_only/share_ptr_typedefs.h"
#include "curves/CurveSystem.h"

namespace ho
{
	struct LineRenderer;
}

namespace nho
{
	class VisualVector;
	class VisualPoint;


	namespace EAnimMode
	{
		enum type
		{
			ALL_POINTS_SYNC,
			FASTER_TIP,
			SWEEP_BY_HEIGHT
		};
	}

	class VectorProjectionAnimation : public Entity
	{
	public:
		void tick(float dt_sec);
		void projectFromAtoB(const glm::vec3& vecA, const glm::vec3& vecB, const glm::vec3& aStart, const glm::vec3& bStart, bool bResetAnimation = true);
		void projectFromAtoB(const VisualVector& a, const VisualVector& b, bool bResetAnimation = true);
		void render(const glm::mat4& projection_view, std::optional<glm::vec3> cameraPos) const;
		void setColor(const glm::vec3& color);
		void setShouldRender(bool bShouldRender) { bRender = bShouldRender; }
		void setPointScale(float newScale);

		void pauseAnimation();
		void resumeAnimation();
		bool isAnimationDone() const;
		bool shouldRender() const { return bRender; }
	protected:
		virtual void postConstruct() override;
	public: //controls for ui tweaking
		float animDurSec = 1.f;
		float animSpeedupFactor = 0.65f; //factor controlling async behavior in anim
		EAnimMode::type animMode = EAnimMode::SWEEP_BY_HEIGHT;
		bool bLoop = false;
		bool bRenderLineFromTip = false;
	private:
		bool bRender = false;
		bool bPaused = false;
		size_t numPointsInAnimation = 40;
		float animCurTime = 0.f;
		Curve<200> animCurve = CurveSystem::get().generateSigmoid_medp(100.f);
		float pointScale = 1.f;
	private:
		std::vector<sp<VisualPoint>> lerpPoints;
		struct LerpData
		{
			glm::vec3 start;
			glm::vec3 end;
			glm::vec3 startToEnd;
			float percToTip = 1.f; //[0,1] -- value used for special animation trick that makes tip move faster than base
		};
		std::vector<LerpData> lerpData;
		static sp<ho::LineRenderer> lineRenderer;
	};

	

}