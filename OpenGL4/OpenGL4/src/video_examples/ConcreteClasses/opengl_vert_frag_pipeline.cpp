#include "../VideoDemoHelperBase.h"
#include "../../new_utils/header_only/shader.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Scene Anim class
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class VertFragPipeline final : public VideoDemoHelperBase
{
public:
	virtual WindowParameters defineWindow();
	virtual void init() override;
	virtual void inputPoll(float dt_sec) override;
	virtual void tick(float dt_sec) override;
	virtual void render_game(float dt_sec) override;
	virtual void render_UI(float dt_sec) override;
private:

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Impl
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VideoDemoHelperBase::WindowParameters VertFragPipeline::defineWindow()
{
	VideoDemoHelperBase::WindowParameters params;
	params.windowResolution.x = 600;
	params.windowResolution.y = 1000;
	return params;
}

void VertFragPipeline::init()
{

}

void VertFragPipeline::inputPoll(float dt_sec)
{
	
}

void VertFragPipeline::tick(float dt_sec)
{

}

void VertFragPipeline::render_game(float dt_sec)
{

}

void VertFragPipeline::render_UI(float dt_sec)
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// main
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace
{
	static void true_main()
	{
		VertFragPipeline anim;
		anim.start();
	}
}

//int main()
//{
//	true_main();
//}