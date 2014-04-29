/*
   Filename: EffectParameters.cpp
   Purpose:  Parses formatting for shader specfication

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

#include "EffectParameters.hpp"

#include "RenderGL3.hpp"

namespace Engine {
    
    EffectShaderTypes::Type shaderTypeFromString(std::string type) {
        if (type == "glsl_150")
            return EffectShaderTypes::GLSL_150;
        else
            return EffectShaderTypes::Unknown;
    }
    
    std::string resolvePath(std::string basePath, std::string path) {
        if (path.find_first_of('/') == 0)
            return path;
        else
            return basePath + path;
    }
    
    ShaderSpec readShaderSpec(std::string basePath, Json::Value spec) {
        ShaderSpec ret;
        ret.type = shaderTypeFromString(spec["type"].asString());
        ret.vertexShaderPath = resolvePath(basePath, spec["vertex"].asString());
        ret.fragmentShaderPath = resolvePath(basePath, spec["fragment"].asString());
        return ret;
    }
    
    EffectParameters::EffectParameters() {
        
    }
    
    EffectParameters::EffectParameters(std::string basePath, Json::Value root) {
        this->_read(basePath, root);
    }
    
    Shader* EffectParameters::CreateShader() {
        ShaderSpec spec = this->_getBestShaderSpec();
        Shader* shader = new Shader();
        Logger::begin("EffectParameters", Logger::LogLevel_Verbose) << "Using Shader: " << spec.vertexShaderPath << " : " << spec.fragmentShaderPath << Logger::end();
        shader->Init(spec.vertexShaderPath, spec.fragmentShaderPath);
        return shader;
    }
    
    ShaderSettings EffectParameters::GetShaderSettings() {
        return _shaderSettings;
    }
    
    bool EffectParameters::NeedsUpdate() {
        return false;
    }
    
    void EffectParameters::_read(std::string basePath, Json::Value root) {
        Json::Value shadersNode = root["shader"];
        for (int i = 0; i < shadersNode.size(); i++) {
            this->_shaders.push_back(readShaderSpec(basePath, shadersNode[i]));
        }
        
        Json::Value vertexBufferParams = root["vertexBufferParams"];
        Json::Value cameraSettings = root["cameraSettings"];
        
        this->_textureCount = root["textures"].asInt();
        this->_shaderSettings.vertexParam = vertexBufferParams["vertex"].asString();
        this->_shaderSettings.colorParam = vertexBufferParams["color"].asString();
        this->_shaderSettings.texCoardParam = vertexBufferParams["texCoard"].asString();
        
        this->_shaderSettings.modelMatrixParam = cameraSettings["model"].asString();
        this->_shaderSettings.viewMatrixParam = cameraSettings["view"].asString();
        this->_shaderSettings.projectionMatrixParam = cameraSettings["projection"].asString();
        
        this->_userParams = root["userParams"];
    }
    
    ShaderSpec EffectParameters::_getBestShaderSpec() {
        EffectShaderTypes::Type bestType = GetRenderGL()->GetBestEffectShaderType();
        for (auto iter = this->_shaders.begin(); iter != this->_shaders.end(); iter++) {
            if (iter->type == bestType) {
                return *iter;
            }
        }
        return this->_shaders.at(0);
    }
    
    namespace EffectReader {
        std::string getBasePath(std::string path) {
            return path.substr(0, path.find_last_of('/') + 1);
        }
        
        EffectParameters GetEffectFromFile(std::string filename) {
            std::string fileContent = std::string(Filesystem::GetFileContent(filename));
            
            Json::Reader reader;
            Json::Value root;
            
            reader.parse(fileContent, root);
            
            return EffectParameters(getBasePath(filename), root);
        }
    }
}