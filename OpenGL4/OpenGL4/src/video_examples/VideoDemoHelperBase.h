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
	virtual void init() = 0;
	virtual void inputPoll(float dt_sec) = 0;
	virtual void tick(float dt_sec) = 0;
	virtual void render_game(float dt_sec) = 0;
	virtual void render_UI(float dt_sec) = 0;
};

class VideoDemoHelperBase : public IDemoInterface
{
public:
	VideoDemoHelperBase();
	~VideoDemoHelperBase();
	VideoDemoHelperBase(const VideoDemoHelperBase& copy) = delete;
	VideoDemoHelperBase(VideoDemoHelperBase&& move) = delete;
	VideoDemoHelperBase& operator=(const VideoDemoHelperBase& copy) = delete;
	VideoDemoHelperBase& operator=(VideoDemoHelperBase&&move) = delete;
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


