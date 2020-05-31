#include "ProjectionAnimation.h"
#include "VisualVector.h"
#include "../header_only/math_utils.h"
#include "VisualPoint.h"

namespace nho
{

	void VectorProjectionAnimation::tick(float dt_sec)
	{
		animCurTime += dt_sec;

		bool bUseEqualAnim = false;

		switch (animMode)
		{
		case EAnimMode::ALL_POINTS_SYNC:
			{
				float a = glm::clamp<float>(animCurTime / animDurSec, 0, 1.f);
				for (size_t pnt = 0; pnt < lerpPoints.size(); pnt++)
				{
					const sp<VisualPoint>& point = lerpPoints[pnt];
					LerpData& lerp = lerpData[pnt];
					point->setPosition(lerp.start + lerp.startToEnd * animCurve.eval_smooth(a));
				}
			}
			break;
		case EAnimMode::FASTER_TIP:
			{
				//leading animation where tip is ahead of tail
				for (size_t pnt = 0; pnt < lerpPoints.size(); pnt++)
				{
					//fastest possible a tip can complete is just slightly faster than animation dur time
					//alternatively could use a curve to drive faster movement initialy for tip, might look better and be more scalable
					float maxSpeedup = animSpeedupFactor * animDurSec;

					LerpData& lerp = lerpData[pnt];

					float adjustedDuration = animDurSec - maxSpeedup * lerp.percToTip;

					//since different points finish at different times, we need to calculate this for each point using this method.
					float a = glm::clamp<float>(animCurTime / (adjustedDuration), 0, 1.f);
					const sp<VisualPoint>& point = lerpPoints[pnt];
					point->setPosition(lerp.start + lerp.startToEnd * animCurve.eval_smooth(a));
					//point->setPosition(lerp.start + lerp.startToEnd * a); //no curve
				}
			}
			break;
		case EAnimMode::SWEEP_BY_HEIGHT:
			{
				float animPerc = glm::clamp<float>(animCurTime / (animDurSec), 0, 1.f);
				animPerc = animCurve.eval_smooth(animPerc);

				for (size_t pnt = 0; pnt < lerpPoints.size(); pnt++)
				{
					LerpData& lerp = lerpData[pnt];
					const sp<VisualPoint>& point = lerpPoints[pnt];

					float startPerc = 1.f - lerp.percToTip;

					if (animPerc >= startPerc)
					{
						float percRangeForThisPoint = lerp.percToTip; //ie 1.f -startPerc
						float adjustPercForThisPoint = (animPerc - startPerc) / percRangeForThisPoint; //adjusted to [0,1] 
						
						point->setPosition(lerp.start + lerp.startToEnd * adjustPercForThisPoint); //curve is driving entire percent so will still look nice

					}
					else
					{
						point->setPosition(lerp.start);
					}
				}
			}
			break;
		default:
			break;

		}

		if (bLoop && animCurTime > animDurSec)
		{
			animCurTime = 0;
		}
	}

	void VectorProjectionAnimation::projectFromAtoB(const VisualVector& a, const VisualVector& b)
	{
		using namespace glm;

		//this assumes that the vectors share a base point
		const vec3 vecA = a.getVec();
		const vec3 aStart = a.getStart(); //the pointi n space that the visualizatin of the vector comes from

		const vec3 vecB = b.getVec();
		const vec3 bStart = b.getStart();

		const vec3 resultProjection = MathUtils::projectAontoB(vecA, vecB);

		animCurTime = 0;

		const vec3 vecA_n = glm::normalize(vecA);
		const vec3 vecB_n = glm::normalize(vecB);

		const size_t numPnts = lerpPoints.size();
		for (size_t pnt = 0; pnt < numPnts; pnt++)
		{
			float perc = pnt / float(numPnts); //percentage of the vector that this point belongs to
			lerpData[pnt].start = aStart + vecA * perc;
			lerpData[pnt].end = bStart + resultProjection * perc;
			lerpData[pnt].startToEnd = lerpData[pnt].end - lerpData[pnt].start;
			lerpData[pnt].percToTip = perc + 0.00001f;
		}

		bRender = true;
	}

	void VectorProjectionAnimation::render(const glm::mat4& projection_view, std::optional<glm::vec3> cameraPos) const
	{
		if (bRender)
		{
			for (const sp<VisualPoint>& pnt : lerpPoints)
			{
				pnt->render(projection_view, cameraPos);
			}
		}
	}

	void VectorProjectionAnimation::setColor(const glm::vec3& color)
	{
		for (const sp<VisualPoint>& point : lerpPoints)
		{
			point->color = color;
		}
	}

	void VectorProjectionAnimation::postConstruct()
	{
		//generate the points for the animation
		for (size_t pnt = 0; pnt < numPointsInAnimation; pnt++)
		{
			lerpPoints.push_back(new_sp<VisualPoint>());
			lerpData.emplace_back();
		}

	}

}

