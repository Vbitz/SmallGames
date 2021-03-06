/*
   Filename: TextureLoader.cpp
   Purpose:  Texture loader

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

#define GLEW_STATIC
#include "vendor/GL/glew.h"

#include "TextureLoader.hpp"

#include "Application.hpp"
#include "Profiler.hpp"

#include "vendor/soil/SOIL.h"

namespace Engine {
    Texture::Texture() {
        
    }
    
    Texture::Texture(RenderDriver* render, GLuint textureID) : _render(render), _uuid(Platform::GenerateUUID()) {
        Logger::begin("Texture", Logger::LogLevel_Verbose) << "Creating Texture: " << textureID << " [" << Platform::StringifyUUID(this->_uuid) << "]" << Logger::end();
        this->_setTextureID(textureID);
    }
    
    Texture::~Texture() {
        this->Invalidate();
    }
    
    void Texture::Invalidate() {
        Logger::begin("Texture", Logger::LogLevel_Verbose) << "Invalidating Texture: " << this->_textureID << " [" << Platform::StringifyUUID(this->_uuid) << "]" << Logger::end();
        if (this->IsValid())
            glDeleteTextures(1, &this->_textureID);
        this->_textureID = std::numeric_limits<unsigned int>::max();
    }
    
    void Texture::Save(std::string filename) {
        this->Begin();
        
        unsigned char* pixels = new unsigned char[4 * this->_width * this->_height];
        glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
        
        this->End();
        
        ImageWriter::SaveBufferToFile(filename, pixels, this->_width, this->_height);
    }
    
    bool Texture::IsValid() {
        return glIsTexture(this->_textureID);
    }
    
    void Texture::Begin() {
        if (!this->IsValid()) {
            throw "Invalid Texture";
        }
        
        ENGINE_PROFILER_SCOPE;
        
        this->_render->CheckError("Texture::Begin::PreBind");
        glBindTexture(GL_TEXTURE_2D, this->_textureID);
        
        this->_render->CheckError("Texture::Begin::PostBind");
    }
    
    void Texture::End() {
        ENGINE_PROFILER_SCOPE;
        
		this->_render->EnableDefaultTexture();
    }
    
    int Texture::GetWidth() {
        return this->_width;
    }
    
    int Texture::GetHeight() {
        return this->_height;
    }
    
    void Texture::_setTextureID(GLuint textureID) {
        this->_setTextureID(textureID, false);
    }
    
    void Texture::_setTextureID(GLuint textureID, bool deleteOld) {
        if (deleteOld && this->IsValid())
            glDeleteTextures(1, &this->_textureID);
        this->_textureID = textureID;
        
        this->Begin();
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
                                 GL_TEXTURE_WIDTH, &this->_width);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
                                 GL_TEXTURE_HEIGHT, &this->_height);
        this->End();
    }
    
    namespace ImageReader {
        
        TexturePtr TextureFromFileBuffer(unsigned char *buffer, long bufferLength) {
            int width, height, chaneals;
            unsigned char* pixel = SOIL_load_image_from_memory(buffer, bufferLength, &width, &height, &chaneals, SOIL_LOAD_RGBA);
            
            TexturePtr tex = TextureFromBuffer(pixel, width, height);
            
            SOIL_free_image_data(pixel);
            
            return tex;
        }
        
        TexturePtr TextureFromBuffer(unsigned char *texture, int width, int heigth) {
            return TextureFromBuffer(std::numeric_limits<unsigned int>::max(), texture, width, heigth);
        }
        
        TexturePtr TextureFromBuffer(GLuint textureID, unsigned char *texture, int width, int height) {
            RenderDriverPtr render = GetAppSingilton()->GetRender();
            
            GLuint text = 0;
            
            render->CheckError("Pre Image Load");
            
            glGenTextures(1, &text);

			if (text == 0) {
				Logger::begin("TextureLoader", Logger::LogLevel_Error) << "glGenTextures Failed" << Logger::end();
			}
            
            glBindTexture(GL_TEXTURE_2D, text);
            
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                            GL_NEAREST_MIPMAP_NEAREST );
            
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                            GL_NEAREST );
            
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture);
            
            glGenerateMipmap(GL_TEXTURE_2D);
            
            glBindTexture(GL_TEXTURE_2D, 0);
            
            render->CheckError("Post Image Load");
            
            return new Texture(render, text);
        }
        
        TexturePtr TextureFromBuffer(float* texture, int width, int height) {
            return TextureFromBuffer(std::numeric_limits<unsigned int>::max(), texture, width, height);
        }
        
        TexturePtr TextureFromBuffer(GLuint textureID, float* texture, int width, int height) {
            RenderDriverPtr render = GetAppSingilton()->GetRender();
            
            GLuint text = 0;
            
            render->CheckError("Pre Image Load");
            
            glGenTextures(1, &text);
            
            glBindTexture(GL_TEXTURE_2D, text);
            
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                            GL_NEAREST_MIPMAP_NEAREST );
            
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                            GL_NEAREST );
            
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, texture);
            
            glGenerateMipmap(GL_TEXTURE_2D);
            
            glBindTexture(GL_TEXTURE_2D, 0);
            
            render->CheckError("Post Image Load");
            
            return new Texture(render, text);
        }
        
        ResourceManager::ImageResource* TextureFromFile(std::string filename) {
            // Create a Source
            ResourceManager::Load(filename);
            
            return new ResourceManager::ImageResource(filename);
        }
    }
    
    namespace ImageWriter {
        bool SaveBufferToFile(std::string filename, unsigned char* pixels, int width, int height) {
            return SaveBufferToFile(filename, pixels, width, height, false);
        }
        
        bool SaveBufferToFile(std::string filename, unsigned char* pixels, int width, int height, bool topDown) {
            
            Filesystem::TouchFile(filename);
            
            const char* filepath = Filesystem::GetRealPath(filename).c_str();
            
            SOIL_save_image(filepath, SOIL_SAVE_TYPE_BMP, width, height, 4, pixels);
            
            return true;
        }

    }
}