/*
   Filename: Window_glfw.cpp
   Purpose:  Window creation and management

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

#include "Window.hpp"

#define GLEW_STATIC
#include "vendor/GL/glew.h"

#include <GLFW/glfw3.h>

#include "RenderGL3.hpp"

#include "Events.hpp"
#include "Logger.hpp"

#include <cstdio>

#define GLM_FORCE_RADIANS
#include "vendor/glm/gtc/matrix_transform.hpp"

#include "Platform.hpp"

#ifdef _PLATFORM_OSX
#include <unistd.h>
#endif

namespace Engine {
    // from https://github.com/glfw/glfw/blob/master/tests/events.c
    static std::string glfwGetKeyName(int key)
    {
        if (key < 256) {
			char* string = new char[2];
			string[0] = (char) key;
			string[1] = '\0';
            return std::string(string);
        }
        switch (key)
        {
                // Function keys
            case GLFW_KEY_ESCAPE:       return "ESCAPE";
            case GLFW_KEY_F1:           return "F1";
            case GLFW_KEY_F2:           return "F2";
            case GLFW_KEY_F3:           return "F3";
            case GLFW_KEY_F4:           return "F4";
            case GLFW_KEY_F5:           return "F5";
            case GLFW_KEY_F6:           return "F6";
            case GLFW_KEY_F7:           return "F7";
            case GLFW_KEY_F8:           return "F8";
            case GLFW_KEY_F9:           return "F9";
            case GLFW_KEY_F10:          return "F10";
            case GLFW_KEY_F11:          return "F11";
            case GLFW_KEY_F12:          return "F12";
            case GLFW_KEY_F13:          return "F13";
            case GLFW_KEY_F14:          return "F14";
            case GLFW_KEY_F15:          return "F15";
            case GLFW_KEY_F16:          return "F16";
            case GLFW_KEY_F17:          return "F17";
            case GLFW_KEY_F18:          return "F18";
            case GLFW_KEY_F19:          return "F19";
            case GLFW_KEY_F20:          return "F20";
            case GLFW_KEY_F21:          return "F21";
            case GLFW_KEY_F22:          return "F22";
            case GLFW_KEY_F23:          return "F23";
            case GLFW_KEY_F24:          return "F24";
            case GLFW_KEY_F25:          return "F25";
            case GLFW_KEY_UP:           return "UP";
            case GLFW_KEY_DOWN:         return "DOWN";
            case GLFW_KEY_LEFT:         return "LEFT";
            case GLFW_KEY_RIGHT:        return "RIGHT";
            case GLFW_KEY_LEFT_SHIFT:   return "LEFT SHIFT";
            case GLFW_KEY_RIGHT_SHIFT:  return "RIGHT SHIFT";
            case GLFW_KEY_LEFT_CONTROL: return "LEFT CONTROL";
            case GLFW_KEY_RIGHT_CONTROL: return "RIGHT CONTROL";
            case GLFW_KEY_LEFT_ALT:     return "LEFT ALT";
            case GLFW_KEY_RIGHT_ALT:    return "RIGHT ALT";
            case GLFW_KEY_TAB:          return "TAB";
            case GLFW_KEY_ENTER:        return "ENTER";
            case GLFW_KEY_BACKSPACE:    return "BACKSPACE";
            case GLFW_KEY_INSERT:       return "INSERT";
            case GLFW_KEY_DELETE:       return "DELETE";
            case GLFW_KEY_PAGE_UP:      return "PAGE UP";
            case GLFW_KEY_PAGE_DOWN:    return "PAGE DOWN";
            case GLFW_KEY_HOME:         return "HOME";
            case GLFW_KEY_END:          return "END";
            case GLFW_KEY_KP_0:         return "KEYPAD 0";
            case GLFW_KEY_KP_1:         return "KEYPAD 1";
            case GLFW_KEY_KP_2:         return "KEYPAD 2";
            case GLFW_KEY_KP_3:         return "KEYPAD 3";
            case GLFW_KEY_KP_4:         return "KEYPAD 4";
            case GLFW_KEY_KP_5:         return "KEYPAD 5";
            case GLFW_KEY_KP_6:         return "KEYPAD 6";
            case GLFW_KEY_KP_7:         return "KEYPAD 7";
            case GLFW_KEY_KP_8:         return "KEYPAD 8";
            case GLFW_KEY_KP_9:         return "KEYPAD 9";
            case GLFW_KEY_KP_DIVIDE:    return "KEYPAD DIVIDE";
            case GLFW_KEY_KP_MULTIPLY:  return "KEYPAD MULTPLY";
            case GLFW_KEY_KP_SUBTRACT:  return "KEYPAD SUBTRACT";
            case GLFW_KEY_KP_ADD:       return "KEYPAD ADD";
            case GLFW_KEY_KP_DECIMAL:   return "KEYPAD DECIMAL";
            case GLFW_KEY_KP_EQUAL:     return "KEYPAD EQUAL";
            case GLFW_KEY_KP_ENTER:     return "KEYPAD ENTER";
            case GLFW_KEY_PRINT_SCREEN: return "PRINT SCREEN";
            case GLFW_KEY_NUM_LOCK:     return "NUM LOCK";
            case GLFW_KEY_CAPS_LOCK:    return "CAPS LOCK";
            case GLFW_KEY_SCROLL_LOCK:  return "SCROLL LOCK";
            case GLFW_KEY_PAUSE:        return "PAUSE";
            case GLFW_KEY_LEFT_SUPER:   return "LEFT SUPER";
            case GLFW_KEY_RIGHT_SUPER:  return "RIGHT SUPER";
            case GLFW_KEY_MENU:         return "MENU";
                
            default:                    assert(false); return NULL;
        }
    }
    
    static std::string glfwGetKeyStateName(int state) {
        switch (state) {
            case GLFW_PRESS:    return "press";
            case GLFW_REPEAT:   return "repeat";
            case GLFW_RELEASE:  return "release";
            default:            return "unknown";
        }
    }
    
    int glfwMouseFromMouseButton(MouseButton b) {
        switch (b) {
            case MouseButton_Left:      return GLFW_MOUSE_BUTTON_LEFT;
            case MouseButton_Right:     return GLFW_MOUSE_BUTTON_RIGHT;
            case MouseButton_Middle:    return GLFW_MOUSE_BUTTON_MIDDLE;
        }
    }
    
    std::string glfwGetMouseButtonName(int e) {
        switch (e) {
            case GLFW_MOUSE_BUTTON_1:   return "mouseLeft";
            case GLFW_MOUSE_BUTTON_2:   return "mouseRight";
            case GLFW_MOUSE_BUTTON_3:   return "mouseMiddle";
            case GLFW_MOUSE_BUTTON_4:   return "mouse4";
            case GLFW_MOUSE_BUTTON_5:   return "mouse5";
            case GLFW_MOUSE_BUTTON_6:   return "mouse6";
            case GLFW_MOUSE_BUTTON_7:   return "mouse7";
            case GLFW_MOUSE_BUTTON_8:   return "mouse8";
            default:                    return "unknown";
        }
    }
    
    KeyStatus glfwKeyToKeyStatus(int s) {
        switch (s) {
            case GLFW_PRESS:            return Key_Press;
            case GLFW_RELEASE:          return Key_Release;
            case GLFW_REPEAT:           return Key_Repeat;
            default:                    assert(false); return Key_Release;
        }
    }
    
    class Window_glfw : public Window {
    public:
        
        Window_glfw(GraphicsVersion v) : Window(v) { this->_init(); }
        
        void Show() override {
            if (this->_window == NULL) {
                this->_create();
            }
            assert(this->_window != NULL); // The program will crash out if window creation fails
            this->_visible = true;
            glfwShowWindow(this->_window);
        }
        
        void Hide() override {
            assert(this->_window != NULL);
            this->_visible = false;
            glfwHideWindow(this->_window);
        }
        
        void Begin() override {
            assert(this->_window != NULL);
            glfwMakeContextCurrent(this->_window);
        }
        
        void End() override {
            glfwMakeContextCurrent(NULL);
        }
        
        void Present() override {
            assert(this->_window != NULL);
            glfwSwapBuffers(this->_window);
            glfwPollEvents();
        }
        
        virtual void Reset() override {
            this->_destroy();
            this->_create();
        }
        
        glm::vec2 GetCursorPos() override {
            assert(this->_window != NULL);
            double xPos, yPos;
            glfwGetCursorPos(this->_window, &xPos, &yPos);
            return glm::vec2(xPos, yPos);
        }
        
        bool GetMouseButtonPressed(MouseButton b) override {
            assert(this->_window != NULL);
            return glfwGetMouseButton(this->_window, glfwMouseFromMouseButton(b)) == GLFW_PRESS;
        }
        
        KeyStatus GetKeyStatus(int key) override {
            return glfwKeyToKeyStatus(glfwGetKey(this->_window, key));
        }
        
        glm::vec2 GetWindowSize() override {
            return this->_size;
        }
        
        bool GetVisible() override {
            return this->_visible;
        }
        
        bool IsFocused() override {
            assert(this->_window != NULL);
            return glfwGetWindowAttrib(this->_window, GLFW_FOCUSED);
        }
        
        bool ShouldClose() override {
			assert(this->_window != NULL);
			return glfwWindowShouldClose(this->_window);
        }
        
        void WaitEvents() override {
            glfwWaitEvents();
        }
        
        void SetWindowSize(glm::vec2 s) override {
            this->_size = s;
            if (this->_window != NULL) {
                glfwSetWindowSize(this->_window, this->_size.x, this->_size.y);
            }
        }
        
        std::string GetTitle() override {
            return this->_title;
        }
        
        void SetTitle(std::string title) override {
            this->_title = title;
            if (this->_window != NULL) {
                glfwSetWindowTitle(this->_window, this->_title.c_str());
            }
        }
        
        bool GetFullscreen() override {
            return this->_fullscreen
                || glfwGetWindowMonitor(this->_window) != NULL
                || this->GetWindowSize() == this->_getCurrentScreenSize();
        }
        
        void SetFullscreen(bool fullscreen) override {
            this->_fullscreen = fullscreen;
            if (this->_window != NULL) {
                this->Reset();
            }
        }
        
        void SetVSync(bool vSync) override {
            this->_vSync = vSync;
            if (this->_window != NULL) {
                glfwSwapInterval(this->_vSync ? 1 : 0);
            }
        }
        
        void SetAntiAlias(int samples) override {
            this->_aaSamples = samples;
        }
        
        void SetDebug(bool debug) override {
            this->_debug = debug;
        }
        
        void SetCaptureMouse(bool capture) override {
            this->_captureMouse = capture;
            if (this->_window  != NULL) {
                glfwSetInputMode(this->_window, GLFW_CURSOR, capture ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
            }
        }
        
        OpenGLVersion GetGlVersion() override {
            assert(this->_window != NULL);
            OpenGLVersion ret;
            
            ret.major = glfwGetWindowAttrib(this->_window, GLFW_CONTEXT_VERSION_MAJOR);
            ret.minor = glfwGetWindowAttrib(this->_window, GLFW_CONTEXT_VERSION_MINOR);
            ret.revision = glfwGetWindowAttrib(this->_window, GLFW_CONTEXT_REVISION);
            ret.glslVersion = (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION);
            ret.glewVersion = (const char*) glewGetString(GLEW_VERSION);
            ret.fullGLVersion = (const char*) glGetString(GL_VERSION);
            ret.glVendor = (const char*) glGetString(GL_VENDOR);
            ret.glRenderer = (const char*) glGetString(GL_RENDERER);
            
            return ret;
        }
        
        std::string GetWindowVersion() override {
            std::stringstream glfwVersion;
            
            glfwVersion << "GLFW v"
            << GLFW_VERSION_MAJOR
            << "." << GLFW_VERSION_MINOR
            << "." << GLFW_VERSION_REVISION;
        
            return glfwVersion.str();
        }
        
        int GetMaxTextureSize() override {
            GLint result = 0;
            
            glGetIntegerv(GL_MAX_TEXTURE_SIZE, &result);
            
            return result;
        }
        
        glm::mat4 GetOrthoProjection() override {
            glm::vec2 window = this->GetWindowSize();
            return glm::ortho(0.0f, window.x, window.y, 0.0f, 1000.0f, -1000.0f);
        }
        
        float GetAspectRatio() override {
            glm::vec2 window = this->GetWindowSize();
            return window.x / window.y;
        }
        
        RenderDriver* GetRender() override {
            return this->_render;
        }
        
    private:
        glm::vec2 _getCurrentScreenSize() {
            const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            return {mode->width, mode->height};
        }
        
        void _init() override {
            
        }
        
        void _destroy() override {
            if (this->_window == NULL) return;
            glfwDestroyWindow(this->_window);
            this->_window = NULL;
            GetEventsSingilton()->GetEvent("destroyWindow")->Emit();
        }
        
        void _resizeCallback(int width, int height) {
            this->_size = glm::vec2(width, height);
            
            Json::Value val(Json::objectValue);
            
            val["width"] = width;
            val["height"] = height;

			glViewport(0, 0, width, height);
            
            GetEventsSingilton()->GetEvent("rawResize")->Emit(val);
        }
        
        void _keypressCallback(int rawKey, int scanCode, int state, int mods) {
            if (!glfwGetWindowAttrib(this->_window, GLFW_FOCUSED) && !this->GetFullscreen()) {
                return;
            }
            
            std::string key = glfwGetKeyName(rawKey);
            
            if (rawKey == -1) { // On OSX -1 is the function key and should be ignored
                return;
            }
            
            Json::Value val(Json::objectValue);
            
            val["rawKey"] = rawKey;
            val["key"] = key;
            val["rawPress"] = state;
            val["state"] = glfwGetKeyStateName(state);
            val["shift"] = (mods & GLFW_MOD_SHIFT);
            
            GetEventsSingilton()->GetEvent("rawInput")->Emit(val);
        }
        
        void _mouseButtonCallback(int button, int action, int mods) {
            Json::Value val(Json::objectValue);
            
            std::string buttonName = glfwGetMouseButtonName(button);
            
            val["button"] = buttonName;
            val["action"] = action == GLFW_PRESS ? "press" : "release";
            val["rawMods"] = mods;
            
            glm::vec2 cursorPos = this->GetCursorPos();
            
            val["x"] = std::floor(cursorPos.x);
            val["y"] = std::floor(cursorPos.y);
            
            GetEventsSingilton()->GetEvent("mouseButton")->Emit(val);
        }
        
        static void WindowResize(GLFWwindow* window, int width, int height) {
            Window_glfw* win = (Window_glfw*) glfwGetWindowUserPointer(window);
            win->_resizeCallback(width, height);
        }
        
        static void WindowKeyPress(GLFWwindow* window, int rawKey, int scanCode, int state, int mods) {
            Window_glfw* win = (Window_glfw*) glfwGetWindowUserPointer(window);
            win->_keypressCallback(rawKey, scanCode, state, mods);
        }
        
        static void WindowMouseButton(GLFWwindow* window, int button, int action, int mods) {
            Window_glfw* win = (Window_glfw*) glfwGetWindowUserPointer(window);
            win->_mouseButtonCallback(button, action, mods);
        }
        
        void _create() {
            glfwDefaultWindowHints();
            
            glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
            
            if (this->_version == GraphicsVersion::OpenGL_Modern) {
#ifdef _PLATFORM_OSX
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
                glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
            } else if (this->_version == GraphicsVersion::OpenGL_Legacy) {
                
            }
            
            glfwWindowHint(GLFW_SAMPLES, this->_aaSamples);
            
            if (this->_debug) {
                glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, this->_debug ? GL_TRUE : GL_FALSE);
            }
            
            this->_window = glfwCreateWindow(this->_size.x, this->_size.y, this->_title.c_str(), this->_fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);
            
            if (this->_window == NULL) {
                Logger::begin("Window", Logger::LogLevel_Error) << "Error Creating Window" << Logger::end();
            }
            
            this->Begin();
            
            glfwSwapInterval(this->_vSync ? 1 : 0);
            
            glewExperimental = GL_TRUE;
            
            GLenum err = glewInit();
            
            if (err != GLEW_OK) {
                Logger::begin("Window", Logger::LogLevel_Error) << "Error starting GLEW: " << glewGetErrorString(err) << Logger::end();
                glfwDestroyWindow(this->_window);
                this->_window = NULL;
            }
            
            glGetError(); // GLEW always causes a GL error when it gets extentions
            
            glfwSetWindowUserPointer(this->_window, this);
            
            glfwSetWindowSizeCallback(this->_window, WindowResize);
            glfwSetKeyCallback(this->_window, WindowKeyPress);
            glfwSetMouseButtonCallback(this->_window, WindowMouseButton);
            
            if (this->_captureMouse) {
                glfwSetInputMode(this->_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            
            switch (this->_version) {
                case GraphicsVersion::OpenGL_Modern:
                case GraphicsVersion::OpenGL_Legacy:
                    this->_render = CreateRenderGL3();
                    break;
            }
            
            GetEventsSingilton()->GetEvent("postCreateContext")->Emit();
        }
        
        RenderDriverPtr _render = NULL;
        GLFWwindow* _window = NULL;
        
        glm::vec2 _size = glm::vec2(800, 600);
        bool _visible = false;
        bool _fullscreen = false;
        bool _captureMouse = false;
        std::string _title = "Engine2D";
        int _aaSamples = 0;
        bool _debug = false;
        bool _vSync = true;
    };
    
    void OnGLFWError(int error, const char* msg) {
        Logger::begin("Window", Logger::LogLevel_Error) << "GLFW Error : " << error << " : " << msg << Logger::end();
    }
    
    void Window::StaticInit() {
        glfwSetErrorCallback(OnGLFWError);
        glfwInit();
#ifdef _PLATFORM_OSX
        // As it turns out GLFW will normaly change the current directory to the Application bundle's resources path when inited on OSX, this screws up PhysFS since if the engine is hosted inside another application like python it will jump to that application's resources path
        chdir(Platform::GetInitalDirectory());
#endif
    }
    
    void Window::StaticDestroy() {
        glfwTerminate();
    }
    
    Window* CreateWindow(GraphicsVersion v) {
        return new Window_glfw(v);
    }
}