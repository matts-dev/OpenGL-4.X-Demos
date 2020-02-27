#pragma once
#pragma once

#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include <iostream>

//_WIN32 is defined in both x64 and x86
#ifdef _WIN32 

#define ERROR_CHECK_GL_RELEASE 0

//Error Check OpenGL (MSCV version)
#if _DEBUG | ERROR_CHECK_GL_RELEASE 
#define ec(func) UtilGL::clearErrorsGL(); /*clear previous errors */\
	func;			/*call function*/ \
	UtilGL::LogGLErrors(#func, __FILE__, __LINE__)
#else
#define ec(func) func
#endif //_DEBUG

#endif //_WIN32 

namespace UtilGL
{

	inline void clearErrorsGL()
	{
		unsigned long error = GL_NO_ERROR;
		do
		{
			error = glGetError();
		} while (error != GL_NO_ERROR);
	}

	inline void LogGLErrors(const char* functionName, const char* file, int line)
	{
		bool bError = false;
		while (GLenum error = glGetError())
		{
			std::cerr << "OpenGLError:" << std::hex << error << " " << functionName << " " << file << " " << line << std::endl;
#ifdef _WIN32 
			__debugbreak();
#endif //_WIN32
		}
	}

}