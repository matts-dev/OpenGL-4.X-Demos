
#include "../../new_utils/header_only/opengl_debug_utils.h"
#include "../../new_utils/header_only/shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <imgui.1.69.gl/imgui.h>
#include <imgui.1.69.gl/imgui_impl_glfw.h>
#include <imgui.1.69.gl/imgui_impl_opengl3.h>
#include "../../new_utils/header_only/bitmap_font/bitmap_font_base.h"
#include "../../new_utils/header_only/bitmap_font/Montserrat_BitmapFont.h"
#include "../../new_utils/header_only/simple_quaternion_camera.h"
#include "../../new_utils/header_only/line_renderer.h"
#include "../../video_examples/WindowManager.h"
#include "../../new_utils/cpp_required/VisualVector.h"
#include "../../new_utils/header_only/share_ptr_typedefs.h"

using nho::VisualVector;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base Application Declaration - Geometric Algebra
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class GrassmannAlgebraDemo final : public WindowManager
{
public:
	static sp<ho::Montserrat_BMF> font;
	static up<QuaternionCamera> quatCam;
	static up<ho::LineRenderer> lineRenderer;
public:
	virtual WindowParameters defineWindow() override;
	virtual void init() override;
	virtual void inputPoll(float dt_sec) override;
	virtual void tick(float dt_sec) override;
	virtual void render_game(float dt_sec) override;
	virtual void render_UI(float dt_sec) override;
private:
	//up<ho::Shader> coneTipShader = nullptr;
	sp<ho::TextBlockSceneNode> TestText3D = nullptr;
	float fovY_rad = glm::radians<float>(45.f);
	float near = 0.1f;
	float far = 100.f;

	//size_t slideIdx = 0;
	std::vector<nho::VisualVector> visualVectors;
};

/** Shared statics implementation for linker to link against*/
/*static*/ sp<ho::Montserrat_BMF> GrassmannAlgebraDemo::font = nullptr;
/*static*/ up<QuaternionCamera> GrassmannAlgebraDemo::quatCam = nullptr;
/*static*/ up<ho::LineRenderer> GrassmannAlgebraDemo::lineRenderer = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base Application Implementation - Geometric Algebra
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

WindowManager::WindowParameters GrassmannAlgebraDemo::defineWindow()
{
	WindowManager::WindowParameters params;
	params.windowResolution.x = 1400;
	params.windowResolution.y = 900;

	return params;
}

void GrassmannAlgebraDemo::init()
{
	lineRenderer = new_up<ho::LineRenderer>();
	quatCam = new_up<QuaternionCamera>();
	quatCam->pos = glm::vec3(0, 0, 5.f);

	frameRenderData.window = window;

	font = new_sp<ho::Montserrat_BMF>("./assets/textures/font/Montserrat_ss_alpha_1024x1024_wb.png");
	TestText3D = new_sp<ho::TextBlockSceneNode>(font, "Testing 3 2 1.");

	ec(glEnable(GL_DEPTH_TEST));
}

void GrassmannAlgebraDemo::inputPoll(float dt_sec)
{

}

void GrassmannAlgebraDemo::tick(float dt_sec)
{
	quatCam->tick(dt_sec, window);


}

void GrassmannAlgebraDemo::render_game(float dt_sec)
{
	using namespace glm;

	frameRenderData.view = quatCam->getView();
	frameRenderData.projection = glm::perspective(quatCam->getFOVy_rad(), aspect, near, far);
	frameRenderData.projection_view = frameRenderData.projection * frameRenderData.view;
	frameRenderData.camera = quatCam.get();

	ec(glClearColor(0.f, 0.f, 0.f, 0.f));
	ec(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));

	vec3 camPos = quatCam->getPosition();

	for (nho::VisualVector& vector : visualVectors)
	{
		vector.render(frameRenderData.projection_view, camPos);
	}

	//if (slideIdx < slides.size())
	//{
	//	slides[slideIdx]->render_game(dt_sec);
	//}
}

void GrassmannAlgebraDemo::render_UI(float dt_sec)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	/////////////////////////////////////////////////////////////////////////////////////
	// Overall demo ui
	/////////////////////////////////////////////////////////////////////////////////////
	{
		static bool bFirstDraw = true;
		if (bFirstDraw)
		{
			bFirstDraw = false;
			ImGui::SetNextWindowPos(ImVec2{ 0, (frameRenderData.fbHeight * 0.9f) });
		}

		ImGuiWindowFlags flags = 0;
		ImGui::Begin("Slides", nullptr, flags);
		{
			if (ImGui::Button("Previous"))
			{
				//--slideIdx;
				//slideIdx = glm::clamp<size_t>(slideIdx, 0, slides.size() - 1);
			}
			ImGui::SameLine();
			if (ImGui::Button("Next"))
			{
				//++slideIdx;
				//slideIdx = glm::clamp<size_t>(slideIdx, 0, slides.size() - 1);
			}
		}
		ImGui::End();
	}

	//if (slideIdx < slides.size())
	//{
	//	slides[slideIdx]->render_UI(dt_sec);
	//}

	ImGui::EndFrame(); //added this to test decoupling rendering from frame; seems okay
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


static void true_main()
{
	GrassmannAlgebraDemo demo;
	demo.start();
}

int main()
{
	true_main();
}