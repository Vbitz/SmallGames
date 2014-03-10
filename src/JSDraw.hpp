/*
   Filename: JSDraw.hpp
   Purpose:  Javascript bindings for Engine::Draw2D

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

#include "Scripting.hpp"

namespace Engine {

	namespace JsDraw {
        
        void InitDraw(v8::Handle<v8::ObjectTemplate> obj);
         
	}

}