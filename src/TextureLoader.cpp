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

#include "TextureLoader.hpp"

#include "RenderGL3.hpp"

namespace Engine {
    Texture::Texture() {
        
    }
    
    Texture::Texture(RenderGL3* render, GLuint textureID) : _render(render) {
        Logger::begin("Texture", Logger::LogLevel_Verbose) << "Creating Texture: " << textureID << Logger::end();
        this->_setTextureID(textureID);
    }
    
    Texture::~Texture() {
        this->Invalidate();
    }
    
    void Texture::Invalidate() {
        if (this->IsValid())
            glDeleteTextures(1, &this->_textureID);
        this->_textureID = UINT_MAX;
    }
    
    void Texture::Save(std::string filename) {
        this->Begin();
        
        BYTE* pixels = new BYTE[4 * this->_width * this->_height];
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
        
        this->_render->CheckGLError("Texture::Begin::PreBind");
        glBindTexture(GL_TEXTURE_2D, this->_textureID);
        this->_render->CheckGLError("Texture::Begin::PostBind");
    }
    
    void Texture::End() {
        glBindTexture(GL_TEXTURE_2D, 0);
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
        
        Texture* TextureFromFileBuffer(unsigned char *buffer, long bufferLength) {
            FIMEMORY* mem = FreeImage_OpenMemory((BYTE*) buffer, (unsigned int)bufferLength);
            
            FIBITMAP *lImg = FreeImage_LoadFromMemory(FreeImage_GetFileTypeFromMemory(mem), mem);
            
            FIBITMAP *img = 0;
            
            if (FreeImage_GetBPP(lImg) == 24) {
                Logger::begin("JSDraw", Logger::LogLevel_Warning) << "Converting image to 32bit" << Logger::end();
                img = FreeImage_ConvertTo32Bits(lImg);
            } else {
                img = lImg;
            }
            
            int width = FreeImage_GetWidth(img);
            int height = FreeImage_GetHeight(img);
            
            FreeImage_FlipVertical(img); // a fix for freeimage
            
            unsigned char* pixel = (unsigned char*)FreeImage_GetBits(img);
            unsigned char* texture = new unsigned char[4 * width * height];
            
            int pos = 0;
            
            for (int x = 0; x < width; x++) {
                for (int y = 0; y < height; y++) {
                    texture[pos + 0] = pixel[pos + 2];
                    texture[pos + 1] = pixel[pos + 1];
                    texture[pos + 2] = pixel[pos + 0];
                    texture[pos + 3] = pixel[pos + 3];
                    pos += 4;
                }
            }
            
            FreeImage_CloseMemory(mem);
            
            Texture* tex = TextureFromBuffer(texture, width, height);
            
            delete [] texture; // that should fix some anoying memory leaks
            
            return tex;
        }
        
        Texture* TextureFromBuffer(unsigned char *texture, int width, int heigth) {
            return TextureFromBuffer(UINT_MAX, texture, width, heigth);
        }
        
        Texture* TextureFromBuffer(GLuint textureID, unsigned char *texture, int width, int height) {
            RenderGL3* render = GetRenderGL();
            
            GLuint text = 0;
            
            render->CheckGLError("Pre Image Load");
            
            glGenTextures(1, &text);
            
            glBindTexture(GL_TEXTURE_2D, text);
            
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                            GL_NEAREST_MIPMAP_NEAREST );
            
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                            GL_NEAREST );
            
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture);
            
            glGenerateMipmap(GL_TEXTURE_2D);
            
            glBindTexture(GL_TEXTURE_2D, 0);
            
            render->CheckGLError("Post Image Load");
            
            return new Texture(render, text);
        }
        
        Texture* TextureFromBuffer(float* texture, int width, int height) {
            return TextureFromBuffer(UINT_MAX, texture, width, height);
        }
        
        Texture* TextureFromBuffer(GLuint textureID, float* texture, int width, int height) {
            RenderGL3* render = GetRenderGL();
            
            GLuint text = 0;
            
            render->CheckGLError("Pre Image Load");
            
            glGenTextures(1, &text);
            
            glBindTexture(GL_TEXTURE_2D, text);
            
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                            GL_NEAREST_MIPMAP_NEAREST );
            
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                            GL_NEAREST );
            
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, texture);
            
            glGenerateMipmap(GL_TEXTURE_2D);
            
            glBindTexture(GL_TEXTURE_2D, 0);
            
            render->CheckGLError("Post Image Load");
            
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
            SaveBufferToFile(filename, pixels, width, height, false);
        }
        
        bool SaveBufferToFile(std::string filename, unsigned char* pixels, int width, int height, bool topDown) {
            FIMEMORY* mem = FreeImage_OpenMemory();
            
            FIBITMAP* image = FreeImage_ConvertFromRawBits(pixels, width, height, ((((32 * width) + 31) / 32) * 4), 32, 0xFF000000, 0x00FF0000, 0x0000FF00, topDown);
            
            FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilename(filename.c_str());
            
            if (format == FIF_UNKNOWN) {
                format = FIF_BMP;
            }
            
            FreeImage_SaveToMemory(format, image, mem);
            
            long fileSize = FreeImage_TellMemory(mem);
            
            FreeImage_SeekMemory(mem, 0, SEEK_SET);
            
            char* buffer = (char*)malloc(sizeof(char) * fileSize);
            
            FreeImage_ReadMemory(buffer, sizeof(char), (unsigned int) fileSize, mem);
            
            Filesystem::WriteFile(filename, buffer, fileSize);
            
            FreeImage_CloseMemory(mem);
        }

    }
}