#pragma once

#include "common.hpp"
#include "Filesystem.hpp"

#include <vector>
#include <map>

namespace Engine {
    class Shader {
    public:
        Shader();
        Shader(std::string shaderFilename);
        
        ~Shader();
        
        void Init(std::string vertShaderFilename, std::string fragShaderFilename);
        
        void Begin();
        void End();
        
        void BindUniform(std::string token);
        void UploadUniform(std::string token, GLfloat* data, int verts);
        void UploadUniform(std::string token, glm::mat4 matrix);
        void UploadUniform(std::string token, GLfloat x, GLfloat y);
        
        void BindVertexAttrib(std::string token, int attribSize, int totalSize, int stride);
        
        bool IsLoaded() {
            return this->_loaded;
        }
        
    private:
        bool compile(const char* vertSource, const char* fragSource);
        
        inline bool checkProgramPointer() {
            return this->_programPointer != 0 && this->_loaded;
        }
        
        bool _loaded;
        
        std::map<std::string, GLuint> _uniforms;
        std::map<std::string, GLuint> _attribs;
        
        GLuint _programPointer, _vertPointer, _fragPointer;
    };
}