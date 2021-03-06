/*
   Filename: Filesystem.hpp
   Purpose:  C++ abstraction over PhysFS

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

#include "Logger.hpp"

#include "vendor/json/json.h"

#include <string>
#include <vector>

namespace Engine {
	namespace Filesystem {
        ENGINE_CLASS(File);
        
        enum class FileMode {
            Read,
            Write,
            Append
        };
        
        class File {
        public:
            
            virtual ~File() {};
            
            virtual void Close() = 0;
            virtual FileMode GetMode() = 0;
            
            static FilePtr Open(std::string path, FileMode mode);
        };
        
		bool IsLoaded();

		void Init(const char* argv0);
		void Destroy();

        bool Mount(std::string path, std::string fsPath);
        bool Mount(std::string path);
        void SetupUserDir(std::string path);
        
		bool FileExists(std::string path);
        
		bool FolderExists(std::string path);
        
        void Mkdir(std::string path);
        
		std::vector<std::string> GetDirectoryContent(std::string path);

		long FileSize(std::string path);
        
        char* GetFileContent(std::string path);
        char* GetFileContent(std::string path, long& fileSize);
        
        std::string GetFileHexDigest(Hash::DigestType type, std::string path);
        Json::Value LoadJsonFile(std::string path);
        
        void WriteFile(std::string path, const char* content, long length);
        void TouchFile(std::string path);
        void DeleteFile(std::string path);

		std::string GetRealPath(std::string path);
		long GetFileModifyTime(std::string path);
        
        bool HasSetUserDir();
	}
}