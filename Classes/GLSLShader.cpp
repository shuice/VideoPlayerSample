/*
 *  GLSLShader.cpp
 *  LIB_GL_SHADER
 *
 *  Created by mbp on 09/01/20.
 *  Copyright 2009 occlusion. All rights reserved.
 *
 */

#include "GLSLShader.h"
#include <assert.h>
#include "FileAccess.h"
//#define DEBUG


GLSLShader::~GLSLShader(){
	if(sh_)glDeleteProgram(sh_);
}

bool GLSLShader::compile(){
	GLuint vertShader;
	GLuint fragShader;
	
	
	GLchar *source;
#if defined(DEBUG) | defined(_DEBUG)
	char log[256];
	GLsizei logLen = 256, logGetLen;
	GLint compiled, linked;
#endif
	
	vertShader = glCreateShader(GL_VERTEX_SHADER);	
	fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	if(this->dataType() == kShaderDataTypeSourceFile){
		FileAccess vertex(this->vshData().c_str(), FILE_READ);
		
		source = vertex.allRead();
		int size = vertex.size();
		glShaderSource(vertShader, 1, (const char **)&source, &size);
		
		FileAccess fragment(this->fshData().c_str(), FILE_READ);
		
		source = fragment.allRead();
		size = fragment.size();
		glShaderSource(fragShader, 1, (const GLchar **)&source, &size);
	}
	else if(this->dataType() == kShaderDataTypeSourceData){
		const GLchar *source = this->vshData().c_str();
		glShaderSource(vertShader, 1, (const GLchar **)&source, NULL);
		source = this->fshData().c_str();
		glShaderSource(fragShader, 1, (const GLchar **)&source, NULL);
	}
	
	glCompileShader(vertShader);
#if defined(DEBUG) | defined(_DEBUG)
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &compiled);
	glGetShaderInfoLog(vertShader, logLen, &logGetLen, log);
	printf(log, NULL);
	if(compiled == GL_FALSE){
		
		fprintf(stderr, "Compile error in VertexShader %s\n", this->vshData().c_str());
		return false;
	}
#endif

	glCompileShader(fragShader);
#if defined(DEBUG) | defined(_DEBUG)
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &compiled);
	glGetShaderInfoLog(fragShader, logLen, &logGetLen, log);
	printf(log, NULL);
	if(compiled == GL_FALSE){
		fprintf(stderr, "Compile error in FragmentShader %s\n", this->fshData().c_str());
		return false;
	}
#endif
	
	sh_ = glCreateProgram();
	glAttachShader(sh_, vertShader);
	glAttachShader(sh_, fragShader);
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);
	
	glLinkProgram(sh_);
#if defined(DEBUG) | defined(_DEBUG)
	glGetProgramiv(sh_, GL_LINK_STATUS, &linked);
	if(linked == GL_FALSE){
		fprintf(stderr, "Link error %s, %s\n", this->vshData().c_str(), this->fshData().c_str());
		return false;
	}
#endif
	
	return true;
}


void GLSLShader::beginShader(){
	glGetIntegerv(GL_CURRENT_PROGRAM, reinterpret_cast<GLint *> (&bufSh_));
	isRendering_ = true;
	glUseProgram(sh_);
	
};
bool GLSLShader::endShader(){
	glUseProgram(bufSh_);
	
	isRendering_ = false;
	return true;
};



void GLSLShader::setTexture(const char *name, int param){
	assert(this->isRendering());
	GLint attr;
	attr = this->findAttrs_(name);
	if(attr == -1){
		attr = glGetUniformLocation(sh_, name);
		this->registAttrs_(name, attr);
	}
	glUniform1i(attr, param);
}


void GLSLShader::setVertexPointer(const char *name, const float *p, int size){
	assert(this->isRendering());
	GLint attr;
	attr = this->findAttrs_(name);
	if(attr == -1){
		attr = glGetAttribLocation(sh_, name);
		this->registAttrs_(name, attr);
	}
	glVertexAttribPointer(attr, size, GL_FLOAT, GL_FALSE, 0, p);
	glEnableVertexAttribArray(attr);
}


inline GLint GLSLShader::findAttrs_(const char *name){
	std::map<std::string, GLint>::const_iterator answer = attrs_.find(name);
	if(answer == attrs_.end()){
		return -1;
	}
	return answer->second;
}
inline void GLSLShader::registAttrs_(const char *name, GLint attr){
	assert(attrs_.find(name) == attrs_.end());
	attrs_.insert(std::pair<std::string, GLint>(name, attr));
}
