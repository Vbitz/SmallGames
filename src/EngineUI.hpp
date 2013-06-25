#pragma once

#include "common.hpp"

#include "main.hpp"

#include "Draw2D.hpp"

namespace Engine {
    namespace EngineUI {
        void Draw();
        void OnKeyPress(int key, int press, bool shift);
        
        void SetActive(bool active);
        
        void ToggleConsole();
        
        bool ConsoleActive();
    }
}