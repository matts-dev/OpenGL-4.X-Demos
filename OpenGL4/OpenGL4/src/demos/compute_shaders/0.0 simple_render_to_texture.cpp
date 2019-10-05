#pragma once
#include<iostream>
#include <string>

#include<glad/glad.h> //include opengl headers, so should be before anything that uses those headers (such as GLFW)
#include<GLFW/glfw3.h>

#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace
{
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// opengl 4.3 style of error checking
	static void APIENTRY openGLErrorCallback_4_3(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
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

	void true_main()
	{
		int width = 800;
		int height = 600;

		glfwInit();
		glfwSetErrorCallback([](int errorCode, const char* errorDescription) {std::cout << "GLFW error : " << errorCode << " : " << errorDescription << std::endl; });
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

		GLFWwindow* window = glfwCreateWindow(width, height, "OpenglContext", nullptr, nullptr);
		if (!window)
		{
			std::cerr << "failed to create window" << std::endl;
			exit(-1);
		}
		glfwMakeContextCurrent(window);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cerr << "failed to initialize glad with processes " << std::endl;
			exit(-1);
		}

		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(&openGLErrorCallback_4_3, nullptr);

		glViewport(0, 0, width, height);
		glfwSetFramebufferSizeCallback(window, [](GLFWwindow*window, int width, int height) {  glViewport(0, 0, width, height); });

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

		const char* vertex_shader_src = R"(
				#version 330 core
				layout (location = 0) in vec3 position;				

				void main(){
					gl_Position = vec4(position, 1);
				}
			)";
		GLuint triVertShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(triVertShader, 1, &vertex_shader_src, nullptr);
		glCompileShader(triVertShader);

		const char* frag_shader_src = R"(
				#version 330 core
				out vec4 fragmentColor;

				void main(){
					fragmentColor = vec4(1.0f, 0.84f, 0.0f, 1.0f);
				}
			)";
		GLuint triFragShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(triFragShader, 1, &frag_shader_src, nullptr);
		glCompileShader(triFragShader);

		auto verifyShaderCompiled = [](const char* shadername, GLuint shader)
		{
			GLint success = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				constexpr int size = 512;
				char infolog[size];

				glGetShaderInfoLog(shader, size, nullptr, infolog);
				std::cerr << "shader failed to compile: " << shadername << infolog << std::endl;
				exit(-1);
			}
		};
		verifyShaderCompiled("vertex shader", triVertShader);
		verifyShaderCompiled("fragment shader", triFragShader);

		GLuint triangleShaderProg = glCreateProgram();
		glAttachShader(triangleShaderProg, triVertShader);
		glAttachShader(triangleShaderProg, triFragShader);
		glLinkProgram(triangleShaderProg);
		auto verifyShaderLink = [](GLuint triShaderProgram)
		{
			GLint success = 0;
			glGetProgramiv(triShaderProgram, GL_LINK_STATUS, &success);
			if (!success)
			{
				constexpr int size = 512;
				char infolog[size];

				glGetProgramInfoLog(triShaderProgram, size, nullptr, infolog);
				std::cerr << "SHADER LINK ERROR: " << infolog << std::endl;
				exit(-1);
			}
		};
		verifyShaderLink(triangleShaderProg);

		glDeleteShader(triVertShader);
		glDeleteShader(triFragShader);

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

		GLuint quadVAO;
		glGenVertexArrays(1, &quadVAO);
		glBindVertexArray(quadVAO);

		GLuint quadVBO;
		glGenBuffers(1, &quadVBO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);

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
		GLuint postProcesShader_vert = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(postProcesShader_vert, 1, &postprocess_vertex_shader_src, nullptr);
		glCompileShader(postProcesShader_vert);
		verifyShaderCompiled("vertex shader", postProcesShader_vert);

		const char* postprocess_frag_shader_src = R"(
				#version 330 core
				out vec4 fragmentColor;

				in vec2 texCoord;

				uniform sampler2D screencapture;
				
				void main(){
					fragmentColor = texture(screencapture, texCoord);
				}
			)";
		GLuint postProcesShader_frag = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(postProcesShader_frag, 1, &postprocess_frag_shader_src, nullptr);
		glCompileShader(postProcesShader_frag);
		verifyShaderCompiled("fragment shader", postProcesShader_frag);

		GLuint postProcessShaderProg = glCreateProgram();
		glAttachShader(postProcessShaderProg, postProcesShader_vert);
		glAttachShader(postProcessShaderProg, postProcesShader_frag);
		glLinkProgram(postProcessShaderProg);

		glDeleteShader(postProcesShader_vert);
		glDeleteShader(postProcesShader_frag);

		int uniformLocation = glGetUniformLocation(postProcessShaderProg, "screencapture");
		glUseProgram(postProcessShaderProg);
		glUniform1i(uniformLocation, 0);

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

		float lastFrameTime = 0;
		while (!glfwWindowShouldClose(window))
		{
			float currentTime = static_cast<float>(glfwGetTime());
			float deltaTime = currentTime - lastFrameTime;
			lastFrameTime = currentTime;

			////////////////////////////////////////////////////////
			// Process Input
			////////////////////////////////////////////////////////
			if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			{
				glfwSetWindowShouldClose(window, true);
			}

			////////////////////////////////////////////////////////
			// Off screen frame buffer
			////////////////////////////////////////////////////////
			glBindFramebuffer(GL_FRAMEBUFFER, framebufferObject);

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(triangleShaderProg);

			glBindVertexArray(vao);
			glDrawArrays(GL_TRIANGLES, 0, 3);

			////////////////////////////////////////////////////////
			// Default on screen frame buffer
			////////////////////////////////////////////////////////
			glUseProgram(postProcessShaderProg);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDisable(GL_DEPTH_TEST); //don't let depth buffer disrupt drawing (although this doesn't seem to affect setup)

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, fbo_Texture_ColorAttachment);

			glBindVertexArray(quadVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			glEnable(GL_DEPTH_TEST); //don't let depth buffer disrupt drawing

			glfwSwapBuffers(window);
			glfwPollEvents();

		}

		glfwTerminate();
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);

		glDeleteVertexArrays(1, &quadVAO);
		glDeleteBuffers(1, &quadVBO);

		//framebuffer related
		glDeleteFramebuffers(1, &framebufferObject);
		glDeleteTextures(1, &fbo_Texture_ColorAttachment);
		glDeleteRenderbuffers(1, &fbo_RenderBufferObject_DepthStencil);
		glDeleteProgram(postProcessShaderProg);
		glDeleteProgram(triangleShaderProg);
	}
}

//int main()
//{
//	true_main();
//}