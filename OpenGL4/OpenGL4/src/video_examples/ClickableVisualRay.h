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
		void setStartPnt(const glm::vec3& newStart);
		void setDirVec(const glm::vec3& newDir);

		glm::vec3 getStartPnt(){return start->getPosition();};
		glm::vec3 getDirVec() { return dir->getVec(); };

		float getT() const { return t; }
		void setT(float val);

		void setUseOffsetTipMesh(bool bOffset) { dir->bUseCenteredMesh = !bOffset;}
		bool isUsingOffsetTip() { return !dir->bUseCenteredMesh; }

		void tick(float dt_sec);

		void getCollision(std::vector<const TriangleList_SNO*>& collisionContainer);
	private:
		sp<ClickableVisualVector> dir;
		sp<ClickableVisualPoint> start;
		float t = 1.0;
	};
}

