#include "ClickableVisualRay.h"

namespace nho
{
	ClickableVisualRay::ClickableVisualRay()
	{
		dir = new_sp<ClickableVisualVector>();
		start = new_sp<ClickableVisualPoint>();
	}


	void ClickableVisualRay::render(const glm::mat4& projection_view, std::optional<glm::vec3> cameraPos) const
	{
		start->render(projection_view, cameraPos);
		dir->render(projection_view, cameraPos);
	}

	void ClickableVisualRay::setStartPnt(const glm::vec3& newStart)
	{
		start->setPosition(newStart);
		dir->setStart(newStart);
	}

	void ClickableVisualRay::setDirVec(const glm::vec3& newDir)
	{
		dir->setVector(newDir * t);
	}

	void ClickableVisualRay::setT(float val)
	{
		t = val;
		//making assumption that these vectors will always have a normalized dir direction, that way we don't need to cache off a separate dir that is not normalized.
		glm::vec3 dir_n = glm::normalize(dir->getVec());
		glm::vec3 dirScaledByT = dir_n * t;
		dir->setVector(dirScaledByT);
	}

	void ClickableVisualRay::tick(float dt_sec)
	{
		//always keep the start position aligned with the dir vector
		dir->setStart(start->getPosition());

		glm::vec3 rawDir = dir->getVec();
		float dirLen = glm::length(rawDir);
		t = dirLen; //assume we're always constructing a ray of unit length, in that case we can use the length of the visual vector to be t
	}

	void ClickableVisualRay::getCollision(std::vector<const TriangleList_SNO*>& collisionContainer)
	{
		collisionContainer.push_back(&dir->endCollision->getTriangleList());
		collisionContainer.push_back(&start->pointCollision->getTriangleList());
	}

}

