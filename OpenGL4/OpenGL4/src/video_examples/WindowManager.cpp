#include "WindowManager.h"

#include<iostream>

#include <imgui.1.69.gl/imgui.h>
#include <imgui.1.69.gl/imgui_impl_glfw.h>
#include <imgui.1.69.gl/imgui_impl_opengl3.h>
#include <unordered_map>

namespace
{
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// opengl 4.3 style of error checking
	void APIENTRY openGLErrorCallback_4_3(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	{
		auto typeToText = [](GLenum type)
		{
			switch (type)
			{
				case GL_DEBUG_TYPE_ERROR: return "GL_DEBUG_TYPE_ERROR";
				case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR";
				case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR";
				case GL_DEBUG_TYPE_PORTABILITY: return "GL_DEBUG_TYPE_PORTABILITY";
				case GL_DEBUG_TYPE_PERFORMANCE: return "GL_DEBUG_TYPE_PERFORMANCE";
				default: return "unknown type string";
			}
		};

		auto severityToText = [](GLenum severity)
		{
			switch (severity)
			{
				case GL_DEBUG_SEVERITY_HIGH: return "GL_DEBUG_SEVERITY_HIGH";
				case GL_DEBUG_SEVERITY_MEDIUM: return "GL_DEBUG_SEVERITY_MEDIUM";
				case GL_DEBUG_SEVERITY_LOW: return "GL_DEBUG_SEVERITY_LOW";
				case GL_DEBUG_SEVERITY_NOTIFICATION: return "GL_DEBUG_SEVERITY_NOTIFICATION";
				default: return "unknown";
			}
		};

		if (type == GL_DEBUG_TYPE_ERROR || type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR || type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR || type == GL_DEBUG_TYPE_PORTABILITY || type == GL_DEBUG_TYPE_PERFORMANCE)
		{
			std::cout << "OpenGL Error : " << message << std::endl;
			std::cout << "source : " << source << "\ttype : " << typeToText(type) << "\tid : " << id << "\tseverity : " << severityToText(severity) << std::endl;
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
// IDemoInterface
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** Providing method implementations for abstract methods so that subclasses can always call super and compile. This helps
future proof the code as one can always just call super (without failing to compile) and if the super is added later
thing will just work rather than creating esoteric bugs by introducing places where code isn't calling super. */
void IDemoInterface::init(){}
void IDemoInterface::inputPoll(float dt_sec){}
void IDemoInterface::tick(float dt_sec){}
void IDemoInterface::render_game(float dt_sec){}
void IDemoInterface::render_UI(float dt_sec) {}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
// Window Manager
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static std::unordered_map<GLFWwindow*, WindowManager*> windowToOwnerMap;
bool WindowManager::bEnableDebugUI = true;

static void static_handleFramebufferSizeChanged(GLFWwindow*window, int width, int height)
{
	if (WindowManager* windowManager = windowToOwnerMap[window])
	{
		windowManager->handleFramebufferSizeChanged(width,height);
	}
}

/*static*/ WindowManager::FrameRenderData WindowManager::frameRenderData;

WindowManager::WindowManager()
{
	//need to call virtuals of subclass to get window configuration information, so we do not do window registration here
}

WindowManager::~WindowManager()
{
	windowToOwnerMap.erase(window);
}

void WindowManager::start()
{
	WindowParameters windowParams = defineWindow();

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	window = glfwCreateWindow(windowParams.windowResolution.x, windowParams.windowResolution.y, windowName.c_str(), nullptr, nullptr);
	if (!window)
	{
		std::cerr << "failed to create window" << std::endl;
		return;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "failed to initialize glad with processes " << std::endl;
		return;
	}

	windowToOwnerMap[window] = this;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Set up ui
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO imguiIO = ImGui::GetIO();
	ImGui_ImplOpenGL3_Init("#version 330 core"); //example puts this after window binding, but seems okay
	ImGui_ImplGlfw_InitForOpenGL(window, /*install callbacks*/true); //false will require manually calling callbacks in our own handlers //#TODO make virtuals to override for this and do not install default imgui handlers
	ImGui::GetStyle().ScaleAllSizes(0.5f); //probably shouldn't do this here

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Set up window
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	glViewport(0, 0, windowParams.windowResolution.x, windowParams.windowResolution.y);
	glfwSetFramebufferSizeCallback(window, [](GLFWwindow*window, int width, int height) {  glViewport(0, 0, width, height); });

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(&openGLErrorCallback_4_3, nullptr);

	init();

	while (!glfwWindowShouldClose(window))
	{
		static float lastFrameTime = -1.f; //simulate some time on the first frame where time is 0.
		float thisFrameTime = float(glfwGetTime());
		float dt_sec = thisFrameTime - lastFrameTime;
		lastFrameTime = thisFrameTime;

		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		aspect = float(width) / height;

		glfwPollEvents();

		inputPoll(dt_sec);
		tick(dt_sec);
		render_game(dt_sec);
		render_UI(dt_sec);

		glfwSwapBuffers(window);
	}
}

void WindowManager::handleFramebufferSizeChanged(int width, int height)
{
	glViewport(0, 0, width, height);
}
