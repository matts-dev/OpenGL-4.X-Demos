#pragma once
#include <vector>

#include "ClickableVisualVector.h"
#include "InteractableDemo.h"

#include "../new_utils/header_only/share_ptr_typedefs.h"

class DrawVectorDemo : public InteractableDemo
{
public:
	virtual void init()override;
	virtual void render_game(float dt_sec) override;
	virtual void tick(float dt_sec) override;
protected:
	virtual void gatherInteractableCubeObjects(std::vector<const TriangleList_SNO *>& objectList) override;
private:
	void tick_drawVector(float dt_sec);
	void tick_selectVectors(float dt_sec);
protected:
	std::vector<sp<nho::ClickableVisualVector>> customVectors;
	sp<nho::ClickableVisualVector> newVector = nullptr;
private:
	bool bSelectNextVector = true;
	sp<nho::ClickableVisualVector> selectionFormer = nullptr;
	sp<nho::ClickableVisualVector> selectionLater = nullptr;
};
