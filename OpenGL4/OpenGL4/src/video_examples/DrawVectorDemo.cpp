#include "DrawVectorDemo.h"

using namespace nho;

void DrawVectorDemo::gatherInteractableCubeObjects(std::vector<const TriangleList_SNO *>& objectList)
{
	InteractableDemo::gatherInteractableCubeObjects(objectList);

	for (const sp<ClickableVisualVector>& vec : customVectors)
	{
		objectList.push_back(&vec->startCollision->getTriangleList());
		objectList.push_back(&vec->endCollision->getTriangleList());
	}
}

void DrawVectorDemo::tick_drawVector(float dt_sec)
{
	std::optional<glm::vec3> lineStart = getDrawLineStart();
	std::optional<glm::vec3> lineEnd = getDrawLineEnd();

	bool bDrawingLine = lineStart && lineEnd;
	bool bCreatedVectorForLine = bool(newVector);
	if (bDrawingLine)
	{
		if (!bCreatedVectorForLine)
		{
			newVector = new_sp<ClickableVisualVector>();
			newVector->setStart(*lineStart);
			newVector->setEnd(*lineEnd);
			customVectors.push_back(newVector);
		}
		else
		{
			newVector->setEnd(*lineEnd);
		}
	}
	else
	{
		if (newVector)
		{
			if (glm::length(newVector->getVec()) < 0.5f)
			{
				//remove the custom vector as it is small and probably an error
				customVectors.pop_back();
			}

			//clear out previous new vectors
			newVector = nullptr;
		}
	}
}

void DrawVectorDemo::tick_selectVectors(float dt_sec)
{
	if (activeClickTarget)
	{
		if (bSelectNextVector)
		{
			if (const VectorCollisionTriangleList* triList = dynamic_cast<const VectorCollisionTriangleList*>(activeClickTarget))
			{
				if (triList->owner && triList->owner)
				{
					if (const SceneNode_VectorEnd* sn = dynamic_cast<const SceneNode_VectorEnd*>(triList->owner))
					{
						if (sn && sn->owner)
						{
							if (selectionLater)
							{
								selectionLater->color = glm::vec3(1, 1, 1);

							}

							selectionFormer = selectionLater;
							selectionLater = sn->owner->getShared();

							if (selectionFormer)
							{
								selectionFormer->color = glm::vec3(0, 1, 0);
							}
							if (selectionLater)
							{
								selectionLater->color = glm::vec3(1, 0, 0);
							}
							bSelectNextVector = false;
						}
					}
				}
			}
		}
	}
	else
	{
		bSelectNextVector = true;
	}
}

void DrawVectorDemo::tick(float dt_sec)
{
	InteractableDemo::tick(dt_sec);

	//DRAW LOGIC
	tick_drawVector(dt_sec);
	tick_selectVectors(dt_sec);

}

void DrawVectorDemo::init()
{
	InteractableDemo::init();
	bRenderLineGeneration = false;
}

void DrawVectorDemo::render_game(float dt_sec)
{
	InteractableDemo::render_game(dt_sec);
	glm::vec3 camPos = rd->camera->getPosition();
	for (const sp<ClickableVisualVector>& vec : customVectors)
	{
		vec->render(rd->projection_view, camPos);
	}
}
