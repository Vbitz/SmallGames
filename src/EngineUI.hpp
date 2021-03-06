/*
   Filename: EngineUI.hpp
   Purpose:  Draws the EngineUI and console on top of rendering

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

#include "Application.hpp"
#include "Draw2D.hpp"

namespace Engine {
    ENGINE_CLASS(Application);
    
    ENGINE_CLASS(EngineUI);
    
    class EngineUI {
    public:
        enum class CurrentView {
            Console,
            Settings,
            Profiler
        };
        
        EngineUI(ApplicationPtr app);
        
        void Draw();
        void OnKeyPress(int key, int press, bool shift);
        
        void SetActive(bool active);
        
        void ToggleConsole();
        
        void ClearConsole();
        
        bool ConsoleActive();
    private:
        struct ToastInfomation {
            std::string domain, info;
            double timeOnScreen = 0.0;
        };
        
        int _renderProfileZone(RenderDriverPtr renderGL, glm::vec2 windowSize, Json::Value& data, int x, int xIndent, int baseY, int y);
        void _pushToast(std::string domain, std::string info);
        std::string _getHeapUsageString();
        double _getHeapUsage();
        
        ApplicationPtr _app;
        Draw2DPtr _draw;
        
        CurrentView _currentView = CurrentView::Console;
        
        std::stringstream _ss;
        
        std::stringstream _currentConsoleInput;
        
        bool _active = true;
        bool _showConsole = false;
        
        static const int timingResolution = 1000;
        
        double _lastDrawTimes[timingResolution];
        double _lastFrameTimes[timingResolution];
        double _lastHeapUsages[timingResolution];
        int _currentLastDrawTimePos = 0;
        double _profilerDrawTimeScale = 10000;
        
        std::vector<std::string> _commandHistory;
        size_t _currentHistoryLine = 0;
        
        std::vector<ToastInfomation> _toasts;
        
        Json::Value _cachedProfilerDetails;
        double _currentProfilerScroll = 0;
        int _profilerX = 0;
        
        static EventMagic _profilerHook(Json::Value args, void* userPointer);
        static EventMagic _captureLastDrawTimes(Json::Value args, void* userPointer);
        static EventMagic _createToast(Json::Value args, void* userPointer);
    };
}