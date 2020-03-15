#pragma once
#include <memory>
//#include "../header_only/ray_utils.h"
#include "../new_utils/header_only/shader.h"
#include "../new_utils/header_only/SceneNode.h"
#include "../new_utils/header_only/Transform.h"
#include "../new_utils/cpp_required/VisualVector.h"
#include "../new_utils/header_only/ray_utils.h"
#include "../new_utils/header_only/Event.h"
#include "InteractableDemo.h" //TriangleList_SNO
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
	public:
		const VectorCollisionTriangleList& getTriangleList() { return *myTriangleBox; }
	public:
		struct ClickableVisualVector* owner = nullptr;
	private:
		VectorCollisionTriangleList* myTriangleBox;
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Visual vector that allows user interaction with its ends
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct ClickableVisualVector : 
		public VisualVector,
		public std::enable_shared_from_this<ClickableVisualVector>,
		public ho::IEventSubscriber
	{
		ClickableVisualVector();
		ClickableVisualVector(const ClickableVisualVector& copy);
		ClickableVisualVector& operator=(const ClickableVisualVector& copy);
		//ClickableVisualVector(ClickableVisualVector&& move) = delete;
		//ClickableVisualVector& operator=(ClickableVisualVector&& move) = delete;
		virtual void onValuesUpdated(const VisualVector::POD& values) override;
		sp<ClickableVisualVector> getShared() { return sp_this(); }
	protected:
		virtual void postConstruct() override;
	private:
		void sharedInit();

		void handleStartDirty();
		void handleEndDirty();
		void handleStartCollisionUpdated();
		void handleEndCollisionUpdated();
	public:
		sp<SceneNode_VectorEnd> startCollision;
		sp<SceneNode_VectorEnd> endCollision;
		//float tipCollisionCorrectionDistance = 0.5f;
	private:
		bool bUpdatingFromSceneNode = false;
	};
}