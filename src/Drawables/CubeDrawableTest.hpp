/*
   Filename: Drawables/CubeDrawableTest.hpp
   Purpose:  Draw a square using th drawable system

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

#include "../RenderDriver.hpp"
#include "../Draw2D.hpp"

namespace Engine {
    namespace Drawables {
        class CubeDrawableTest : public Drawable {
        public:
            CubeDrawableTest(RenderDriver* render) : Drawable(render) { }
            
            void Draw() override;
            
        protected:
            void _init() override;
            
        private:
            Draw2D* _draw;
        };
    }
}