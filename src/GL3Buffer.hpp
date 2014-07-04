/*
   Filename: GL3Buffer.hpp
   Purpose:  VBO manager for OpenGL 3.x

   Part of Engine2D

   Copyright (C) 2014 Vbitz

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#pragma once

#include "Shader.hpp"
#include "EffectParameters.hpp"
#include "RenderDriver.hpp"

namespace Engine {
    class Shader;
    class RenderGL3;
    
    /*
     Buffer format
     (x, y, z)      Position
     (r, g, b, a)   Color
     (u, v)         TexCourd
     */
    
#pragma pack(0)
    struct BufferFormat {
        glm::vec3 pos;
        Color4f col;
        glm::vec2 uv;
    };
    
    // .eglb format
    struct GL3BufferDiskFormat {
        unsigned char magic[4] = {'E', 'G', 'L', 'B'};
        
        unsigned int vertexOffset;
        unsigned int vertexCount;
        
        unsigned int indexOffset;
        unsigned int indexCount;
    };
    
    typedef std::vector<BufferFormat> VertexBuffer;
    typedef std::vector<ushort> IndexBuffer;
    
    typedef VertexBuffer& VertexBufferRef;
    typedef IndexBuffer& IndexBufferRef;
    
    ENGINE_CLASS(GL3Buffer);
    
	class GL3Buffer {
	public:
		GL3Buffer(RenderDriverPtr render, EffectParametersPtr params);
		~GL3Buffer();

        void AddVert(glm::vec3 pos);
        void AddVert(glm::vec3 pos, Color4f col);
        void AddVert(glm::vec3 pos, Color4f col, glm::vec2 uv);
        
        void Reset();
        
		void Draw(PolygonMode mode, glm::mat4 model, glm::mat4 view);
        
        bool NeedsUpdate();
        bool Update();
        
        void Invalidate() {
            this->_shaderBound = false;
        }
        
        void Save(std::string filename);
        void Load(std::string filename);

	private:
        void _init();
        void _shutdown();
        ShaderPtr _getShader();
        
        void _begin();
        void _end();
        
		void bindShader();
        
		void _upload();
        
        RenderDriverPtr _getRender() {
            return this->_renderGL;
        }
        
        uint _vertexArrayPointer, _elementBufferPointer, _vertexBufferPointer;
        
        VertexBuffer _vertexBuffer;
        IndexBuffer _indexBuffer;
        
        ushort _vertexCount = 0;
        
        bool _dirty = false;
        
        RenderDriverPtr _renderGL = NULL;
        ShaderPtr _currentShader = NULL;
        EffectParametersPtr _currentEffect;
        
        bool _shaderBound = false;
	};
}