#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Transform.h"
#include "Event.h"
#include "share_ptr_typedefs.h"
#include <optional>
#include <memory>

namespace ho
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// This has been converted from my javascript projects which is why naming conventions may be more explicit.
	//	_SymbolName means the symbol is private.
	//  FunctionName_v means the function is a polymorphic (virtual) function.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class SceneNode : 
		public std::enable_shared_from_this<SceneNode>,
		public IEventSubscriber
	{
	public:
		SceneNode()
		{
			//do not bind to parent in ctor because we cannot get shared_this for event.
			_bDirty = false;
			_cleanState();
		}
		virtual ~SceneNode() {}
		//#suggested implement these, be sure that delegate subscription is handled.
		SceneNode(const SceneNode& copy) = delete;
		SceneNode(SceneNode&& move) = delete;
		SceneNode& operator=(const SceneNode& copy) = delete;
		SceneNode& operator=(SceneNode&& move) = delete;

	public:
		///////////////////////////////////////////////////////////////////////////
		// Base functionality
		///////////////////////////////////////////////////////////////////////////
		inline bool isDirty() {return _bDirty; }
		void makeDirty()
		{
			_bDirty = true;
			dirtyEvent.broadcast();
		}
		glm::mat4 getWorldMat()
		{
			if (isDirty())
			{
				_cleanState();
			}

			return _getCachedWorldMat();
		}

		glm::mat4 getLocalModelMat()
		{
			if (isDirty()) 
			{ 
				_cleanState(); 
			}
			return cached_LocalModelMat;
		}

		void requestClean()
		{
			if (isDirty()) { _cleanState(); }
		}

		glm::vec3 getLocalPosition() { return _localXform.position; }
		virtual void setLocalPosition(const glm::vec3& pos)
		{
			_localXform.position = pos;
			makeDirty();
		}

		glm::quat getLocalRotation() { return _localXform.rotQuat; }
		virtual void setLocalRotation(const glm::quat newLocalRotQuat)
		{
			_localXform.rotQuat = newLocalRotQuat;
			makeDirty();
		}

		glm::vec3 getLocalScale() { return _localXform.scale; }
		virtual void setLocalScale(const glm::vec3& newScale )
		{
			_localXform.scale = newScale;
			makeDirty();
		}

		const ho::Transform& getLocalTransform() { return _localXform; }
		virtual void setLocalTransform(const ho::Transform& newTransform)
		{
			_localXform = newTransform;
			makeDirty();
		}

		glm::vec4 getWorldPosition()
		{
			if (isDirty()){_cleanState();}
			return cached_WorldPos;
		}

		glm::mat4 getInverseWorldMat()
		{
			if (isDirty()) { _cleanState(); }
			if (!cached_InverseWorldModel.has_value())
			{
				cached_InverseWorldModel = glm::inverse(cached_WorldModelMat);
			}
			return cached_InverseWorldModel.value();
		}

		void setParent(const sp<SceneNode>& newParentSceneNode)
		{
			if (newParentSceneNode.get() == this)
			{
				return;
			}

			if (_parentNode)
			{
				//remove previous event listener (weak binding prevents circular shared pointers)
				_parentNode->dirtyEvent.removeWeak(sp_this(), &SceneNode::_handleParentDirty);
			}

			makeDirty();
			_parentNode = newParentSceneNode;
			if (_parentNode) //pass null to clear parent.
			{
				_parentNode->dirtyEvent.addWeakObj(sp_this(), &SceneNode::_handleParentDirty);
			}
		}

		const sp<SceneNode>& getParent(){return _parentNode;}

		const sp<SceneNode>& getTopParent()
		{
			//using pointers to sp for quick traversal since we cannot replace references and we don't want to create/destroy smart pointers
			sp<SceneNode>* child = &_parentNode;
			while ((*child)->_parentNode)
			{
				child = &(*child)->_parentNode;
			}
			return *child;
		}
	protected:
		///////////////////////////////////////////////////////////////////////////
		//virtuals
		///////////////////////////////////////////////////////////////////////////
		/** Called when resolving dirty flag. For updating child local caches only; cleaning is not complete when this method is called. */
		virtual void v_ChildUpdateCachedPostClean() {}

		/** A safe method to query things that have a dirty flag immediately after cleaned*/
		virtual void v_CleanComplete() {}

	private:

		/** Doesn't do recursive checks; should only be called if checks have already been done. */
		glm::mat4 _getCachedWorldMat()
		{
			return cached_WorldModelMat;
		}

		/** Updates current node and any dirty parents. */
		bool _cleanState()
		{
			const bool bWasDirty = isDirty();
			if (bWasDirty)
			{
				cached_LocalModelMat = _localXform.getModelMatrix();
				cached_ParentWorldModelMat = _parentNode ? _parentNode->getWorldMat() : glm::mat4(1.f);
				cached_WorldModelMat = cached_ParentWorldModelMat * cached_LocalModelMat;

				cached_InverseWorldModel = std::nullopt; //lazy calculate if needed

				_updateCachesPostClean();
				_bDirty = false;
				v_CleanComplete();
			}

			//return true if recalculation happened
			return bWasDirty;
		}

		void _handleParentDirty()
		{
			makeDirty();
		}

		void _updateCachesPostClean()
		{
			glm::mat4 worldXform = _getCachedWorldMat();
			cached_WorldPos = worldXform * glm::vec4(0.f, 0.f, 0.f, 1.f);

			v_ChildUpdateCachedPostClean();
		}
	public:
		Event<> dirtyEvent;
	private:
		bool _bDirty = false;
		Transform _localXform{};
		glm::vec4 cached_WorldPos{ 0.f };
		glm::mat4 cached_LocalModelMat{ 1.f };
		glm::mat4 cached_ParentWorldModelMat{ 1.f };
		glm::mat4 cached_WorldModelMat{ 1.f };
		std::optional<glm::mat4> cached_InverseWorldModel = std::nullopt; //lazy calculate
		sp<SceneNode> _parentNode = nullptr;
	};
}
