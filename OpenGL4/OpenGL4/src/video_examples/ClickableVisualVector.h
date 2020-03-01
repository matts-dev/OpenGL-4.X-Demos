#pragma once
//#include "../header_only/ray_utils.h"
#include "../new_utils/header_only/shader.h"
#include "../new_utils/header_only/SceneNode.h"
#include "../new_utils/header_only/Transform.h"
#include "../new_utils/cpp_required/VisualVector.h"
#include "../new_utils/header_only/ray_utils.h"
#include "./InteractableDemo.h"
namespace nho
{

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// A triangle list that has extra data for making interaction influence a clickable visual vector
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	struct VectorCollisionTriangleList 
		: public TriangleList_SNO
	{
		VectorCollisionTriangleList();
		glm::mat4 cachedPreviousXform;
		bool bRepresentsTip = false;
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// scene wrapper around vector collision triangle list
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class SceneNode_VectorEnd : public SceneNode_TriangleList
	{
	public:
		SceneNode_VectorEnd();
	private:
		VectorCollisionTriangleList* myTriangleBox;
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Visual vector that allows user interaction with its ends
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct ClickableVisualVector : public VisualVector
	{
		ClickableVisualVector();
		ClickableVisualVector(const ClickableVisualVector& copy);
		ClickableVisualVector& operator=(const ClickableVisualVector& copy);
		virtual void onValuesUpdated(const VisualVector::POD& values) override;
	private:
		void sharedInit();
	public:
		sp<SceneNode_VectorEnd> startCollision;
		sp<SceneNode_VectorEnd> endCollision;
	};
}