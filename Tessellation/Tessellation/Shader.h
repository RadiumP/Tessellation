#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <GL/glew.h>

class Shader
{
public:
	GLuint Program;
	// Constructor generates the shader on the fly
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath, const GLchar* tcsPath, const GLchar* tesPath, const GLchar* gsPath)
	{
		// 1. Retrieve the  source code from filePath
		std::string vertexCode;
		std::string fragmentCode;
		std::string tcsCode;
		std::string tesCode;
		std::string gsCode;

		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		std::ifstream tcShaderFile;
		std::ifstream teShaderFile;
		std::ifstream gShaderFile;

		// ensures ifstream objects can throw exceptions:
		vShaderFile.exceptions(std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::badbit);
		tcShaderFile.exceptions(std::ifstream::badbit);
		teShaderFile.exceptions(std::ifstream::badbit);
		gShaderFile.exceptions(std::ifstream::badbit);


		try
		{
			// Open files
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			tcShaderFile.open(tcsPath);
			teShaderFile.open(tesPath);
			gShaderFile.open(gsPath);


			std::stringstream vShaderStream, fShaderStream, tcShaderStream, teShaderStream, gShaderStream;


			// Read file's buffer contents into streams
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			tcShaderStream << tcShaderFile.rdbuf();
			teShaderStream << teShaderFile.rdbuf();
			gShaderStream << gShaderFile.rdbuf();


			// close file handlers
			vShaderFile.close();
			fShaderFile.close();
			tcShaderFile.close();
			teShaderFile.close();
			gShaderFile.close();



			// Convert stream into string
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
			tcsCode = tcShaderStream.str();
			tesCode = teShaderStream.str();
			gsCode = gShaderStream.str();



		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
		}
		const GLchar* vShaderCode = vertexCode.c_str();
		const GLchar* fShaderCode = fragmentCode.c_str();
		const GLchar* tcShaderCode = tcsCode.c_str();
		const GLchar* teShaderCode = tesCode.c_str();
		const GLchar* gShaderCode = gsCode.c_str();



		// 2. Compile shaders
		GLuint vertex, fragment, tcs, tes, gs;
		GLint success;
		GLchar infoLog[512];



		// Vertex Shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		// Print compile errors if any
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		// tc Shader
		tcs = glCreateShader(GL_TESS_CONTROL_SHADER);
		glShaderSource(tcs, 1, &tcShaderCode, NULL);
		glCompileShader(tcs);
		glGetShaderiv(tcs, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(tcs, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::TESS CONTROL::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		// te Shader
		tes = glCreateShader(GL_TESS_EVALUATION_SHADER);
		glShaderSource(tes, 1, &teShaderCode, NULL);
		glCompileShader(tes);
		glGetShaderiv(tes, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(tes, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::TESS EVALUATION::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		//g Shader
		gs = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(gs, 1, &gShaderCode, NULL);
		glCompileShader(gs);
		glGetShaderiv(gs, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(gs, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::TESS EVALUATION::COMPILATION_FAILED\n" << infoLog << std::endl;
		}


		// Fragment Shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		// Print compile errors if any
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}


		// Shader Program
		this->Program = glCreateProgram();
		glAttachShader(this->Program, vertex);
		glAttachShader(this->Program, tcs);
		glAttachShader(this->Program, tes);
		glAttachShader(this->Program, gs);
		glAttachShader(this->Program, fragment);
		glLinkProgram(this->Program);
		// Print linking errors if any
		glGetProgramiv(this->Program, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(this->Program, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}
		// Delete the shaders as they're linked into our program now and no longer necessery
		glDeleteShader(vertex);
		glDeleteShader(fragment);
		glDeleteShader(tcs);
		glDeleteShader(tes);
		glDeleteShader(gs);

	}
	
	
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath, const GLchar* tcsPath, const GLchar* tesPath)
	{
		// 1. Retrieve the  source code from filePath
		std::string vertexCode;
		std::string fragmentCode;
		std::string tcsCode;
		std::string tesCode;
		

		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		std::ifstream tcShaderFile;
		std::ifstream teShaderFile;
		

		// ensures ifstream objects can throw exceptions:
		vShaderFile.exceptions(std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::badbit);
		tcShaderFile.exceptions(std::ifstream::badbit);
		teShaderFile.exceptions(std::ifstream::badbit);
		


		try
		{
			// Open files
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			tcShaderFile.open(tcsPath);
			teShaderFile.open(tesPath);
			


			std::stringstream vShaderStream, fShaderStream, tcShaderStream, teShaderStream;


			// Read file's buffer contents into streams
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			tcShaderStream << tcShaderFile.rdbuf();
			teShaderStream << teShaderFile.rdbuf();
			


			// close file handlers
			vShaderFile.close();
			fShaderFile.close();
			tcShaderFile.close();
			teShaderFile.close();
			



			// Convert stream into string
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
			tcsCode = tcShaderStream.str();
			tesCode = teShaderStream.str();
			



		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
		}
		const GLchar* vShaderCode = vertexCode.c_str();
		const GLchar* fShaderCode = fragmentCode.c_str();
		const GLchar* tcShaderCode = tcsCode.c_str();
		const GLchar* teShaderCode = tesCode.c_str();
		


		// 2. Compile shaders
		GLuint vertex, fragment, tcs, tes;
		GLint success;
		GLchar infoLog[512];



		// Vertex Shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		// Print compile errors if any
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		// tc Shader
		tcs = glCreateShader(GL_TESS_CONTROL_SHADER);
		glShaderSource(tcs, 1, &tcShaderCode, NULL);
		glCompileShader(tcs);
		glGetShaderiv(tcs, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(tcs, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::TESS CONTROL::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		// te Shader
		tes = glCreateShader(GL_TESS_EVALUATION_SHADER);
		glShaderSource(tes, 1, &teShaderCode, NULL);
		glCompileShader(tes);
		glGetShaderiv(tes, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(tes, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::TESS EVALUATION::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		

		// Fragment Shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		// Print compile errors if any
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}


		// Shader Program
		this->Program = glCreateProgram();
		glAttachShader(this->Program, vertex);
		glAttachShader(this->Program, tcs);
		glAttachShader(this->Program, tes);
		
		glAttachShader(this->Program, fragment);
		glLinkProgram(this->Program);
		// Print linking errors if any
		glGetProgramiv(this->Program, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(this->Program, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}
		// Delete the shaders as they're linked into our program now and no longer necessery
		glDeleteShader(vertex);
		glDeleteShader(fragment);
		glDeleteShader(tcs);
		glDeleteShader(tes);
		

	}


	Shader(const GLchar* vertexPath, const GLchar* fragmentPath)
	{
		// 1. Retrieve the vertex/fragment source code from filePath
		std::string vertexCode;
		std::string fragmentCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		// ensures ifstream objects can throw exceptions:
		vShaderFile.exceptions(std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::badbit);
		try
		{
			// Open files
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			std::stringstream vShaderStream, fShaderStream;
			// Read file's buffer contents into streams
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			// close file handlers
			vShaderFile.close();
			fShaderFile.close();
			// Convert stream into string
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
		}
		const GLchar* vShaderCode = vertexCode.c_str();
		const GLchar * fShaderCode = fragmentCode.c_str();
		// 2. Compile shaders
		GLuint vertex, fragment;
		GLint success;
		GLchar infoLog[512];
		// Vertex Shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		// Print compile errors if any
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		// Fragment Shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		// Print compile errors if any
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		// Shader Program
		this->Program = glCreateProgram();
		glAttachShader(this->Program, vertex);
		glAttachShader(this->Program, fragment);
		glLinkProgram(this->Program);
		// Print linking errors if any
		glGetProgramiv(this->Program, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(this->Program, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}
		// Delete the shaders as they're linked into our program now and no longer necessery
		glDeleteShader(vertex);
		glDeleteShader(fragment);

	}
	// Uses the current shader
	void Use()
	{
		glUseProgram(this->Program);
	}

	void unbind()
	{
		glUseProgram(0);
	}
};

#endif