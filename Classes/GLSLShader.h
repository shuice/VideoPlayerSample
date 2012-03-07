/*
 *  GLSLShader.h
 *  LIB_GL_SHADER
 *
 *  Created by mbp on 09/01/20.
 *  Copyright 2009 occlusion. All rights reserved.
 *
 */


#if (defined(__APPLE__) && defined(__MACH__))
#include "TargetConditionals.h"

#if TARGET_OS_IPHONE
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#else
#include <OpenGL/OpenGL.h>
#endif

#endif

#include <string>
#include <map>
#include <vector>

typedef enum{
	kShaderDataTypeSourceFile,
	kShaderDataTypeSourceData,
} ShaderDataType;

class GLSLShader{
	bool isRendering_;
	
	ShaderDataType dataType_;
	std::string vshData_;
	std::string fshData_;
	
	GLuint sh_;
	GLuint bufSh_;
	std::map<std::string, GLint> attrs_;
	std::vector<GLint> vertexAttrs_;

	void init_();
	
	inline GLint findAttrs_(const char *name);
	inline void registAttrs_(const char *name, GLint attr);
	std::string &vshData(){	return vshData_;	}
	std::string &fshData(){	return fshData_;	}
	ShaderDataType dataType(){	return dataType_;	}
	
public:
	GLSLShader(const char *vshader, const char *pshader, ShaderDataType type = kShaderDataTypeSourceFile):
	vshData_(vshader), dataType_(type), isRendering_(false){
		if(pshader)	fshData_ = pshader;
	};
	GLSLShader(char *vshader, char *pshader, ShaderDataType type = kShaderDataTypeSourceData):
	vshData_(vshader), dataType_(type), isRendering_(false){
		if(pshader)	fshData_ = pshader;
	};
	virtual ~GLSLShader();
	bool isValid(){	return (sh_ != 0);	};
	bool compile();
	void beginShader();
	bool endShader();

	void setGlobalMatrix(const char *name){};
	
	inline void setTexture(GLint attr, int param){glUniform1i(attr, param);	}
	void setTexture(const char *name, int param);
	
	inline GLint vectorAttribute(const char *name){return glGetAttribLocation(sh_, name);};

	void setVertexPointer(const char *name, const float *p, int size);
	inline void setVertexPointer(GLint attr, const float *p, int size){
		glVertexAttribPointer(attr, size, GL_FLOAT, GL_FALSE, 0, p);
		glEnableVertexAttribArray(attr);
	}
	inline void setVertexPointer(GLint attr, const double *p, int size){
#if (defined(__APPLE__) && defined(__MACH__) && defined(TARGET_OS_IPHONE))
//		assert(false);
#else
		glVertexAttribPointer(attr, size, GL_DOUBLE, GL_FALSE, 0, p);
		glEnableVertexAttribArray(attr);
#endif
	}
	inline void setVertexPointer(GLint attr, const unsigned char *p, int size){
		glVertexAttribPointer(attr, size, GL_UNSIGNED_BYTE, GL_FALSE, 0, p);
		glEnableVertexAttribArray(attr);
	}
	
	inline GLint parameterAttribute(const char *name){return glGetUniformLocation(sh_, name);}

	inline void setParameter(GLint attr, float p1){glUniform1f(attr, p1);}
	inline void setParameterMatrix44(GLint attr, const float *p){glUniformMatrix4fv(attr, 1, false, p);}

	inline bool isRendering(){return isRendering_;};
	
};