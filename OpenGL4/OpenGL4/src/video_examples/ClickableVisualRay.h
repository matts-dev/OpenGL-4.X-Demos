#pragma once
#include "../new_utils/header_only/share_ptr_typedefs.h"
#include "ClickableVisualVector.h"
#include "ClickableVisualPoint.h"

namespace nho
{
	class ClickableVisualRay
	{
	public:
		ClickableVisualRay();

		void render(const glm::mat4& projectin_view, std::optional<glm::vec3> cameraPos) const;
	public:
		//sp<nho::ClickableVisualVector> getDir() const { return dir; }
		//sp<nho::ClickableVisualPoint> getStart() const { return start; }
		//void setDir(const sp<nho::ClickableVisualVector>& val) { dir = val; }
		//void setStart(const sp<nho::ClickableVisualPoint>& val) { start = val; }
		void setStartPnt(const glm::vec3& newStart);
		void setDirVec(const glm::vec3& newDir);


		float getT() const { return t; }
		void setT(float val);

		void tick(float dt_sec);

		void getCollision(std::vector<const TriangleList_SNO*>& collisionContainer);
	private:
		sp<ClickableVisualVector> dir;
		sp<ClickableVisualPoint> start;
		float t = 1.0;
	};
}
