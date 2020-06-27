#include "VectorGridLines.h"
#include "../../header_only/line_renderer.h"
#include "../VisualVector.h"

namespace nho
{
	/*static*/ sp<ho::LineRenderer> VectorGridLines::lineRenderer;

	void VectorGridLines::postConstruct()
	{
		Parent::postConstruct();

		if (!lineRenderer)
		{
			lineRenderer = new_sp<ho::LineRenderer>();
		}
	}

	void VectorGridLines::setVector(const sp<VisualVector>& vector)
	{
		trackedVector = vector;
	}

	void VectorGridLines::tick(float dt_sec)
	{
		animTimeSec += dt_sec;

		if (trackedVector) 
		{
			//TODO remove if not needed? this can be generated in render
		}
	}

	void VectorGridLines::render(const glm::mat4& projection_view, std::optional<glm::vec3> cameraPos)
	{
		using namespace glm;

		bool bPassInvariants = numTicks >= 2;

		if (trackedVector && bPassInvariants)
		{
			 const glm::vec3 start_p = trackedVector->getStart(); 
			 const glm::vec3 dir_v = trackedVector->getVec();
			 const glm::vec3 dir_n = glm::normalize(dir_v);

			//create basis for vector, assuming vector is always pointing in right direction
			 glm::vec3 temp = basis.up_n;
			 if (abs(glm::dot(basis.up_n, dir_n)) > 0.98f) {temp = -basis.right_n;}

			//I believe flipping will be tolerable so it is okay if this ends up generating a left-handed basis system.
			const glm::vec3 vecRight = dir_n;
			const glm::vec3 vecUp = normalize(cross(temp, vecRight));
			const glm::vec3 vecDepth = normalize(cross(vecRight, vecUp));

			float percComplete = glm::clamp(animTimeSec / animationDurationSec, 0.f, 1.f);
			float alpha = animCurve.eval_smooth(percComplete);

			lineRenderer->renderLine(start_p, dir_v*percComplete*float(numTicks), gridBaseColor, projection_view);
			lineRenderer->renderLine(start_p, -dir_v *percComplete*float(numTicks), gridBaseColor, projection_view);

			//glm::clamp<size_t>(size_t(alpha * float(numTicks)) + 1, 0, numTicks);
			for (size_t gridTick = 1; gridTick < numTicks; ++gridTick)
			{
				float gridLocPerc = glm::clamp<float>(gridTick / float(numTicks - 1), 0.f, 1.f); //numTicks will be 0 if only rendering 1 tick;

				if (alpha > gridLocPerc)
				{
					//this isn't going to look quite right, but may look okay. all grid lines will finish growing at same time.
					float gridPercValidRange = 1.0f - gridLocPerc;
					float gridGrowthPerc = glm::clamp((alpha - gridLocPerc) / gridPercValidRange, 0.f, 1.f);	//[0,1]

					auto renderTick = [&](const glm::vec3& renderDir_v) {
						glm::vec3 gridLoc = start_p + (renderDir_v * float(gridTick));
					
						//render up ticks
						glm::vec3 tickStart = gridLoc;
						glm::vec3 tickEnd = gridLoc + gridGrowthPerc * tickHalfLength*vecUp;
						lineRenderer->renderLine(tickStart, tickEnd, upTickColor, projection_view);

						tickEnd = gridLoc - gridGrowthPerc * tickHalfLength*vecUp;
						lineRenderer->renderLine(tickStart, tickEnd, upTickColor, projection_view);

						//render depth ticks
						tickEnd = gridLoc + gridGrowthPerc * tickHalfLength* vecDepth;
						lineRenderer->renderLine(tickStart, tickEnd, depthTickColor, projection_view);

						tickEnd = gridLoc - gridGrowthPerc * tickHalfLength* vecDepth;
						lineRenderer->renderLine(tickStart, tickEnd, depthTickColor, projection_view);
					};
					renderTick(dir_v);
					renderTick(-dir_v); //mirror
				}
				else
				{
					break;//not going to render anything yet as animation has not progressed to the point to render any further ticks.
				}
			}
		}
	}

}

