#pragma once

#include<string>

//window library and OpenGL libraries
#include <glad/glad.h> //note: sets up OpenGL headers, so should be before anything that uses those headers (such as GLFW)
#include <GLFW/glfw3.h>

//math libraries
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

struct IDemoInterface
{
	/** Interface function bodies provided in cpp to allow any subclass to call its super without compile error.*/
	virtual void init() = 0;
	virtual void inputPoll(float dt_sec) = 0;
	virtual void tick(float dt_sec) = 0;
	virtual void render_game(float dt_sec) = 0;
	virtual void render_UI(float dt_sec) = 0;
};

class WindowManager : public IDemoInterface	
{
public:
	static bool bEnableDebugUI;
public:
	struct FrameRenderData
	{
		GLFWwindow* window = nullptr;
		struct ICamera* camera = nullptr;
		glm::mat4 view{ 1.f };
		glm::mat4 projection{ 1.f };
		glm::mat4 projection_view{ 1.f };
	};
	static FrameRenderData frameRenderData;
public:
	WindowManager();
	~WindowManager();
	WindowManager(const WindowManager& copy) = delete;
	WindowManager(WindowManager&& move) = delete;
	WindowManager& operator=(const WindowManager& copy) = delete;
	WindowManager& operator=(WindowManager&&move) = delete;
public:
	void start();
protected:
	struct WindowParameters
	{
		glm::ivec2 windowResolution = { 500,800 };
	};
protected:
	virtual WindowParameters defineWindow() = 0;
	virtual void init() = 0;
public:
	virtual void handleFramebufferSizeChanged(int width, int height);
protected:
	GLFWwindow* window;
	std::string windowName;
	float aspect = 1.f;
};


