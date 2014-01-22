#include "GL3Buffer.hpp"

#include "Draw2D.hpp"

namespace Engine {
    GL3Buffer::GL3Buffer(EffectParameters params) : _currentEffect(params), _shaderBound(false) {
        this->_init();
    }
    
    GL3Buffer::~GL3Buffer() {
        this->_shutdown();
    }
    
    void GL3Buffer::_init() {
        glGenVertexArrays(1, &this->_vertexArrayPointer);
        glGenBuffers(1, &this->_vertexBufferPointer);
    }
    
    void GL3Buffer::_shutdown() {
        glDeleteBuffers(1, &this->_vertexBufferPointer);
        glDeleteVertexArrays(1, &this->_vertexArrayPointer);
    }
    
    bool GL3Buffer::NeedsUpdate() {
        if (this->_getShader()->NeedsUpdate()) {
            Logger::begin("GL3Buffer", Logger::LogLevel_Verbose) << "GL3Buffer reloaded due to Shader" << Logger::end();
            return true;
        }
        
        if (!glIsBuffer(this->_vertexBufferPointer)) {
            Logger::begin("GL3Buffer", Logger::LogLevel_Verbose) << "GL3Buffer reloaded due to Vertex Buffer" << Logger::end();
            return true;
        }
        
        return false;
    }
    
    bool GL3Buffer::Update() {
        if (!this->NeedsUpdate()) {
            return false;
        }
        
        if (glIsBuffer(this->_vertexBufferPointer)) {
            glDeleteBuffers(1, &this->_vertexBufferPointer);
        }
        
        if (glIsVertexArray(this->_vertexArrayPointer)) {
            glDeleteBuffers(1, &this->_vertexArrayPointer);
        }
        
        this->Invalidate();
        
        this->_init();
        
        this->_getShader()->Update();
        
        return true;
    }
    
    void GL3Buffer::Upload(float *buffer, int count) {
        glBindVertexArray(this->_vertexArrayPointer);
        glBindBuffer(GL_ARRAY_BUFFER, this->_vertexBufferPointer);
        
        glBufferData(GL_ARRAY_BUFFER, count, buffer, GL_STATIC_DRAW);
        
        Draw2D::CheckGLError("GL3Buffer::Upload::PostUploadBufferData");
        
        if (!this->_shaderBound) {
            this->bindShader();
            this->_shaderBound = true;
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    
    void GL3Buffer::Draw(GLenum mode, glm::mat4 model, glm::mat4 view, int vertexCount) {
        glBindVertexArray(this->_vertexArrayPointer);
        glBindBuffer(GL_ARRAY_BUFFER, this->_vertexBufferPointer);
        
        this->_getShader()->Begin();
        
        Draw2D::CheckGLError("GL3Buffer::Draw::PostBindShader");
        
        GLint viewpoint[4];
        
        glGetIntegerv(GL_VIEWPORT, viewpoint);
        
        glm::mat4 proj = glm::ortho(0.0f, (float) viewpoint[2], (float) viewpoint[3], 0.0f, 1.0f, -1.0f);
        
        ShaderSettings settings = this->_currentEffect.GetShaderSettings();
        
        this->_getShader()->UploadUniform(settings.modelMatrixParam, model);
        this->_getShader()->UploadUniform(settings.viewMatrixParam, view);
        this->_getShader()->UploadUniform(settings.projectionMatrixParam, proj);
        
        Draw2D::CheckGLError("GL3Buffer::Draw::PostUploadUniform");
        
        glDrawArrays(mode, 0, vertexCount);
        
        Draw2D::CheckGLError("GL3Buffer::Draw::PostDraw");
        
        this->_getShader()->End();
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    
    Shader* GL3Buffer::_getShader() {
        if (this->_currentShader == NULL) {
            this->_currentShader = this->_currentEffect.CreateShader();
        }
        return this->_currentShader;
    }
    
    void GL3Buffer::bindShader() {
        this->_getShader()->Begin();
        
        Draw2D::CheckGLError("GL3Buffer::Upload::PostBeginShader");
        
        ShaderSettings settings = this->_currentEffect.GetShaderSettings();
        
        this->_getShader()->BindUniform(settings.modelMatrixParam);
        this->_getShader()->BindUniform(settings.viewMatrixParam);
        this->_getShader()->BindUniform(settings.projectionMatrixParam);
        
        Draw2D::CheckGLError("GL3Buffer::Upload::PostBindViewpointSize");
        
        this->_getShader()->BindVertexAttrib(settings.vertexParam, 3, 9, 0);
        this->_getShader()->BindVertexAttrib(settings.colorParam, 4, 9, 3);
        this->_getShader()->BindVertexAttrib(settings.texCoardParam, 2, 9, 7);
        
        Draw2D::CheckGLError("GL3Buffer::Upload::PostBindVertexAttributes");
        
        this->_getShader()->End();
    }
}