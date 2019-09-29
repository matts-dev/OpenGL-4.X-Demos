#pragma once
#include<iostream>
#include <string>

#include<glad/glad.h> //include opengl headers, so should be before anything that uses those headers (such as GLFW)
#include<GLFW/glfw3.h>

#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../../old_utils/CameraFPS.h"
#include "../../old_utils/nu_utils.h"
#include "../../old_utils/Shader.h"

namespace
{
	CameraFPS camera(45.0f, -90.f, 0.f);

	float lastFrameTime = 0.f;
	float deltaTime = 0.0f;

	float pitch = 0.f;
	float yaw = -90.f;

	float FOV = 45.0f;

	const char* vertex_shader_src = R"(
				#version 330 core
				layout (location = 0) in vec3 position;				

				void main(){
					gl_Position = vec4(position, 1);
				}
			)";
	const char* frag_shader_src = R"(
				#version 330 core
				out vec4 fragmentColor;

				void main(){
					fragmentColor = vec4(1.0f, 0.84f, 0.0f, 1.0f);
				}
			)";

	const char* postprocess_vertex_shader_src = R"(
				#version 330 core
				layout (location = 0) in vec2 position;				
				layout (location = 1) in vec2 inTexCoord;
				
				out vec2 texCoord;

				void main(){
					texCoord = inTexCoord;
					gl_Position = vec4(position.x, position.y, 0.0f, 1.0f);
				}
			)";
	const char* postprocess_frag_shader_src = R"(
				#version 330 core
				out vec4 fragmentColor;

				in vec2 texCoord;

				uniform sampler2D screencapture;
				
				void main(){
					fragmentColor = texture(screencapture, texCoord);
				}
			)";

	void processInput(GLFWwindow* window)
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, true);
		}
		camera.handleInput(window, deltaTime);
	}

	void true_main()
	{
		camera.setPosition(0.0f, 0.0f, 3.0f);
		int width = 800;
		int height = 600;

		GLFWwindow* window = init_window_4_6(width, height);

		glViewport(0, 0, width, height);
		glfwSetFramebufferSizeCallback(window, [](GLFWwindow*window, int width, int height) {  glViewport(0, 0, width, height); });
		camera.exclusiveGLFWCallbackRegister(window);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		////////////////////////////////////////////////////////
		// draw data
		////////////////////////////////////////////////////////
		float vertices[] = {
			-0.5f, 0.5f, 0.0f,
			0.0f, -0.5f, 0.0f,
			0.5f, 0.5f, 0.0f
		};

		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertShader, 1, &vertex_shader_src, nullptr);
		glCompileShader(vertShader);
		verifyShaderCompiled("vertex shader", vertShader);

		GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragShader, 1, &frag_shader_src, nullptr);
		glCompileShader(fragShader);
		verifyShaderCompiled("fragment shader", fragShader);

		GLuint shaderProg = glCreateProgram();
		glAttachShader(shaderProg, vertShader);
		glAttachShader(shaderProg, fragShader);
		glLinkProgram(shaderProg);
		verifyShaderLink(shaderProg);

		glDeleteShader(vertShader);
		glDeleteShader(fragShader);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(0));
		glEnableVertexAttribArray(0);


		////////////////////////////////////////////////////////
		// Post processing quad (ie render to texture)
		////////////////////////////////////////////////////////
		float quadVerts[] = {
			//x    y         s    t
			-1.0, -1.0,		0.0, 0.0,
			-1.0, 1.0,		0.0, 1.0,
			1.0, -1.0,		1.0, 0.0,

			1.0, -1.0,      1.0, 0.0,
			-1.0, 1.0,      0.0, 1.0,
			1.0, 1.0,		1.0, 1.0
		};

		GLuint postVAO;
		glGenVertexArrays(1, &postVAO);
		glBindVertexArray(postVAO);

		GLuint postVBO;
		glGenBuffers(1, &postVBO);
		glBindBuffer(GL_ARRAY_BUFFER, postVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);



		Shader postprocessShader(postprocess_vertex_shader_src, postprocess_frag_shader_src, false);
		postprocessShader.use();
		postprocessShader.setUniform1i("screencapture", 0);

		Shader drawShader(vertex_shader_src, frag_shader_src, false);
		drawShader.use();

		glEnable(GL_DEPTH_TEST);

		// -------------------------------------- FRAME BUFFER SETUP-------------------------------------------------------------
		GLuint framebufferObject;
		glGenFramebuffers(1, &framebufferObject);

		glBindFramebuffer(GL_FRAMEBUFFER, framebufferObject); //bind both read/write to the target framebuffer
		///glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufferObject); //this allows us to bind to the specific framebuffer to write to (but not read) {eg glReadPixels}
		///glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferObject); //we can also specifically bind to a frame buffer to read from (I guess for depth/stencil testing) {eg rendering, clearing, otherWriteOperations}

		GLuint fbo_Texture_ColorAttachment;
		glGenTextures(1, &fbo_Texture_ColorAttachment);
		glBindTexture(GL_TEXTURE_2D, fbo_Texture_ColorAttachment);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_INT, nullptr); //pass null to texture data, we only want to allocate memory -- not fill it.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_Texture_ColorAttachment, 0); //last argument is mipmap level

		GLuint fbo_RenderBufferObject_DepthStencil;
		glGenRenderbuffers(1, &fbo_RenderBufferObject_DepthStencil);
		glBindRenderbuffer(GL_RENDERBUFFER, fbo_RenderBufferObject_DepthStencil);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fbo_RenderBufferObject_DepthStencil);
		glBindRenderbuffer(GL_RENDERBUFFER, 0); //unbind the render buffer

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cerr << "error on setup of framebuffer" << std::endl;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0); //bind to default frame buffer
		// -------------------------------------- END FRAME BUFFER -------------------------------------------------------------

		while (!glfwWindowShouldClose(window))
		{
			float currentTime = static_cast<float>(glfwGetTime());
			deltaTime = currentTime - lastFrameTime;
			lastFrameTime = currentTime;

			processInput(window);

			//bind off-screen framebuffer to render 
			glBindFramebuffer(GL_FRAMEBUFFER, framebufferObject);

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			drawShader.use();

			glBindVertexArray(vao);
			glDrawArrays(GL_TRIANGLES, 0, 3);

			//enable default framebuffer object
			postprocessShader.use();
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDisable(GL_DEPTH_TEST); //don't let depth buffer disrupt drawing (although this doesn't seem to affect setup)

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, fbo_Texture_ColorAttachment);

			glBindVertexArray(postVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			glEnable(GL_DEPTH_TEST); //don't let depth buffer disrupt drawing

			glfwSwapBuffers(window);
			glfwPollEvents();

		}

		glfwTerminate();
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);

		glDeleteVertexArrays(1, &postVAO);
		glDeleteBuffers(1, &postVBO);

		//framebuffer related
		glDeleteFramebuffers(1, &framebufferObject);
		glDeleteTextures(1, &fbo_Texture_ColorAttachment);
		glDeleteRenderbuffers(1, &fbo_RenderBufferObject_DepthStencil);
	}
}

//int main()
//{
//	true_main();
//}