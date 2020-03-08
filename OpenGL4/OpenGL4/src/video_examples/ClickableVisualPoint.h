#pragma once

#include <memory>

#include "../new_utils/header_only/shader.h"
#include "../new_utils/header_only/SceneNode.h"
#include "../new_utils/header_only/Transform.h"
#include "../new_utils/cpp_required/VisualVector.h"
#include "../new_utils/header_only/ray_utils.h"
#include "../new_utils/header_only/Event.h"
#include "../new_utils/cpp_required/VisualPoint.h"

#include "ClickableVisualVector.h"

/** This is a quick copy paste of the visual vector class -- refer to that for questions/comments about implementation */
namespace nho
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// A triangle list that has extra data for making interaction influence a clickable visual vector
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	struct PointCollisionTriangleList
		: public TriangleList_SNO
	{
		PointCollisionTriangleList();
		glm::mat4 cachedPreviousXform;
		bool bRepresentsTip = false;
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// scene wrapper around vector collision triangle list
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class SceneNode_Point : public SceneNode_TriangleList
	{
	public:
		SceneNode_Point();
	public:
		const PointCollisionTriangleList& getTriangleList() { return *myTriangleBox; }
	public:
		struct ClickableVisualPoint* owner = nullptr;
	private:
		PointCollisionTriangleList* myTriangleBox;
	};


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Visual vector that allows user interaction with its ends
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct ClickableVisualPoint :
		public nho::VisualPoint,
		public std::enable_shared_from_this<ClickableVisualPoint>,
		public ho::IEventSubscriber
	{
		ClickableVisualPoint();
		//ClickableVisualPoint(const ClickableVisualPoint& copy);
		//ClickableVisualPoint& operator=(const ClickableVisualPoint& copy);
		virtual void onValuesUpdated(const nho::VisualPoint::POD& values) override;
		sp<ClickableVisualPoint> getShared() { return sp_this(); }
	protected:
		virtual void postConstruct() override;
	private:
		void sharedInit();
		void handleStartDirty();
		void handlePointCollisionUpdated();
	public:
		sp<nho::SceneNode_Point> pointCollision;
	private:
		bool bUpdatingFromSceneNode = false;
	};
};